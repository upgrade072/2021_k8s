/**
 * @file   dnsUtil.c
 * @author night1700 <night1700@night1700-15ZB970-GP50ML>
 * @date   Tue Mar  9 11:13:53 2021
 * 
 * @brief  
 * 
 * 
 */

#include<stdio.h> //printf
#include<string.h>    //strlen
#include<stdlib.h>    //malloc
#include<sys/socket.h>    //you know what this is for
#include<arpa/inet.h> //inet_addr , inet_ntoa , ntohs etc
#include<netinet/in.h>
#include<unistd.h>    //getpid
#include<time.h>	// clock_gettime


#include "dnsUtil.h"

int SEND_CNT;
int RECV_CNT;

/* char dns_servers[10][100]; */
/* int dns_server_count = 0; */


/** 
 * 
 * 
 * @param reader 
 * @param buffer 
 * @param count 
 * 
 * @return 
 */
u_char* ReadName(unsigned char* reader,unsigned char* buffer,int* count)
{
    unsigned char *name;
    unsigned int p=0,jumped=0,offset;
    int i , j;
 
    *count = 1;
    name = (unsigned char*)malloc(256);
 
    name[0]='\0';
 
    //read the names in 3www6google3com format
    while(*reader!=0)
    {
        if(*reader>=192)
        {
            offset = (*reader)*256 + *(reader+1) - 49152; //49152 = 11000000 00000000 ;)
            reader = buffer + offset - 1;
            jumped = 1; //we have jumped to another location so counting wont go up!
        }
        else
        {
            name[p++]=*reader;
        }
 
        reader = reader+1;
 
        if(jumped==0)
        {
            *count = *count + 1; //if we havent jumped to another location then we can count up
        }
    }
 
    name[p]='\0'; //string complete
    if(jumped==1)
    {
        *count = *count + 1; //number of steps we actually moved forward in the packet
    }
 
    //now convert 3www6google3com0 to www.google.com
    for(i=0;i<(int)strlen((const char*)name);i++) 
    {
        p=name[i];
        for(j=0;j<(int)p;j++) 
        {
            name[i]=name[i+1];
            i=i+1;
        }
        name[i]='.';
    }
    name[i-1]='\0'; //remove the last dot
    return name;
}
 
/*
 * This will convert www.google.com to 3www6google3com 
 * got it :)
 * */
void ChangetoDnsNameFormat(unsigned char* dns,unsigned char* host) 
{
    int lock = 0 , i;
    strcat((char*)host,".");
     
    for(i = 0 ; i < strlen((char*)host) ; i++) 
    {
        if(host[i]=='.') 
        {
            *dns++ = i-lock;
            for(;lock<i;lock++) 
            {
                *dns++=host[lock];
            }
            lock++; //or lock=i+1;
        }
    }
    *dns++='\0';
}

/** 
 * 
 * 
 * @param adjust_count 
 * @param last_turn 
 * @param sleep_cnt 
 * @param turn_count 
 */
/* void adjust_sleep(int *adjust_count, int *last_turn, int *sleep_cnt, int *turn_count) */
/* { */
/* 	if (*adjust_count <= 0) */
/* 		return; */

/* 	if (*last_turn > 0) { */
/* 		*turn_count = *turn_count - 1; */
/* 		*adjust_count = *adjust_count - 1; */
/* 	} else if ((*sleep_cnt / *turn_count) > 120) { */
/* 		*turn_count = *turn_count + 1; */
/* 	} */

/* 	if (*adjust_count == 0) { */
/* 		printf("-------------------------------------------------------\n"); */
/* 		printf("adjust_complete, turn_count to (%d)\n", *turn_count); */
/* 		printf("-------------------------------------------------------\n"); */
/* 	} */
/* } */


/** 
 * 
 * 
 * @param query 
 * @param hostName 
 * @param nameServer 
 */
void setupDnsQuery(unsigned char **query, char *buf, char *hostName)
{
    unsigned char *host = (unsigned char*)hostName;
    int query_type = T_A;
    struct DNS_HEADER *dns = NULL;
    struct QUESTION *qinfo = NULL;

    //Set the DNS structure to standard queries
    dns = (struct DNS_HEADER *)buf;
 
    dns->id = (unsigned short) htons(getpid());
    dns->qr = 0; //This is a query
    dns->opcode = 0; //This is a standard query
    dns->aa = 0; //Not Authoritative
    dns->tc = 0; //This message is not truncated
    dns->rd = 1; //Recursion Desired
    dns->ra = 0; //Recursion not available! hey we dont have it (lol)
    dns->z = 0;
    dns->ad = 0;
    dns->cd = 0;
    dns->rcode = 0;
    dns->q_count = htons(1); //we have only 1 question
    dns->ans_count = 0;
    dns->auth_count = 0;
    dns->add_count = 0;
 
    //point to the query portion
    *query =(unsigned char*)&buf[sizeof(struct DNS_HEADER)];

    ChangetoDnsNameFormat(*query , host);
    //fill it
    qinfo =(struct QUESTION*)&buf[sizeof(struct DNS_HEADER) + (strlen((const char*)(*query)) + 1)];
    qinfo->qtype = htons(query_type); //type of the query , A , MX , CNAME , NS etc
    qinfo->qclass = htons(1); //its internet (lol)
}

/** 
 * 
 * 
 * @param buf 
 * @param nameServer 
 */
int runDnsQuery(char *buf, int maxBufSiz, char *qname, char *nameServer, int timeOutMil)
{
    int	s;
    int	i = 0;
    int	ret = 0;
    struct sockaddr_in dest;
    s = socket(AF_INET , SOCK_DGRAM , IPPROTO_UDP); //UDP packet for DNS queries
#if 1
	struct timeval opt_val = {0, 0};
    opt_val.tv_usec = timeOutMil * 1000;
	int opt_len = sizeof(opt_val);
	setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &opt_val, opt_len);
#endif
 
    dest.sin_family = AF_INET;
    dest.sin_port = htons(53);
    dest.sin_addr.s_addr = inet_addr(nameServer); //dns servers
  
    /* printf("Sending Packet...\n"); */
    if(sendto(s,(char*)buf,
              sizeof(struct DNS_HEADER) + (strlen((const char*)qname)+1) + sizeof(struct QUESTION),
              0,(struct sockaddr*)&dest,sizeof(dest)) < 0) {
        perror("sendto failed");
        ret = -1;
    }
    else {
		SEND_CNT++;
        /* printf("Done"); */
        //Receive the answer
        i = sizeof(dest);
        /* printf("Receiving answer...\n"); */
        if(recvfrom (s,(char*)buf, maxBufSiz, 0, (struct sockaddr*)&dest , (socklen_t*)&i ) < 0) {
            perror("recvfrom failed");
            ret = -1;
        }
        else {
            RECV_CNT++;
        }
    }
    /* printf("Done"); */
    return ret;
}

/** 
 * 
 * 
 * @param buf 
 */
void parseDnsResult(RES_RECORD *answers,RES_RECORD *auth, RES_RECORD *addit, unsigned char *buf, char *query)
{
    int	stop = 0;
    int	i = 0;
    int	j = 0;
    int	loopCnt = 0;
    int loopCnt2 = 0;
    unsigned char *reader = NULL;
    struct DNS_HEADER *dns = NULL;
    
    dns = (struct DNS_HEADER*) buf;
    //move ahead of the dns header and the query field
    reader = &buf[sizeof(struct DNS_HEADER) + (strlen((const char*)query)+1) + sizeof(struct QUESTION)];

    loopCnt = ntohs(dns->ans_count);
    for(i = 0; i < loopCnt; i++) {
        if(MAX_RES_RECORD_CNT == i) {
            break;
        }
        answers[i].name = ReadName(reader,buf,&stop);
        reader = reader + stop;
        answers[i].resource = (struct R_DATA*)(reader);
        reader = reader + sizeof(struct R_DATA);
        if(ntohs(answers[i].resource->type) == 1) { //if its an ipv4 address
            /* printf("answers i=%d resource->type=%d resource->data_len=%d alloc size=%d\n", */
            /*        i, ntohs(answers[i].resource->type), ntohs(answers[i].resource->data_len), */
            /*        ntohs(answers[i].resource->data_len) + 1); */
            answers[i].rdata = (unsigned char*)malloc(ntohs(answers[i].resource->data_len) + 1);
            answers[i].malloced = 1;
            for(j=0 ; j<ntohs(answers[i].resource->data_len) ; j++) {
                answers[i].rdata[j] = reader[j];
            }
            answers[i].rdata[ntohs(answers[i].resource->data_len)] = '\0';
            reader = reader + ntohs(answers[i].resource->data_len);
        }
        else {
            answers[i].rdata = ReadName(reader,buf,&stop);
            reader = reader + stop;
        }
    }
    
    loopCnt = ntohs(dns->auth_count);
    for(i = 0; i < loopCnt; i++) {
        if(MAX_RES_RECORD_CNT == i) {
            break;
        }
        auth[i].malloced = 1;
        auth[i].name = ReadName(reader,buf,&stop);
        reader+=stop;
        
        auth[i].resource = (struct R_DATA*)(reader);
        reader += sizeof(struct R_DATA);
        
        auth[i].rdata = ReadName(reader,buf,&stop);
        reader += stop;
    }
    
    loopCnt = ntohs(dns->add_count);
    for(i = 0; i < loopCnt; i++) {
        if(MAX_RES_RECORD_CNT == i) {
            break;
        }
        addit[i].malloced = 1;
        addit[i].name = ReadName(reader,buf,&stop);
        reader += stop;
        addit[i].resource = (struct R_DATA*)(reader);
        reader += sizeof(struct R_DATA);
        if(ntohs(addit[i].resource->type) == 1) {
            /* printf("addit i=%d resource->type=%d resource->data_len=%d alloc size=%d\n", */
            /*        i, ntohs(addit[i].resource->type), ntohs(addit[i].resource->data_len), */
            /*        ntohs(addit[i].resource->data_len) + 1); */
            addit[i].rdata = (unsigned char*)malloc(ntohs(addit[i].resource->data_len) + 1);

            loopCnt2 = ntohs(addit[i].resource->data_len);
            for(j = 0; j < loopCnt2; j++) {
                addit[i].rdata[j] = reader[j];
            }
            addit[i].rdata[ntohs(addit[i].resource->data_len)]='\0';
            reader += ntohs(addit[i].resource->data_len);
        }
        else {
            addit[i].rdata = ReadName(reader,buf,&stop);
            reader += stop;
        }
    }
}

/** 
 * 
 * 
 * @param resRecord 
 * @param maxResRecordCnt 
 */
void clearResRecord(RES_RECORD *resRecord, int maxResRecordCnt)
{
    int	i = 0;
    RES_RECORD *delRecord = NULL;
    for(i = 0; i < maxResRecordCnt; i++) {
        delRecord = &resRecord[i];
        if(delRecord->malloced) {
            /* printf("i=%d free\n", i); */
            if(delRecord->rdata) {
                /* printf("rdata=%p\n", delRecord->rdata); */
                free(delRecord->rdata);
            }
            if(delRecord->name) {
                /* printf("name=%p\n", delRecord->name); */
                free(delRecord->name);
            }
        }
    }
}
/** 
 * 
 * 
 * @param answers 
 * @param auth 
 * @param addit 
 */
void clearParseResult(RES_RECORD *answers, RES_RECORD *auth, RES_RECORD *addit)
{
    clearResRecord(answers, MAX_RES_RECORD_CNT);
    clearResRecord(auth, MAX_RES_RECORD_CNT);
    clearResRecord(addit, MAX_RES_RECORD_CNT);
}


/** 
 * 
 * 
 * @param ip 
 * @param hostName 
 * @param nameServer 
 * @param timeMil 
 * 
 * @return 
 */
int getHostFirstIpByNameTimeOut(char *ip, char *hostName, char *nameServer, int timeOutMil)
{
    unsigned char buf[MAX_DNS_BUFFER_SIZE];
    unsigned char *qname = NULL;
    int i = 0;
    int	ret = -1;
    struct sockaddr_in a;
    /* struct DNS_HEADER *dns = NULL; */
    RES_RECORD answers[MAX_RES_RECORD_CNT];
    RES_RECORD auth[MAX_RES_RECORD_CNT];
    RES_RECORD addit[MAX_RES_RECORD_CNT]; //the replies from the DNS server
	memset(answers, 0x00, sizeof(RES_RECORD) * 20);
	memset(auth, 0x00, sizeof(RES_RECORD) * 20);
	memset(addit, 0x00, sizeof(RES_RECORD) * 20);
    
    setupDnsQuery(&qname, (char*)&buf[0], hostName);
    if((ret = runDnsQuery((char*)&buf[0], MAX_DNS_BUFFER_SIZE, (char*)qname, nameServer, timeOutMil)) == 0) {
        parseDnsResult(answers, auth, addit, &buf[0], (char*)qname);
        /* dns = (struct DNS_HEADER*)buf; */
        /* printf("\nAnswer Records : %d \n" , ntohs(dns->ans_count) ); */
        if(ntohs(answers[i].resource->type) == T_A) {
            long *p;
            p=(long*)answers[i].rdata;
            a.sin_addr.s_addr=(*p); //working without ntohl
            /* printf("has IPv4 address : %s\n",inet_ntoa(a.sin_addr)); */
            strcpy(ip, inet_ntoa(a.sin_addr));
            ret = 0;
        }
        clearParseResult(answers, auth, addit);
    }
    return ret;
}

/** 
 * 
 * 
 * @param ip 
 * @param hostName 
 * @param nameServer 
 */
int getHostFirstIpByName(char *ip, char *hostName, char *nameServer)
{
    return getHostFirstIpByNameTimeOut(ip, hostName, nameServer, 10);
}
