#include "topo.h"

int udp_fromto_init(int s)
{
	int proto, flag = 0, opt = 1;
	struct sockaddr_storage si;
	socklen_t si_len = sizeof(si);

	errno = ENOSYS;

	memset(&si, 0, sizeof(si));
	if (getsockname(s, (struct sockaddr *) &si, &si_len) < 0) {
		return -1;
	}

	if (si.ss_family == AF_INET) {
		proto = IPPROTO_IP;
		flag = IP_PKTINFO;
	} else if (si.ss_family == AF_INET6) {
		proto = IPPROTO_IPV6;
		flag = IPV6_RECVPKTINFO;
	} else {
		return -1;
	}
	return setsockopt(s, proto, flag, &opt, sizeof(opt));
}

int udp_recv_fromto(int s, void *buf, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen, struct sockaddr *to, socklen_t *tolen)
{
    struct msghdr msgh;
    struct cmsghdr *cmsg;
    struct iovec iov;
    char cbuf[256];
    int err;
    struct sockaddr_storage si;
    socklen_t si_len = sizeof(si);

    /*
     *  Catch the case where the caller passes invalid arguments.
     */
    if (!to || !tolen) return recvfrom(s, buf, len, flags, from, fromlen);

    memset(&si, 0, sizeof(si));

    /*
     *  recvmsg doesn't provide sin_port so we have to
     *  retrieve it using getsockname().
     */
    if (getsockname(s, (struct sockaddr *)&si, &si_len) < 0) {
        return -1;
    }

    /*
     *  Initialize the 'to' address.  It may be INADDR_ANY here,
     *  with a more specific address given by recvmsg(), below.
     */
    if (si.ss_family == AF_INET) {
        struct sockaddr_in *dst = (struct sockaddr_in *) to;
        struct sockaddr_in *src = (struct sockaddr_in *) &si;

        if (*tolen < sizeof(*dst)) {
            errno = EINVAL;
            return -1;
        }
        *tolen = sizeof(*dst);
        *dst = *src;
    } else if (si.ss_family == AF_INET6) {
        struct sockaddr_in6 *dst = (struct sockaddr_in6 *) to;
        struct sockaddr_in6 *src = (struct sockaddr_in6 *) &si;

        if (*tolen < sizeof(*dst)) {
            errno = EINVAL;
            return -1;
        }
        *tolen = sizeof(*dst);
        *dst = *src;
    } else {
        errno = EINVAL;
        return -1;
    }

    /* Set up iov and msgh structures. */
    memset(&cbuf, 0, sizeof(cbuf));
    memset(&msgh, 0, sizeof(struct msghdr));
    iov.iov_base = buf;
    iov.iov_len  = len;
    msgh.msg_control = cbuf;
    msgh.msg_controllen = sizeof(cbuf);
    msgh.msg_name = from;
    msgh.msg_namelen = fromlen ? *fromlen : 0;
    msgh.msg_iov  = &iov;
    msgh.msg_iovlen = 1;
    msgh.msg_flags = 0;

    /* Receive one packet. */
    if ((err = recvmsg(s, &msgh, flags)) < 0) {
        return err;
    }

    if (fromlen) *fromlen = msgh.msg_namelen;

    /* Process auxiliary received data in msgh */
    for (cmsg = CMSG_FIRSTHDR(&msgh);
         cmsg != NULL;
         cmsg = CMSG_NXTHDR(&msgh,cmsg)) {

        if ((cmsg->cmsg_level == IPPROTO_IP) &&
            (cmsg->cmsg_type == IP_PKTINFO)) {
            struct in_pktinfo *i =
                (struct in_pktinfo *) CMSG_DATA(cmsg);
            ((struct sockaddr_in *)to)->sin_addr = i->ipi_addr;
            *tolen = sizeof(struct sockaddr_in);
            break;
        }

        if ((cmsg->cmsg_level == IPPROTO_IPV6) &&
            (cmsg->cmsg_type == IPV6_PKTINFO)) {
            struct in6_pktinfo *i =
                (struct in6_pktinfo *) CMSG_DATA(cmsg);
            ((struct sockaddr_in6 *)to)->sin6_addr = i->ipi6_addr;
            *tolen = sizeof(struct sockaddr_in6);
            break;
        }
    }

    return err;
}

int udp_send_fromto(int s, void *buf, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen, struct sockaddr *to, socklen_t *tolen)
{
    struct msghdr msgh;
    struct iovec iov;
    char cbuf[256];

    /*
     *  Unknown address family, die.
     */
    if (from && (from->sa_family != AF_INET) && (from->sa_family != AF_INET6)) {
        errno = EINVAL;
        return -1;
    }

    /*
     *  If the sendmsg() flags aren't defined, fall back to
     *  using sendto().  These flags are defined on FreeBSD,
     *  but laying it out this way simplifies the look of the
     *  code.
     */

    /*
     *  No "from", just use regular sendto.
     */
    if (!from || (*fromlen == 0)) {
        return sendto(s, buf, len, flags, to, *tolen);
    }

    /* Set up control buffer iov and msgh structures. */
    memset(&cbuf, 0, sizeof(cbuf));
    memset(&msgh, 0, sizeof(msgh));
    memset(&iov, 0, sizeof(iov));
    iov.iov_base = buf;
    iov.iov_len = len;

    msgh.msg_iov = &iov;
    msgh.msg_iovlen = 1;
    msgh.msg_name = to;
    msgh.msg_namelen = *tolen;

    if (from->sa_family == AF_INET) {
        struct sockaddr_in *s4 = (struct sockaddr_in *) from;

        struct cmsghdr *cmsg;
        struct in_pktinfo *pkt;

        msgh.msg_control = cbuf;
        msgh.msg_controllen = CMSG_SPACE(sizeof(*pkt));

        cmsg = CMSG_FIRSTHDR(&msgh);
        cmsg->cmsg_level = IPPROTO_IP;
        cmsg->cmsg_type = IP_PKTINFO;
        cmsg->cmsg_len = CMSG_LEN(sizeof(*pkt));

        pkt = (struct in_pktinfo *) CMSG_DATA(cmsg);
        memset(pkt, 0, sizeof(*pkt));
        pkt->ipi_spec_dst = s4->sin_addr;
    }

    if (from->sa_family == AF_INET6) {
        struct sockaddr_in6 *s6 = (struct sockaddr_in6 *) from;

        struct cmsghdr *cmsg;
        struct in6_pktinfo *pkt;

        msgh.msg_control = cbuf;
        msgh.msg_controllen = CMSG_SPACE(sizeof(*pkt));

        cmsg = CMSG_FIRSTHDR(&msgh);
        cmsg->cmsg_level = IPPROTO_IPV6;
        cmsg->cmsg_type = IPV6_PKTINFO;
        cmsg->cmsg_len = CMSG_LEN(sizeof(*pkt));

        pkt = (struct in6_pktinfo *) CMSG_DATA(cmsg);
        memset(pkt, 0, sizeof(*pkt));
        pkt->ipi6_addr = s6->sin6_addr;
    }

    return sendmsg(s, &msgh, flags);
}

void udp_client_proc(evutil_socket_t fd, short what, void *arg)
{
	main_ctx_t *MAIN_CTX = (main_ctx_t *)arg;

	int sockfd;
	struct sockaddr_in omp_addr;
	struct hostent *he;
	socklen_t addr_len;

	/* create socket */
	if ((he = gethostbyname(MAIN_CTX->omp_ip)) == NULL) {
		fprintf(stderr, "%s() fail to gethostbyname(%s)!\n", __func__, MAIN_CTX->omp_ip);
		return;
	}
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		fprintf(stderr, "%s() fail to call socket()!\n", __func__);
		return;
	}
	omp_addr.sin_family = AF_INET;
	omp_addr.sin_port = htons(MAIN_CTX->base_port);
	omp_addr.sin_addr = *((struct in_addr *)he->h_addr);
	bzero(&(omp_addr.sin_zero), 8);

	/* set timeout */
	struct timeval timeout = {1, 0};
	int optlen = sizeof(timeout);
	setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, optlen);
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, optlen);

	/* send my info */
	char send_msg[1024] = {0,};
	sprintf(send_msg, "%s|%s|%s|%s|%s|%s|%s|%s", 
			MAIN_CTX->name, MAIN_CTX->type, MAIN_CTX->group, 
			MAIN_CTX->my_node_ip, MAIN_CTX->my_pod_ip, MAIN_CTX->port,
			MAIN_CTX->my_node_name, MAIN_CTX->my_pod_name);

	int send_bytes = sendto(sockfd, send_msg, strlen(send_msg), 0,
			(struct sockaddr *)&omp_addr, sizeof(struct sockaddr));
	if (send_bytes < 0) {
		fprintf(stderr, "%s() fail to send_msg=[%s]!\n", __func__, send_msg);
		goto UDP_CLIENT_FIN;
	} else {
		//fprintf(stderr, "%s() send_msg=[%s] to=[%s:%d]!\n", __func__, send_msg, MAIN_CTX->omp_ip, MAIN_CTX->base_port);
	}

	/* recv topology info */
	char recv_msg[TOPO_MAX_BUFFER] = {0,};
	int recv_bytes = recvfrom(sockfd, recv_msg, sizeof(recv_msg), 0, (struct sockaddr *)&omp_addr, &addr_len);
	if (recv_bytes < 0) {
		fprintf(stderr, "%s() recvfrom timeout!\n", __func__);
		goto UDP_CLIENT_FIN;
	}

	/* unset all check exist flag */
	int update_count = 0;
	assoc_info_unset_chk_flag(MAIN_CTX);

	char *line = strtok(recv_msg, "\n");
	while (line) {
		int res = node_proc_omp_send_info(MAIN_CTX, line);
		if (res == NP_ADD || res == NP_REP) {
			update_count++;
		}
		/* move next */
		line = strtok(NULL, "\n");
	}

	/* clear all unchecked node */
	update_count += assoc_info_remove_uncheck_node(MAIN_CTX);
	if (update_count > 0) {
		fprintf(stderr, "%s() find assoc_info updated=(%d)!\n", __func__, update_count);
		assoc_info_print(MAIN_CTX);
		// TODO some rearrange
	}

UDP_CLIENT_FIN:
	if (sockfd > 0) close(sockfd);
	return;
}

void udp_server_proc(evutil_socket_t fd, short what, void *arg)
{
	main_ctx_t *MAIN_CTX = (main_ctx_t *)arg;

	/* prepare */
	socklen_t fl, tl;
	fl = tl = sizeof(struct sockaddr_in);
	struct sockaddr_in client_addr = {0,};
	struct sockaddr_in myrecv_addr = {0,};
	char client_addr_str[INET_ADDRSTRLEN] = {0,};
	char myrecv_addr_str[INET_ADDRSTRLEN] = {0,};

	/* recv packet */
	char recv_buff[TOPO_MAX_BUFFER] = {0,}; 
	int recv_size = udp_recv_fromto(fd, recv_buff, sizeof(recv_buff), 0,
			(struct sockaddr *)&client_addr, &fl, (struct sockaddr *)&myrecv_addr, &tl);
	if (recv_size < 0) {
		fprintf(stderr, "%s() fail to recv code=(%d)!\n", __func__, recv_size);
		return;
	}

	/* save src ip/port info */
	inet_ntop(AF_INET, &client_addr.sin_addr, client_addr_str, INET_ADDRSTRLEN);
	inet_ntop(AF_INET, &myrecv_addr.sin_addr, myrecv_addr_str, INET_ADDRSTRLEN);
	int client_port = ntohs(client_addr.sin_port);
	int myrecv_port = ntohs(myrecv_addr.sin_port);

#if 0
	fprintf(stderr, "%s() recv=(%d)bytes from=(%s:%d) to=(%s:%d) recvmsg=[%s]\n", __func__,
			recv_size, client_addr_str, client_port, myrecv_addr_str, myrecv_port, recv_buff);
#endif

	int res = node_proc_pod_send_info(MAIN_CTX, recv_buff);
	if (res < 0) {
		goto UDP_SERVER_FIN;
	} else if (res == NP_ADD || res == NP_REP) {
		assoc_info_print(MAIN_CTX);
		// TODO some rearrange
		node_info_print(MAIN_CTX);
	}

	// reply topology message
	char reply_msg[TOPO_MAX_BUFFER] = {0,};
	assoc_info_reply_msg(MAIN_CTX, reply_msg);

	int send_size = udp_send_fromto(fd, reply_msg, strlen(reply_msg), 0,
			(struct sockaddr *)&myrecv_addr, &fl, (struct sockaddr *)&client_addr, &tl);
	if (send_size < 0) {
		fprintf(stderr, "%s() send fail from=(%s:%d) to=(%s:%d) msg=[%s]\n", __func__, myrecv_addr_str, myrecv_port, client_addr_str, client_port, reply_msg);
	}

UDP_SERVER_FIN:
	return;
}

