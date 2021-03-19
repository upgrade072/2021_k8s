#ifndef __CONFLIB_H__
#define __CONFLIB_H__

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/errno.h>


#define CONFLIB_MAX_TOKEN_LEN	64

#define eCONF_NOT_EXIST_FILE    -1
#define eCONF_NOT_EXIST_SECTION -2
#define eCONF_NOT_EXIST_KEYWORD -3
#define eCONF_NOT_EXIST_TOKEN   -4
#define eCONF_NOT_FOUND_ENV		-5


#define	LIB_INIT_SYSCONF		(1 << 0)
#define	LIB_INIT_IXPC_QUEUE		(1 << 1)
#define	LIB_INIT_MSGQ			(1 << 2)
#define LIB_INIT_SHM_SYS		(1 << 3)
#define LIB_INIT_KEEPALIVE		(1 << 4)
#define	LIB_INIT_ALL			( LIB_INIT_SYSCONF | \
								LIB_INIT_IXPC_QUEUE | \
								LIB_INIT_MSGQ | \
								LIB_INIT_SHM_SYS | \
								LIB_INIT_KEEPALIVE)

//extern int errno;
extern char *homeEnv, l_sysconf[], l_dataconf[];
extern char mySysName[];
extern int	mySysId;	/* 2011.09.22 */


extern int conflib_getNthTokenInFileSection (const char*, char*, char*, int, char*);
extern int conflib_getTokenCntInFileSection (char*, char*, char*);
extern int conflib_getNTokenInFileSection (char*, char*, char*, int, char[][CONFLIB_MAX_TOKEN_LEN]);
extern int conflib_getStringInFileSection (char*, char*, char*, char*);
extern int conflib_seekSection (FILE*, char*);
extern int conflib_getStringInSection (FILE*, char*, char*);
extern int conflib_initConfigData(void);
int conflib_get_app_num (const char *app_name);

const char* conflib_get_home (void);
char* conflib_get_sysconf (void);
int conflib_get_sysconf_contents (char *section, char *keyword, int  n, char *string); 
int conflib_get_token (const char *fname, char *section, char *keyword, char *buf);
int conflib_get_contents (char *file, char *section, char *keyword, char *buf);
int comlib_start (char *app_name, unsigned int options);


#endif /*__CONFLIB_H__*/
