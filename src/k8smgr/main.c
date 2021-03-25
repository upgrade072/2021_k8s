#include <stdlib.h>
#include <string.h>
#include <event.h>
#include <event2/event.h>
#include <event2/thread.h>
#include <event2/http.h>
#include <evhttp.h>
#include <evdns.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* from ARIEL */
#include <conflib.h>
#include <keepalivelib.h>

/* from build-tree */
#include <fort.h>
#include <libs.h>

#define K8SMGR_HANG_INTVL	3 /* check x 3 ~ interval 3 = 9 sec */
#define K8SMSG_HTTP_LISTEN	32768

extern char *__progname;

/* from keepalive extern-library */
extern int keepaliveIndex;
extern T_keepalive *keepalive;

typedef struct proc_ctx_t {
	char *proc_name;
	int keep_index;
	int alive_status;
	int force_status;
	struct event *ev_check_status;
} proc_ctx_t;

typedef struct pod_info_t {
	char *my_pod_name;
	char *my_service_name;
	char *my_pod_namespace;

	char peer_pod_url[1024];
	int peer_pod_start;
	int peer_pod_ready;
} pod_info_t;

typedef struct main_ctx_t {
	/* my ready status */
	int force_not_ready;

	/* pod env & other */
	pod_info_t pod_info;

	/* for process status (proc_ctx_t list) */
	element_t *proc_list;

	/* for k8s http req */
	struct evhttp *http_server;

	/* for main loop */
	struct event_base *evbase_main;
	struct evdns_base *dnsbase;
} main_ctx_t;

typedef struct httpconn_info_t {
	main_ctx_t *MAIN_CTX;
	struct evhttp_connection *ev_conn;
	struct evhttp_request *ev_req;
	char request_info[1024];
} httpconn_info_t;

int util_set_linger(int fd, int onoff, int linger)
{           
    struct linger l = { .l_linger = linger, .l_onoff = onoff};
    int res = setsockopt(fd, SOL_SOCKET, SO_LINGER, &l, sizeof(l));
        
    return res;
}  

void check_proc_alive(evutil_socket_t fd, short what, void *arg)
{
	(void)fd;
	(void)what;
	proc_ctx_t *proc_ctx = (proc_ctx_t *)arg;

	if (proc_ctx->keep_index >= SYSCONF_MAX_APPL_NUM) {
		return; /* some error */
	}
	
	int keep_index = proc_ctx->keep_index;

	if (keepalive->cnt[keep_index] != keepalive->oldcnt[keep_index]) {
		/* alive */
		keepalive->retry[keep_index] = 0;
	} else {
		/* check hang */
		keepalive->retry[keep_index]++;
	}
	keepalive->oldcnt[keep_index] = keepalive->cnt[keep_index];

	if (proc_ctx->force_status > 0) {
		if (proc_ctx->force_status++ < 5) {
			proc_ctx->alive_status = 0;
		} else {
			proc_ctx->force_status = 0;
		}
		fprintf(stderr, "I'm = [%s] force (%d) alive (%d)!\n", 
				proc_ctx->proc_name, proc_ctx->force_status, proc_ctx->alive_status);
	} else if (keepalive->retry[keep_index] >= K8SMGR_HANG_INTVL) {
		proc_ctx->alive_status = 0;
	} else {
		proc_ctx->alive_status = 1;
	}
}

int create_proc_status_check(main_ctx_t *MAIN_CTX, char *proc_name, int keep_index)
{
	if (keep_index >= SYSCONF_MAX_APPL_NUM) {
		fprintf(stderr, "%s() check keep_index=(%d) excceed max=[%d]!\n", __func__, keep_index, SYSCONF_MAX_APPL_NUM);
		return -1;
	} else {
		/* init retry cnt */
		keepalive->retry[keep_index] = 0;
	}

	proc_ctx_t proc_ctx = { 
		.proc_name = strdup(proc_name),
		.keep_index = keep_index };
	element_t *proc_node = new_element(proc_name, &proc_ctx, sizeof(proc_ctx_t));
	proc_ctx_t *create_ctx = (proc_ctx_t *)proc_node->data;
	create_ctx->ev_check_status = event_new(MAIN_CTX->evbase_main, -1, EV_PERSIST, check_proc_alive, create_ctx);

	struct timeval tm_intvl = {3, 0};
	if (event_add(create_ctx->ev_check_status, &tm_intvl) == -1) {
		fprintf(stderr, "%s() fail to create check_proc_alive!\n", __func__);
		return -1;
	}

	return add_element(MAIN_CTX->proc_list, proc_node);
}

int create_keepalive_list(main_ctx_t *MAIN_CTX)
{
    FILE *fp = NULL;
    int  num = 0;                              
    char getBuf[256] = {0,}, token[64] = {0,};
    const char *sysconf = conflib_get_sysconf();

    if (!sysconf) {
        fprintf(stderr,"%s() sysconf is null!\n", __func__);
        return -1;
    }
    
    if ((fp = fopen(sysconf, "r")) == NULL) {      
        fprintf(stderr,"%s() fopen fail[%s]; err=%d(%s)!\n", __func__, sysconf, errno, strerror(errno));
        return -1;
    }
    
    if (conflib_seekSection(fp, "APPLICATIONS") < 0) {  
        fprintf(stderr,"%s() not found section[APPLICATIONS] in file[%s]!\n", __func__, sysconf);
        return -1;
    }

    while ((fgets(getBuf, sizeof(getBuf), fp) != NULL))
    {
        /* end of section */
        if (getBuf[0] == '[')
            break;

        /* comment line or empty */                                                           
        if (getBuf[0]=='#' || getBuf[0]=='\n')
            continue;

		/* get proc name & do something */
        memset(token, 0x00, sizeof(token));
        sscanf(getBuf, "%63s", token);

		fprintf(stderr, "%s() try add proc_name=[%s] index=(%d)\n", __func__, token, num);
		if (create_proc_status_check(MAIN_CTX, token, num) < 0) {
			fprintf(stderr, "%s() fail add proc_name=[%s] index=(%d)\n", __func__, token, num);
			fclose(fp);
			return -1;
		}
		num++;
    }

    fclose(fp);
    return num;
}

void proc_info_fill_cb(element_t *elem, ft_table_t *table)
{
	proc_ctx_t *proc_ctx = (proc_ctx_t *)elem->data;
	if (elem->key == NULL) {
		ft_printf_ln(table, "NAME|KEEP_IDX|CNT|OLDCNT|RETRY");
	} else {
		if (proc_ctx->keep_index >= SYSCONF_MAX_APPL_NUM) {
			/* some error */
		} else {
			ft_printf_ln(table, "%s|%d|%d|%d|%d",
				proc_ctx->proc_name,
				proc_ctx->keep_index,
				keepalive->cnt[proc_ctx->keep_index],
				keepalive->oldcnt[proc_ctx->keep_index],
				keepalive->retry[proc_ctx->keep_index]);
		}
	}
}

void proc_info_print(main_ctx_t *MAIN_CTX, struct evbuffer *buffer)
{
	ft_table_t *table = ft_create_table();
	ft_set_border_style(table, FT_PLAIN_STYLE);
	ft_set_cell_prop(table, FT_ANY_ROW, 0, FT_CPROP_LEFT_PADDING, 0);

	list_element(MAIN_CTX->proc_list, proc_info_fill_cb, table);

	fprintf(stderr, "%s", ft_to_string(table));
	evbuffer_add_printf(buffer, "%s", ft_to_string(table));

	ft_destroy_table(table);
}

void proc_check_all_alive(element_t *elem, int *all_pod_alive)
{
	proc_ctx_t *proc_ctx = (proc_ctx_t *)elem->data;
	if (elem->key == NULL) {
		return;
	} else {
		if (proc_ctx->alive_status <= 0) {
			*all_pod_alive = 0;
		}
	}
}

static const char *http_method_str(enum evhttp_cmd_type type)
{   
	const char *method;

	switch (type) {
		case EVHTTP_REQ_GET:
			method = "GET";
			break;
		case EVHTTP_REQ_POST:
			method = "POST";
			break;
		case EVHTTP_REQ_HEAD:
			method = "HEAD";
			break;
		case EVHTTP_REQ_PUT:
			method = "PUT";
			break;
		case EVHTTP_REQ_DELETE:
			method = "DELETE";
			break;
		case EVHTTP_REQ_OPTIONS:
			method = "OPTIONS";
			break;
		case EVHTTP_REQ_TRACE:
			method = "TRACE";
			break;
		case EVHTTP_REQ_CONNECT:
			method = "CONNECT";
			break;
		case EVHTTP_REQ_PATCH:
			method = "PATCH";
			break;
		default:
			method = NULL;
			break;
	}

	return (method);
}

void http_reply_common(int success, struct evhttp_request *req, int code, const char *reason, struct evbuffer *databuf)
{
	time_t current_time;
	struct tm * time_info;
	char tm_string[9];  // space for "HH:MM:SS\0"

	time(&current_time);
	time_info = localtime(&current_time);

	strftime(tm_string, sizeof(tm_string), "%H:%M:%S", time_info);

    const char *uri = evhttp_request_get_uri(req);
	fprintf(stderr, "[%s] from(%s:%d) -%s- [%s] ==> [%d:%s]\n", 
			tm_string, req->remote_host, req->remote_port, http_method_str(req->type), uri, code, reason);

	if (success < 0) {
		evhttp_send_error(req, code, reason);
	} else {
		evhttp_send_reply(req, code, reason, databuf);
	}
}

void http_reply_pod_start(struct evhttp_request *req, void *arg)
{
	(void)arg;

    http_reply_common(1, req, 200, "Ok", NULL);
}

void http_reply_pod_ready(struct evhttp_request *req, void *arg)
{
	main_ctx_t *MAIN_CTX = (main_ctx_t *)arg;

	if (req->type == EVHTTP_REQ_GET) {

		if (MAIN_CTX->force_not_ready > 0) {
			/* 503 Service Unavailable */
			return http_reply_common(-1, req, 503, "Service Unavailable", NULL);
		}	

		int all_pod_alive = 1;
		list_element(MAIN_CTX->proc_list, proc_check_all_alive, &all_pod_alive);

		all_pod_alive > 0 ? http_reply_common(0, req, 200, "Ok", NULL) : 
			http_reply_common(-1, req, 503, "Service Unavailable", NULL);

	} else if (req->type == EVHTTP_REQ_PATCH) {
		struct evkeyvalq headers = {0,};
		evhttp_parse_query(evhttp_request_get_uri(req), &headers);
		const char *status = evhttp_find_header(&headers, "status");

		if (status == NULL) {
			/* 503 Service Unavailable */
			return http_reply_common(-1, req, 503, "Service Unavailable", NULL);
		}

		MAIN_CTX->force_not_ready = atoi(status) == 0 ? 1 : 0;

		/* 200 OK */
		http_reply_common(0, req, 200, "Ok", NULL);

		evhttp_clear_headers(&headers);
	} else {
		/* 503 Service Unavailable */
		return http_reply_common(-1, req, 503, "Service Unavailable", NULL);
	}
}

void http_print_pod_status(struct evhttp_request *req, void *arg)
{
	main_ctx_t *MAIN_CTX = (main_ctx_t *)arg;
	struct evbuffer *buffer = evbuffer_new();

	proc_info_print(MAIN_CTX, buffer);

	/* 200 OK */
	http_reply_common(0, req, 200, "Ok", buffer);

	evbuffer_free(buffer);
}

void http_reply_pod_live(struct evhttp_request *req, char *proc_name, main_ctx_t *MAIN_CTX)
{
	element_t *elem = get_element(MAIN_CTX->proc_list, proc_name);
	if (elem == NULL) {
		http_reply_common(-1, req, 404, "Not Found", NULL);
		return;
	}

	proc_ctx_t *proc_ctx = (proc_ctx_t *)elem->data;
	if (proc_ctx->alive_status > 0) {
		/* 200 OK */
		http_reply_common(0, req, 200, "Ok", NULL);
	} else {
		/* 503 Service Unavailable */
		http_reply_common(-1, req, 503, "Service Unavailable", NULL);
	}
}

void http_patch_pod_live(struct evhttp_request *req, char *proc_name, main_ctx_t *MAIN_CTX)
{
	/* curl --request PATCH localhost:32768/live/K8SMGR?status=0 */
	struct evkeyvalq headers = {0,};
	evhttp_parse_query(evhttp_request_get_uri(req), &headers);
	const char *status = evhttp_find_header(&headers, "status");

	if (status == NULL) {
		/* 503 Service Unavailable */
		http_reply_common(-1, req, 503, "Service Unavailable", NULL);
	}

	element_t *elem = get_element(MAIN_CTX->proc_list, proc_name);
	if (elem == NULL) {
		http_reply_common(-1, req, 404, "Not Found", NULL);
		return;
	}

	proc_ctx_t *proc_ctx = (proc_ctx_t *)elem->data;
	proc_ctx->alive_status = atoi(status);
	if (proc_ctx->alive_status == 0) {
		proc_ctx->force_status = 1; /* set force dead for while */
	}

	evhttp_clear_headers(&headers);

	/* 200 OK */
	http_reply_common(0, req, 200, "Ok", NULL);
}

void http_reply_default(struct evhttp_request *req, void *arg)
{
	main_ctx_t *MAIN_CTX = (main_ctx_t *)arg;

    /* decode uri */
    const char *uri = evhttp_request_get_uri(req);
    struct evhttp_uri *decoded = evhttp_uri_parse(uri);
    if (!decoded) {
        http_reply_common(-1, req, HTTP_BADREQUEST, 0, NULL);
        return;
    }

    /* check path */
    const char *path = evhttp_uri_get_path(decoded);
    if (!path) path = "/";

    /* decode path */
    char *decoded_path = evhttp_uridecode(path, 0, NULL);
    if (decoded_path == NULL)
        goto err;
    if (strstr(decoded_path, ".."))
        goto err;

	if (!strncmp(decoded_path, "/live/", strlen("/live/")) && (strlen(decoded_path) > strlen("/live/"))) {
		if (req->type == EVHTTP_REQ_GET) {
			http_reply_pod_live(req, decoded_path + strlen("/live/"), MAIN_CTX);
		} else if (req->type == EVHTTP_REQ_PATCH) {
			http_patch_pod_live(req, decoded_path + strlen("/live/"), MAIN_CTX);
		}
		goto done;
	} else {
		goto err;
	}

err:
    /* nok reply */
    http_reply_common(-1, req, 404, "Not Found", NULL);
done:
    if (decoded)
        evhttp_uri_free(decoded);
    if (decoded_path)
        free(decoded_path);
}


void http_recv_reply(struct evhttp_request *req, void *arg)
{
	/* caution! req may null if timeouted! */
    httpconn_info_t *http_conn = (httpconn_info_t *)arg;
	main_ctx_t *MAIN_CTX = http_conn->MAIN_CTX;

	fprintf(stderr, "%s [%s] = (%d:%s)\n", 
			http_conn->request_info, req == NULL ? "fail" : "done",
			req == NULL ? -1 : evhttp_request_get_response_code(req),
			req == NULL ? "N/A" : evhttp_request_get_response_code_line(req));

	if (req == NULL) {
		/* dns query fail */
		MAIN_CTX->pod_info.peer_pod_start = -1;
		MAIN_CTX->pod_info.peer_pod_ready = -1;
	} else if (!strcmp(req->uri, "/start")) {
		MAIN_CTX->pod_info.peer_pod_start = req->response_code == 200 ? 1 : 0;
	} else if (!strcmp(req->uri, "/ready")) {
		MAIN_CTX->pod_info.peer_pod_ready = req->response_code == 200 ? 1 : 0;
	} else if (!strcmp(req->uri, "/print/status")) {
		/*- sample code [how to handle reply body] -*/
		char buffer[256] = {0,};
		int nread = 0;
		while ((nread = evbuffer_remove(evhttp_request_get_input_buffer(req),
						buffer, sizeof(buffer))) > 0) {
			fwrite(buffer, nread, 1, stdout);
		}
	} else {
		fprintf(stderr, "%s() caution unhandle uri=(%s) callback!\n", __func__, req->uri);
	}
	fprintf(stderr, "%s() check peer_pod_status start=(%d) ready=(%d)\n", __func__,
			MAIN_CTX->pod_info.peer_pod_start, MAIN_CTX->pod_info.peer_pod_ready);

	evhttp_connection_free(http_conn->ev_conn);

    free(http_conn);
}

void make_http_request(main_ctx_t *MAIN_CTX,
        const char *svr_addr, int svr_port,
        enum evhttp_cmd_type cmd, const char *path, int timeout,
        void (*reqcb)(struct evhttp_request *, void *))
{
    httpconn_info_t *http_conn = calloc(1, sizeof(httpconn_info_t));
    http_conn->MAIN_CTX = MAIN_CTX; /* for refer */
    http_conn->ev_conn = evhttp_connection_base_new(MAIN_CTX->evbase_main, MAIN_CTX->dnsbase, svr_addr, svr_port);
    http_conn->ev_req = evhttp_request_new(reqcb, http_conn);
    evhttp_connection_set_timeout(http_conn->ev_conn, timeout);
    evhttp_make_request(http_conn->ev_conn, http_conn->ev_req, cmd, path);
	evhttp_connection_free_on_completion(http_conn->ev_conn);

	sprintf(http_conn->request_info, "http://%s:%d%s", svr_addr, svr_port, path);
}

void main_tick(evutil_socket_t fd, short what, void *arg)
{
	(void)fd; 
	(void)what;
	(void)arg;

	/* I'm Alive */
	keepalivelib_increase();
}

void main_check_peer_pod(evutil_socket_t fd, short what, void *arg)
{
	(void)fd; 
	(void)what;
	main_ctx_t *MAIN_CTX = (main_ctx_t *)arg;

	/* check peer alive */
	make_http_request(MAIN_CTX, MAIN_CTX->pod_info.peer_pod_url, K8SMSG_HTTP_LISTEN, EVHTTP_REQ_GET, "/start", 1, http_recv_reply);
	make_http_request(MAIN_CTX, MAIN_CTX->pod_info.peer_pod_url, K8SMSG_HTTP_LISTEN, EVHTTP_REQ_GET, "/ready", 1, http_recv_reply);
}

int main_init_getenv(main_ctx_t *MAIN_CTX)
{
	/* get pod name (k8s.state-0), suffix = 0 */
	if (getenv("MY_POD_NAME") == NULL) {
		fprintf(stderr, "%s() can't find env(MY_POD_NAME)!\n", __func__);
		return -1;
	} else {
		MAIN_CTX->pod_info.my_pod_name = strdup(getenv("MY_POD_NAME"));
	}
	/* get pod service name */
	if (getenv("MY_SERVICE_NAME") == NULL) {
		fprintf(stderr, "%s() can't find env(MY_SERVICE_NAME)!\n", __func__);
		return -1;
	} else {
		MAIN_CTX->pod_info.my_service_name = strdup(getenv("MY_SERVICE_NAME"));
	}
	/* get pod namespace name */
	if (getenv("MY_POD_NAMESPACE") == NULL) {
		fprintf(stderr, "%s() can't find env(MY_POD_NAMESPACE)!\n", __func__);
		return -1;
	} else {
		MAIN_CTX->pod_info.my_pod_namespace = strdup(getenv("MY_POD_NAMESPACE"));
	}

	/* create peer pod name & url */
	char temp_pod_name[1024] = {0,};
	sprintf(temp_pod_name, "%s", MAIN_CTX->pod_info.my_pod_name);
	char *index_str = strrchr(temp_pod_name, '-');
	int index = atoi(index_str + 1);
	index = index % 2 == 0 ? index + 1 : index - 1;
	index_str[1] = '\0';
	sprintf(MAIN_CTX->pod_info.peer_pod_url, "%s%d.%s.%s.svc.cluster.local", 
			temp_pod_name, index, MAIN_CTX->pod_info.my_service_name, MAIN_CTX->pod_info.my_pod_namespace);

	fprintf(stderr, "%s() get pod_name(%s) service_name(%s) name_space(%s)\n  peer_pod_url[%s]\n",
			__func__, 
			MAIN_CTX->pod_info.my_pod_name, 
			MAIN_CTX->pod_info.my_service_name, 
			MAIN_CTX->pod_info.my_pod_namespace,
			MAIN_CTX->pod_info.peer_pod_url);

	return 0;
}

int main_init_httpserver(main_ctx_t *MAIN_CTX)
{
	/* create k8s reply http server */
	MAIN_CTX->http_server = evhttp_new(MAIN_CTX->evbase_main);
	evhttp_set_allowed_methods (MAIN_CTX->http_server, EVHTTP_REQ_GET|EVHTTP_REQ_PATCH);
	evhttp_set_cb(MAIN_CTX->http_server, "/start", http_reply_pod_start, NULL); 
	evhttp_set_cb(MAIN_CTX->http_server, "/ready", http_reply_pod_ready, MAIN_CTX); 
	evhttp_set_cb(MAIN_CTX->http_server, "/print/status", http_print_pod_status, MAIN_CTX); 
	evhttp_set_gencb (MAIN_CTX->http_server, http_reply_default, MAIN_CTX);
	
	/* start listen */
	struct evhttp_bound_socket *server_sock = evhttp_bind_socket_with_handle(MAIN_CTX->http_server, "0.0.0.0", K8SMSG_HTTP_LISTEN);
	if (server_sock == NULL) {
		fprintf(stderr, "%s() fail to make http_server listen=[0.0.0.0:%d]!\n", __func__, K8SMSG_HTTP_LISTEN);
		return -1;
	} else {
		fprintf(stderr, "%s() start http_server listen=[0.0.0.0:%d]!\n", __func__, K8SMSG_HTTP_LISTEN);
	}

	/* for eliminate close- TIME_WAIT */
	int server_fd = evhttp_bound_socket_get_fd(server_sock);
	struct linger l = { .l_linger = 0, .l_onoff = 1};
	setsockopt(server_fd, SOL_SOCKET, SO_LINGER, &l, sizeof(l));

	return 0;
}

int main_initialize(main_ctx_t *MAIN_CTX)
{
	/* make event base */
	evthread_use_pthreads();
	MAIN_CTX->evbase_main = event_base_new();
	MAIN_CTX->dnsbase = evdns_base_new(MAIN_CTX->evbase_main, 1);

	/* check sysconfig exist */
	if (keepalivelib_init(__progname) < 0) {
		fprintf(stderr, "%s() [%s] unregisterd to sysconfig!\n", __func__, __progname);
		return -1;
	} else {
		fprintf(stderr, "%s() [%s] keepaliveIndex=[%d]\n", __func__, __progname, keepaliveIndex);
	}

	/* create pod info */
	if (main_init_getenv(MAIN_CTX) < 0) {
		return -1;
	}

	/* create http server */
	if (main_init_httpserver(MAIN_CTX) < 0) {
		return -1;
	}

	/* create process check status list */
	MAIN_CTX->proc_list = new_element(NULL, NULL, 0);
	if (create_keepalive_list(MAIN_CTX) < 0) {
		fprintf(stderr, "%s() fail to create process list!\n", __func__);
		return -1;
	}

	/* tick for my condition check */
	struct timeval one_sec = {1, 0};
	struct event *ev_tick_1 = event_new(MAIN_CTX->evbase_main, -1, EV_PERSIST, main_tick, MAIN_CTX);
	if (event_add(ev_tick_1, &one_sec) == -1) {
		fprintf(stderr, "%s() fail to create main_tick_1!\n", __func__);
		return -1;
	}
	/* tick for remote condition check */
	struct timeval two_sec = {2, 0};
	struct event *ev_tick_2 = event_new(MAIN_CTX->evbase_main, -1, EV_PERSIST, main_check_peer_pod, MAIN_CTX);
	if (event_add(ev_tick_2, &two_sec) == -1) {
		fprintf(stderr, "%s() fail to create main_tick_2!\n", __func__);
		return -1;
	}

	return 0;
}

int main()
{
	main_ctx_t MAIN_CTX = {0,};

	if (main_initialize(&MAIN_CTX) < 0) {
		fprintf(stderr, "%s() fail to initialize!\n", __func__);
		exit(0);
	}

	event_base_loop(MAIN_CTX.evbase_main, EVLOOP_NO_EXIT_ON_EMPTY);

	event_base_free(MAIN_CTX.evbase_main);

	fprintf(stderr, "%s(:%d) oops reach here~!\n", __func__, __LINE__);

	return 0;
}
