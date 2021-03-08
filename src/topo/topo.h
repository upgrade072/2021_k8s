#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include <event.h>
#include <event2/event.h>
#include <event2/thread.h>
#include <errno.h>
#include <pthread.h>

#include <fort.h>
#include <libtopo.h>

/* from env */
#define EMS_TOPO_ADDR	"EMS_TOPO_ADDR"
#define MY_SYS_NAME		"MY_SYS_NAME"
#define TOPO_MAX_BUFFER	10240
#define ASSOC_CLEAR_TM	60
#define TOPO_MAX_LIST	128

typedef enum cluster_role_t {
	CR_CLIENT = 0,
	CR_SERVER
} cluster_role_t;

typedef enum node_proc_res_t {
	NP_NONE = 0,
	NP_ADD,
	NP_REP,
	NP_DEL,
	NP_UPTM
} node_proc_res_t;

typedef struct main_ctx_t {
	/* getenv */
	const char *ems_topo_addr;
	const char *my_sys_name;

	/* server or client */
	cluster_role_t role;

	/* for server */
	char mp_label[128];

	/* for client */
	// ---pod info
	const char *my_node_name;
	const char *my_pod_name;
	const char *my_node_ip;
	const char *my_pod_ip;
	// ---mp info
	int my_mp_index;
	char name[128];
	char type[128];
	char group[128];

	/* for communication */
	char omp_ip[128];
	int base_port;
	int omp_sockfd;

	/* for node list management */
	element_t *node_list;

	/* for main loop */
	struct event_base *evbase_main;
} main_ctx_t;

typedef struct node_info_t {
	/* for association table */
	char name[128];
	char type[128];
	char group[128];

	char mgnt_pri_ip[128];
	char mgnt_sec_ip[128];
	int mgnt_port;

	char serv_pri_ip[128];
	char serv_sec_ip[128];

	char info_name[128];
	char vnfc_id[128];

	/* for additional mgnt */
	char node_name[128];
	char pod_name[128];

	int chk_exist_flag;
	time_t heartbeat_tm;
} node_info_t;

typedef struct cb_clean_assoc_arg_t {
	time_t	curr_tm;
	int		clear_count;
} cb_clean_assoc_arg_t;

typedef struct cb_make_list_arg_t {
	node_info_t *node_list;
	int			limit;
	int			used;
} cb_make_list_arg_t;

/* ------------------------- udp.c --------------------------- */
int     udp_fromto_init(int s);
int     udp_recv_fromto(int s, void *buf, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen, struct sockaddr *to, socklen_t *tolen);
int     udp_send_fromto(int s, void *buf, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen, struct sockaddr *to, socklen_t *tolen);
void    udp_client_proc(evutil_socket_t fd, short what, void *arg);
void    udp_server_proc(evutil_socket_t fd, short what, void *arg);
/* ------------------------- util.c --------------------------- */
int     popen_cmd(const char *cmd, char *buf, int len);
void    fopen_cmd(const char *path, const char *banner, const char *buffer);
/* ------------------------- node.c --------------------------- */
void    assoc_info_fill_cb(element_t *elem, ft_table_t *table);
void    assoc_info_print(main_ctx_t *MAIN_CTX);
void    assoc_info_clean_cb(element_t *elem, cb_clean_assoc_arg_t *arg);
int     assoc_info_cleanup(main_ctx_t *MAIN_CTX);
void    assoc_info_get_port_cb(element_t *elem, int *port);
int     assoc_info_get_unuse_port(main_ctx_t *MAIN_CTX);
void    assoc_info_reply_cb(element_t *elem, char *buffer);
void    assoc_info_reply_msg(main_ctx_t *MAIN_CTX, char *buffer);
void    assoc_info_unset_cb(element_t *elem, main_ctx_t *MAIN_CTX);
void    assoc_info_unset_chk_flag(main_ctx_t *MAIN_CTX);
void    assoc_info_remove_cb(element_t *elem, int *clear_count);
int     assoc_info_remove_uncheck_node(main_ctx_t *MAIN_CTX);
void    assoc_info_list_cb(element_t *elem, cb_make_list_arg_t *arg);
int     assoc_info_compare(const void *v1, const void *v2);
int     assoc_info_make_list(main_ctx_t *MAIN_CTX, node_info_t *node_list, size_t size);
int     assoc_info_get_from_pod(const char *recv_str, node_info_t *node_info);
int     assoc_info_get_from_omp(const char *recv_str, node_info_t *node_info);
int     node_proc_omp_send_info(main_ctx_t *MAIN_CTX, char *recv_msg);
int     node_proc_pod_send_info(main_ctx_t *MAIN_CTX, char *recv_msg);
void    node_info_print(main_ctx_t *MAIN_CTX);
/* ------------------------- main.c --------------------------- */
void    main_tick(evutil_socket_t fd, short what, void *arg);
void    proc_exit();
void    proc_start(main_ctx_t *MAIN_CTX);
int     main();
/* ------------------------- init.c --------------------------- */
int     init_omp_server(main_ctx_t *MAIN_CTX);
int     init_omp_client(main_ctx_t *MAIN_CTX);
int     initialize(main_ctx_t *MAIN_CTX);
