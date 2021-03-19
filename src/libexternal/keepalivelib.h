#ifndef __KEEPALIVELIB_H__
#define __KEEPALIVELIB_H__

#include "sysconf.h"

#ifndef _NEW_KEEPALIVE
typedef struct {
    int     cnt[SYSCONF_MAX_APPL_NUM];
    int     oldcnt[SYSCONF_MAX_APPL_NUM];
    int     retry[SYSCONF_MAX_APPL_NUM];
} T_keepalive;
#endif	/*** __ifdnef _NEW_KEEPALIVE ***/	

#ifdef _NEW_KEEPALIVE
typedef struct {
	char 	prcName[16];
    int     cnt;
    int     oldcnt;
    int     retry;
} keepinfo;

typedef struct 
{
	int 		procCnt;
	keepinfo 	info[SYSCONF_MAX_APPL_NUM];
	
} T_keepalive;
#endif	/*** __ifdef _NEW_KEEPALIVE ***/	


#ifndef _NEW_KEEPALIVE
extern int keepalivelib_init (char*);
extern void keepalivelib_increase (void);
#endif	/*** __ifdnef _NEW_KEEPALIVE ***/	

#ifdef _NEW_KEEPALIVE
extern int keepalivelib_init(char*);
extern void keepalivelib_increase(void);
#endif	/*** __ifdef _NEW_KEEPALIVE ***/	

extern int getkeepAliveIdx(char *processName);

#endif /*__KEEPALIVELIB_H__*/
