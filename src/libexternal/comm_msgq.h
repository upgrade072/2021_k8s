#ifndef __COMM_MSGQ_H__
#define __COMM_MSGQ_H__

#include <sys/msg.h>
#include <conflib.h>

#define MAX_MSGQ_LEN        32768 //16384

extern  int commlib_getMsgQKey(const char *l_sysconf, char *appName, int *key, int isPrimary);
extern  int commlib_crteMsgQ (const char *l_sysconf, char *appName, const int initFlag);
extern  int commlib_crteSecondMsgQ (const char *l_sysconf, char *appName, const int initFlag);
extern  int commlib_crteMsgQByKey (const int qkey, const int initFlag);
extern  int commlib_crteNthMsgQ (const char *l_sysconf, char *appName, int nth, const int initFlag);
extern  int commlib_msgget(int key, int initFlag);
extern  int msgq_create (char *app_name, const int init_flag);
extern  int msgq_send (int msgq_id, char *msg, int size);
extern  int msgq_init (char *app_name, int init_flag);
extern  int msgq_recv (char *msg, int size);
extern  int msgq_get_id (void);

int ixpc_que_get_id (void);

struct general_qmsg;
int ixpc_que_send_ex (struct general_qmsg *snd, int mtype, int msgid, char *dest, int length, int seg_flag, int seq);
int ixpc_que_send (struct general_qmsg *snd, int mtype, int msgid, char *dest, int length);
int ixpc_que_send_to_cond (int mtype, int msgid, char *text);

#endif /*__COMM_MSGQ_H__*/
