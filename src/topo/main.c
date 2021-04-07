#include "topo.h"

void main_tick(evutil_socket_t fd, short what, void *arg)
{
	main_ctx_t *MAIN_CTX = (main_ctx_t *)arg;

	if (MAIN_CTX->role == CR_SERVER) {
		int cleanup_count = assoc_info_cleanup(MAIN_CTX);
		if (cleanup_count > 0) {
			fprintf(stderr, "%s() node re-arragnged! remove count=(%d)!\n", __func__, cleanup_count);
			assoc_info_print(MAIN_CTX);
			// TODO : some rearrange

			node_info_print(MAIN_CTX);
		}
	}
}

void proc_exit()
{
	fprintf(stderr, "%s() called!\n", __func__);
	exit(0);
}

void proc_start(main_ctx_t *MAIN_CTX)
{
	MAIN_CTX->evbase_main = event_base_new();

	/* tick event */
	struct timeval one_sec = {1, 0};
	struct event *ev_tick = event_new(MAIN_CTX->evbase_main, -1, EV_PERSIST, main_tick, MAIN_CTX);
	event_add(ev_tick, &one_sec) < 0 ? proc_exit(): NULL;

	/* add event */
	if (MAIN_CTX->role == CR_CLIENT) {
		struct event *ev_udp_snd_rcv = event_new(MAIN_CTX->evbase_main, -1, EV_PERSIST, udp_client_proc, MAIN_CTX);
		event_add(ev_udp_snd_rcv, &one_sec) < 0 ? proc_exit(): NULL;
	} else {
		struct event *ev_udp_rcv_snd = event_new(MAIN_CTX->evbase_main, MAIN_CTX->omp_sockfd, EV_READ | EV_PERSIST, udp_server_proc, MAIN_CTX);
		event_add(ev_udp_rcv_snd, NULL) < 0 ? proc_exit(): NULL;
	}

	/* main loop */
	event_base_loop(MAIN_CTX->evbase_main, EVLOOP_NO_EXIT_ON_EMPTY);

	/* main end */
	event_base_free(MAIN_CTX->evbase_main);
	fprintf(stderr, "oops~! reach here!\n");
}

int main()
{
	main_ctx_t MAIN_CTX = {0,};

	evthread_use_pthreads();

	initialize(&MAIN_CTX) < 0 ? proc_exit() : NULL;

	proc_start(&MAIN_CTX);

	return 0;
}
