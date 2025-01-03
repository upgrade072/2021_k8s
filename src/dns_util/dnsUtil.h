/**
 * @file   dnsUtil.h
 * @author night1700 <night1700@night1700-15ZB970-GP50ML>
 * @date   Tue Mar  9 11:11:45 2021
 * 
 * @brief  
 * 
 * 
 */

#ifndef _DNS_UTIL_H_
#define _DNS_UTIL_H_

#define T_A 1 //Ipv4 address
#define T_NS 2 //Nameserver
#define T_CNAME 5 // canonical name
#define T_SOA 6 /* start of authority zone */
#define T_PTR 12 /* domain name pointer */
#define T_MX 15 //Mail server

#define MAX_DNS_BUFFER_SIZE	65536
 
//DNS header structure
struct DNS_HEADER
{
    unsigned short id; // identification number
 
    unsigned char rd :1; // recursion desired
    unsigned char tc :1; // truncated message
    unsigned char aa :1; // authoritive answer
    unsigned char opcode :4; // purpose of message
    unsigned char qr :1; // query/response flag
 
    unsigned char rcode :4; // response code
    unsigned char cd :1; // checking disabled
    unsigned char ad :1; // authenticated data
    unsigned char z :1; // its z! reserved
    unsigned char ra :1; // recursion available
 
    unsigned short q_count; // number of question entries
    unsigned short ans_count; // number of answer entries
    unsigned short auth_count; // number of authority entries
    unsigned short add_count; // number of resource entries
};

//Constant sized fields of query structure
struct QUESTION
{
    unsigned short qtype;
    unsigned short qclass;
};
 
//Constant sized fields of the resource record structure
#pragma pack(push, 1)
struct R_DATA
{
    unsigned short type;
    unsigned short _class;
    unsigned int ttl;
    unsigned short data_len;
};
#pragma pack(pop)
 
//Pointers to resource record contents
typedef struct _RES_RECORD_
{
    unsigned char *name;
    struct R_DATA *resource;
    unsigned char *rdata;
    int	malloced;
} RES_RECORD;

#define MAX_RES_RECORD_CNT	20
//Structure of a Query
typedef struct
{
    unsigned char *name;
    struct QUESTION *ques;
} QUERY;

//Function Prototypes
extern int getHostFirstIpByNameTimeOut(char *ip, char *hostName, char *nameServer, int timeOutMil);
extern int getHostFirstIpByName(char *ip, char *hostName, char *nameServer);

/* extern char dns_servers[10][100]; */
extern int SEND_CNT;
extern int RECV_CNT;

#endif
