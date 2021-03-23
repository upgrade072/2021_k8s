#include "topo.h"

extern char *__progname;

int init_omp_server(main_ctx_t *MAIN_CTX)
{
	/* get my label */
	char sysconf_file[128] = {0,};
	char mp_label_cmd[512] = {0,};
	sprintf(sysconf_file, "%s/data/sysconfig", getenv("IV_HOME"));
	sprintf(mp_label_cmd, "cat %s | grep MP_LABEL | awk '{print $3}'", sysconf_file);
	if (popen_cmd(mp_label_cmd, MAIN_CTX->mp_label, 127) < 0) {
		return -1;
	} else {
		fprintf(stderr, "%s(): my_mp_label=[%s] checked!\n", __func__, MAIN_CTX->mp_label);
	}

	/* create initial node topology */
	node_info_t node_info;
	memset(&node_info, 0x00, sizeof(node_info_t));
	sprintf(node_info.name, "%s", "OMP");
	sprintf(node_info.type, "%s", "OMP");
	sprintf(node_info.group, "%s", MAIN_CTX->mp_label);
	sprintf(node_info.mgnt_pri_ip, "%s", MAIN_CTX->omp_ip);
	sprintf(node_info.mgnt_sec_ip, "%s", "NULL");
	node_info.mgnt_port = MAIN_CTX->base_port + 1; /* caution ! */
	sprintf(node_info.serv_pri_ip, "%s", "NULL");
	sprintf(node_info.serv_sec_ip, "%s", "NULL");
	sprintf(node_info.info_name, "%s", "OMP_SYSTEM");
	sprintf(node_info.vnfc_id, "%s", "test-vnfc-0000");

	/* init node list & add omp node info */
	MAIN_CTX->node_list = new_element(NULL, NULL, 0);
	element_t *omp_node = new_element(node_info.name, &node_info, sizeof(node_info_t));
	add_element(MAIN_CTX->node_list, omp_node);

	/* create table & print test */
	assoc_info_print(MAIN_CTX);

	/* create recv sockfd */
	if ((MAIN_CTX->omp_sockfd = socket(PF_INET, SOCK_DGRAM, 0)) == -1) {
		fprintf(stderr, "%s() fail to call socket()!\n", __func__);
		return -1;
	}
	int sockopt_flag = 1;
	if (setsockopt(MAIN_CTX->omp_sockfd, SOL_SOCKET, SO_REUSEADDR, &sockopt_flag, sizeof(int)) < 0) {
		fprintf(stderr, "%s() fail to call setsockpot(reuseaddr)!\n", __func__);
		return -1;
	}
	if (udp_fromto_init(MAIN_CTX->omp_sockfd) < 0) {
		fprintf(stderr, "%s() fail to call udp_from_to_init()!\n", __func__);
		return -1;
	}
	struct sockaddr_in omp_addr;
	omp_addr.sin_family = AF_INET;
	omp_addr.sin_port = htons(MAIN_CTX->base_port);
	omp_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	bzero(&(omp_addr.sin_zero), 8);
	if (bind(MAIN_CTX->omp_sockfd, (struct sockaddr *)&omp_addr, sizeof(omp_addr)) < 0) {
		fprintf(stderr, "%s() fail to call bind(INADDR_ANY:%d)!\n", __func__, MAIN_CTX->base_port);
		return -1;
	} else {
		fprintf(stderr, "%s() create sockfd=(%d) (INADDY_ANY:%d)\n", __func__, MAIN_CTX->omp_sockfd, MAIN_CTX->base_port);
	}

	return 0;
}

int init_omp_client(main_ctx_t *MAIN_CTX)
{
	/* get pod env */
	const char *my_node_name = MAIN_CTX->my_node_name = getenv("MY_NODE_NAME");
	const char *my_pod_name = MAIN_CTX->my_pod_name = getenv("MY_POD_NAME");
	const char *my_node_ip = MAIN_CTX->my_node_ip = getenv("MY_NODE_IP");
	const char *my_pod_ip = MAIN_CTX->my_pod_ip = getenv("MY_POD_IP");
	if (my_node_name == NULL) {
		fprintf(stderr,"%s(): can't get [env:MY_NODE_NAME]!\n", __func__);
		return -1;
	} else {
		fprintf(stderr,"%s(): get [env:MY_NODE_NAME=%s]!\n", __func__, my_node_name);
	}
	if (my_pod_name == NULL) {
		fprintf(stderr,"%s(): can't get [env:MY_POD_NAME]!\n", __func__);
		return -1;
	} else {
		fprintf(stderr,"%s(): get [env:MY_POD_NAME=%s]!\n", __func__, my_pod_name);
	}
	if (my_node_ip == NULL) {
		fprintf(stderr,"%s(): can't get [env:MY_NODE_IP]!\n", __func__);
		return -1;
	} else {
		fprintf(stderr,"%s(): get [env:MY_NODE_IP=%s]!\n", __func__, my_node_ip);
	}
	if (my_pod_ip == NULL) {
		fprintf(stderr,"%s(): can't get [env:MY_POD_IP]!\n", __func__);
		return -1;
	} else {
		fprintf(stderr,"%s(): get [env:MY_POD_IP=%s]!\n", __func__, my_pod_ip);
	}

	/* get my index */
	char *index_str = strrchr(my_pod_name, '-');
	if (index_str == NULL) {
		fprintf(stderr, "%s(): can't get index str from my_pod_name=[%s]!\n", __func__, my_pod_name);
		return -1;
	} else {
		int my_mp_index = MAIN_CTX->my_mp_index = atoi(index_str + 1);
		fprintf(stderr, "%s(): get my_mp_index=(%d) from=[%s]\n", __func__, my_mp_index, my_pod_name);
	}

	/* get assoc info from ~/label_conf */
	if (fopen_read("/label_conf/name", MAIN_CTX->name, 127) == NULL) {
		return -1;
	} else {
		fprintf(stderr, "%s(): /label_conf/name=[%s] checked! ", __func__, MAIN_CTX->name);
		sprintf(MAIN_CTX->name + strlen(MAIN_CTX->name), "_%02d", MAIN_CTX->my_mp_index + 1);
		fprintf(stderr, "name converted to=[%s]!\n", MAIN_CTX->name);
	}

	if (fopen_read("/label_conf/type", MAIN_CTX->type, 127) == NULL) {
		return -1;
	} else {
		fprintf(stderr, "%s(): /label_conf/type=[%s] checked!\n", __func__, MAIN_CTX->type);
	}

	if (fopen_read("/label_conf/group", MAIN_CTX->group, 127) == NULL) {
		return -1;
	} else {
		fprintf(stderr, "%s(): /label_conf/group=[%s] checked!\n", __func__, MAIN_CTX->group);
	}

	/* init node list & add omp node info */
	MAIN_CTX->node_list = new_element(NULL, NULL, 0);

	return 0;
}

int initialize(main_ctx_t *MAIN_CTX)
{
	/* start log */
	fprintf(stderr, "----------------------------------\n");
	fprintf(stderr, "process <%s> started-!\n", __progname);
	fprintf(stderr, "----------------------------------\n");

	/* get ems topo svr addr */
	const char *ems_topo_addr = MAIN_CTX->ems_topo_addr = getenv(EMS_TOPO_ADDR);
	if (ems_topo_addr == NULL) {
		fprintf(stderr, "%s(): can't get [env:%s]!\n", __func__, EMS_TOPO_ADDR);
		return -1;
	} else {
		fprintf(stderr, "%s(): get [env:%s=%s]!\n", __func__, EMS_TOPO_ADDR, ems_topo_addr);
	}

	/* get my sys label */
	const char *my_sys_name = MAIN_CTX->my_sys_name = getenv("MY_SYS_NAME");
	if (my_sys_name == NULL) {
		fprintf(stderr, "%s(): can't get [env:%s] --> will client\n", __func__, MY_SYS_NAME);
	} else {
		fprintf(stderr, "%s(): get [env:%s=%s]!\n", __func__, MY_SYS_NAME, my_sys_name);
	}

	/* get omp ip:port */
	char port_str[128] = {0,};
	sscanf(MAIN_CTX->ems_topo_addr, "%127[^:]:%127s", MAIN_CTX->omp_ip, port_str);
	MAIN_CTX->base_port = atoi(port_str);
	fprintf(stderr, "%s(): omp_ip=[%s] base_port=[0x%x]\n", __func__, MAIN_CTX->omp_ip, MAIN_CTX->base_port);

	/* role decision */
	if (my_sys_name != NULL && !strcasecmp(my_sys_name, "omp")) {
		fprintf(stderr, "%s(): my_sys_name=[OMP] so run as server!\n", __func__);
		MAIN_CTX->role = CR_SERVER;
	} else {
		fprintf(stderr, "%s(): my_sys_name=[%s] so run as client!\n", __func__, my_sys_name == NULL ? "null" : my_sys_name);
		MAIN_CTX->role = CR_CLIENT;
	}

	/* if OMP (server) */
	if (MAIN_CTX->role == CR_SERVER) {
		return init_omp_server(MAIN_CTX);
	} else {
		return init_omp_client(MAIN_CTX);
	}

	return 0;
}

