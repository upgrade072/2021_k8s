#include "topo.h"

void assoc_info_fill_cb(element_t *elem, ft_table_t *table)
{
	node_info_t *node_info = (node_info_t *)elem->data;
	if (elem->key == NULL) {
		ft_printf_ln(table, "%s| |%s|%s|%s|%s|%s|%s|%s|%s|%s",
				"### NAME", "TYPE", "GROUP", "MGNT_PRI_IP", "MGNT_SEC_IP", "MGNT_PORT",
				"SERV_PRI_IP", "SERV_SEC_IP", "INFO_NAME", "VNFC_ID");
	} else {
		ft_printf_ln(table, "%s|=|%s|%s|%s|%s|0x%x|%s|%s|%s|%s",
				node_info->name,
				node_info->type,
				node_info->group,
				node_info->mgnt_pri_ip,
				node_info->mgnt_sec_ip,
				node_info->mgnt_port,
				node_info->serv_pri_ip,
				node_info->serv_sec_ip,
				node_info->info_name,
				node_info->vnfc_id);
	}
}

void assoc_info_print(main_ctx_t *MAIN_CTX)
{
	ft_table_t *table = ft_create_table();
	ft_set_border_style(table, FT_PLAIN_STYLE);
	ft_set_cell_prop(table, FT_ANY_ROW, 0, FT_CPROP_LEFT_PADDING, 0);

	list_element(MAIN_CTX->node_list, assoc_info_fill_cb, table);

	// -- save to file
	char assoc_info_path[1024] = {0,};
	if (getenv("IV_HOME")) {
		sprintf(assoc_info_path, "%s/data/associate_config", getenv("IV_HOME"));
	} else {
		sprintf(assoc_info_path, "/data/associate_config");
	}
	fopen_cmd(assoc_info_path, "[ASSOCIATE_SYSTEMS]", ft_to_string(table));
	// -- print to stderr
	fprintf(stderr, "%s", ft_to_string(table));

	ft_destroy_table(table);
}

void assoc_info_clean_cb(element_t *elem, cb_clean_assoc_arg_t *arg)
{
	node_info_t *node_info = (node_info_t *)elem->data;
	if (elem->key == NULL || node_info == NULL) {		/* root node */
		return;
	} else if (node_info->heartbeat_tm == 0) {			/* omp node */
		return;
	} else {
		if (arg->curr_tm - node_info->heartbeat_tm >= ASSOC_CLEAR_TM) {
			fprintf(stderr, "%s() remove un-heartbeat node=(%s) clear_count=(%d)!\n", 
					__func__, elem->key, ++arg->clear_count);
			rem_element(elem);
		}
	}
}

int assoc_info_cleanup(main_ctx_t *MAIN_CTX)
{
	cb_clean_assoc_arg_t arg = {
		.curr_tm = time(NULL), 
		.clear_count = 0
	};
	list_element(MAIN_CTX->node_list, assoc_info_clean_cb, &arg);

	return arg.clear_count;
}

void assoc_info_get_port_cb(element_t *elem, int *port)
{
	node_info_t *node_info = (node_info_t *)elem->data;
	if (elem->key == NULL || node_info == NULL) {		/* root node */
		return;
	} else if (*port < 0) {								/* already used */
		return;
	} else {
		if (node_info->mgnt_port == *port) {
			*port = -1;
		}
	}
}

int assoc_info_get_unuse_port(main_ctx_t *MAIN_CTX)
{
	int port = MAIN_CTX->base_port;
	while (1) {
		int res = ++port;
		list_element(MAIN_CTX->node_list, assoc_info_get_port_cb, &res);

		if (res < 0) {
			continue;
		} else {
			return res;
		}
	}
}

void assoc_info_reply_cb(element_t *elem, char *buffer)
{
	node_info_t *node_info = (node_info_t *)elem->data;
	if (elem->key == NULL || node_info == NULL) {		/* root node */
		return;
	} else {
		char mgnt_port_str[128] = {0,};
		if (node_info->mgnt_port >= 0) {
			sprintf(mgnt_port_str, "0x%x", node_info->mgnt_port);
		} else {
			sprintf(mgnt_port_str, "%s", "NULL");
		}
		sprintf(buffer + strlen(buffer), "%s|%s|%s|%s|%s|%s|%s|%s|%s|%s\n",
				node_info->name,
				node_info->type,
				node_info->group,
				node_info->mgnt_pri_ip,
				node_info->mgnt_sec_ip,
				mgnt_port_str,
				node_info->serv_pri_ip,
				node_info->serv_sec_ip,
				node_info->info_name,
				node_info->vnfc_id);
	}
}

void assoc_info_reply_msg(main_ctx_t *MAIN_CTX, char *buffer)
{
	list_element(MAIN_CTX->node_list, assoc_info_reply_cb, buffer);
}

void assoc_info_unset_cb(element_t *elem, main_ctx_t *MAIN_CTX)
{
	node_info_t *node_info = (node_info_t *)elem->data;
	if (elem->key == NULL || node_info == NULL) {		/* root node */
		return;
	} else {
		node_info->chk_exist_flag = 0;
	}
}

void assoc_info_unset_chk_flag(main_ctx_t *MAIN_CTX)
{
	list_element(MAIN_CTX->node_list, assoc_info_unset_cb, MAIN_CTX);
}

void assoc_info_remove_cb(element_t *elem, int *clear_count)
{
	node_info_t *node_info = (node_info_t *)elem->data;
	if (elem->key == NULL || node_info == NULL) {		/* root node */
		return;
	} else {
		if (node_info->chk_exist_flag == 0) {
			fprintf(stderr, "%s() remove un-checked node=(%s) clear_count=(%d)!\n", 
					__func__, elem->key, ++(*clear_count));
			rem_element(elem);
		}
	}
}

int assoc_info_remove_uncheck_node(main_ctx_t *MAIN_CTX)
{
	int clear_count = 0;
	list_element(MAIN_CTX->node_list, assoc_info_remove_cb, &clear_count);

	return clear_count;
}

void assoc_info_list_cb(element_t *elem, cb_make_list_arg_t *arg)
{
	node_info_t *node_info = (node_info_t *)elem->data;
	if (elem->key == NULL || node_info == NULL) {		/* root node */
		return;
	} else {
		if (arg->used + 1 >= arg->limit) {
			return;
		} else {
			memcpy(&arg->node_list[arg->used++], node_info, sizeof(node_info_t));
		}
	}
}

int assoc_info_compare(const void *v1, const void *v2)
{
	const node_info_t *node1 = v1;
	const node_info_t *node2 = v2;

	/* check node name */
	int res1 = strcmp(node1->node_name, node2->node_name);
	if (res1 < 0)
		return -1;
	else if (res1 > 0)
		return 1;

	/* check pod name */
	int res2 = strcmp(node1->pod_name, node2->pod_name);
	if (res2 < 0)
		return -1;
	else if (res2 > 0)
		return 1;

	return 0;
}

int assoc_info_make_list(main_ctx_t *MAIN_CTX, node_info_t *node_list, size_t size)
{
	cb_make_list_arg_t arg = {
		.node_list = node_list,
		.limit = size / sizeof(node_info_t),
		.used = 0
	};

	list_element(MAIN_CTX->node_list, assoc_info_list_cb, &arg);

	if (arg.used > 0) {
		qsort(arg.node_list, arg.used, sizeof(node_info_t), assoc_info_compare);
	}

	return arg.used;
}

int assoc_info_get_from_pod(const char *recv_str, node_info_t *node_info)
{
	/* clear */
	memset(node_info, 0x00, sizeof(node_info_t));

	/* scan info */
	// FEP_01|MP|LMF|192.168.70.82|10.0.5.34|k8s-vm.5|k8s-test-downward-0
	int scan_res = sscanf(recv_str, "%127[^|]|%127[^|]|%127[^|]|%127[^|]|%127[^|]|%127[^|]|%127s",
			node_info->name,
			node_info->type,
			node_info->group,
			node_info->mgnt_pri_ip,
			node_info->serv_pri_ip,
			node_info->node_name,
			node_info->pod_name);
	sprintf(node_info->mgnt_sec_ip, "%s", "NULL");
	sprintf(node_info->serv_sec_ip, "%s", "NULL");

	if (scan_res == EOF || scan_res <= 0) {
		fprintf(stderr, "%s() fail to parse recv assoc_info=[%s]!\n", __func__, recv_str);
		return -1;
	} else {
		//fprintf(stderr, "%s() success to parse recv assoc_info=[%s] item_num=[%d]!\n", __func__, recv_str, scan_res);
		return 0;
	}
}

int assoc_info_get_from_omp(const char *recv_str, node_info_t *node_info)
{
	/* clear */
	memset(node_info, 0x00, sizeof(node_info_t));

	/* scan info */
	// OMP|OMP|LMF|192.168.70.143|NULL|0x1f41|NULL|NULL|SYSTEM01|test-vnfc-0000
	char mgnt_port_str[128] = {0,};
	int scan_res = sscanf(recv_str, "%127[^|]|%127[^|]|%127[^|]|%127[^|]|%127[^|]|%127[^|]|%127[^|]|%127[^|]|%127[^|]|%127s",
			node_info->name,
			node_info->type,
			node_info->group,
			node_info->mgnt_pri_ip,
			node_info->mgnt_sec_ip,
			mgnt_port_str,
			node_info->serv_pri_ip,
			node_info->serv_sec_ip,
			node_info->info_name,
			node_info->vnfc_id);
	node_info->mgnt_port = strtol(mgnt_port_str + 2, NULL, 16);

	if (scan_res == EOF || scan_res <= 0) {
		fprintf(stderr, "%s() fail to parse recv assoc_info=[%s]!\n", __func__, recv_str);
		return -1;
	} else {
		//fprintf(stderr, "%s() success to parse recv assoc_info=[%s] item_num=[%d]!\n", __func__, recv_str, scan_res);
		return 0;
	}
}

int node_proc_omp_send_info(main_ctx_t *MAIN_CTX, char *recv_msg)
{
	node_info_t node_info;

	if (assoc_info_get_from_omp(recv_msg, &node_info) < 0) {
		return NP_NONE;
	}

	/* node checked */
	node_info.chk_exist_flag = 1;

	/* find node already exist */
	element_t *elem = get_element(MAIN_CTX->node_list, node_info.name);

	/* if new, add node_info */
	if (elem == NULL) {
		fprintf(stderr, "%s() try add new node=(%s) into list!\n", __func__, node_info.name);
		element_t *new_node = new_element(node_info.name, &node_info, sizeof(node_info_t));
		add_element(MAIN_CTX->node_list, new_node);
		return NP_ADD;
	}

	/* check data exist */
	node_info_t *already_node = (node_info_t *)elem->data;
	if (already_node == NULL) {
		return NP_NONE;
	}

	/* if same node exist update hb tm */
	if (!strcmp(already_node->mgnt_pri_ip, node_info.mgnt_pri_ip) &&
			!strcmp(already_node->serv_pri_ip, node_info.serv_pri_ip)) {
		/* if have same info already, update check flag */
		already_node->chk_exist_flag = 1;
		return NP_UPTM;
	}

	/* if key same but info diff, replace node */
	{
		fprintf(stderr, "%s() relpace node=(%s) into list!\n", __func__, node_info.name);
		rem_element(elem);
		element_t *new_node = new_element(node_info.name, &node_info, sizeof(node_info_t));
		add_element(MAIN_CTX->node_list, new_node);
		return NP_REP;
	}
}

int node_proc_pod_send_info(main_ctx_t *MAIN_CTX, char *recv_msg)
{
    /* parsing with node_info */
    node_info_t node_info;
    int res = assoc_info_get_from_pod(recv_msg, &node_info);
    if (res < 0) {
        fprintf(stderr, "%s() parse fail! proc stop!\n", __func__);
		return -1;
    }

    /* save recv_time to node_info */
    node_info.heartbeat_tm = time(NULL);

    /* find node already exist */
    element_t *elem = get_element(MAIN_CTX->node_list, node_info.name);

	if (elem == NULL) {
        /* if new, add node_info */
        node_info.mgnt_port = assoc_info_get_unuse_port(MAIN_CTX);
		if (strlen(node_info.pod_name) > 0) {
			sprintf(node_info.info_name, "%s", node_info.pod_name);
		} else {
			sprintf(node_info.info_name, "SYSTEM%02d", node_info.mgnt_port - MAIN_CTX->base_port);
		}

        fprintf(stderr, "%s() try add new node=(%s) into list with_port=(0x%x)!\n", __func__, node_info.name, node_info.mgnt_port);

        element_t *new_node = new_element(node_info.name, &node_info, sizeof(node_info_t));
        add_element(MAIN_CTX->node_list, new_node);

		return NP_ADD;
	}

	/* check data exist */
	node_info_t *already_node = (node_info_t *)elem->data;
	if (already_node == NULL) {
		return NP_NONE;
	}

	/* if same node exist update hb tm */
	if (!strcmp(already_node->mgnt_pri_ip, node_info.mgnt_pri_ip) &&
			!strcmp(already_node->serv_pri_ip, node_info.serv_pri_ip)) {
		/* if have same info already, update heartbeat_tm */
		already_node->heartbeat_tm = node_info.heartbeat_tm;

		return NP_UPTM;
	}

	/* if have other cluster info, replace node_info */
	{
		node_info.mgnt_port = already_node->mgnt_port;
		if (strlen(node_info.pod_name) > 0) {
			sprintf(node_info.info_name, "%s", node_info.pod_name);
		} else {
			sprintf(node_info.info_name, "SYSTEM%02d", node_info.mgnt_port - MAIN_CTX->base_port);
		}

		fprintf(stderr, "%s() relpace node=(%s) into list with_port=(0x%x)!\n", __func__, node_info.name, node_info.mgnt_port);

		rem_element(elem);

		element_t *new_node = new_element(node_info.name, &node_info, sizeof(node_info_t));
		add_element(MAIN_CTX->node_list, new_node);

		return NP_REP;
	}
}

void node_info_print(main_ctx_t *MAIN_CTX)
{
	/* update node info data */
	node_info_t node_list[TOPO_MAX_LIST];
	int count = assoc_info_make_list(MAIN_CTX, node_list, sizeof(node_list));

	/* create table */
    ft_table_t *table = ft_create_table();
    ft_set_border_style(table, FT_PLAIN_STYLE);
    
    ft_add_separator(table);
    ft_printf_ln(table, "%s|%s|%s|%s|%s|%s", "NODE_NAME", "MGNT_IP", "MGNT_PORT", "POD_NAME", "POD_IP", "MP_NAME");
    ft_add_separator(table);
    for (int i = 0; i < count; i++) {
        ft_printf_ln(table, "%s|%s|%d/0x%x|%s|%s|%s",
                strlen(node_list[i].node_name) > 0 ? node_list[i].node_name : "N/A",
                node_list[i].mgnt_pri_ip,
                node_list[i].mgnt_port,
                node_list[i].mgnt_port,
                strlen(node_list[i].pod_name) > 0 ? node_list[i].pod_name : "N/A",
                strlen(node_list[i].serv_pri_ip) > 0 ? node_list[i].serv_pri_ip : "N/A",
                node_list[i].name);
    }
    ft_add_separator(table);
    
	/* use table */
	// -- save to file
	char node_info_path[1024] = {0,};
	if (getenv("IV_HOME")) {
		sprintf(node_info_path, "%s/data/node_info", getenv("IV_HOME"));
	} else {
		sprintf(node_info_path, "/data/node_info");
	}
	fopen_cmd(node_info_path, "[NODE_INFO]", ft_to_string(table));
	// -- print to stderr
    fprintf(stderr, "%s", ft_to_string(table));

	/* remove table */
    ft_destroy_table(table);
}
