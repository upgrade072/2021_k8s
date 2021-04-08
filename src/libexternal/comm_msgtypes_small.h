
#ifndef __COMM_MSGTYPES_H__
#define __COMM_MSGTYPES_H__

#define COMM_MAX_NAME_LEN   16
#define COMM_MAX_VALUE_LEN  100
 
typedef long    genq_mtype_t;
typedef struct general_qmsg{
	genq_mtype_t    mtype;
#define MAX_GEN_QMSG_LEN        (81920-sizeof(long))
	char            body[MAX_GEN_QMSG_LEN];
} GeneralQMsgType;
#define GEN_QMSG_LEN(length)    (sizeof (genq_mtype_t) + length + 1)

typedef struct ixpc_header {
	int         msgId;   /* / message_id */
	char        segFlag; /* / segment flag -> 한 메시지가 너무 길어 한번에 보내지 못할때 사용된다.*/
	char        seqNo;   /* / sequence number -> segment된 경우 일련번호 */
	char        dummy[2];
#define BYTE_ORDER_TAG  0x1234
	short       byteOrderFlag;
	short       bodyLen;
	char        srcSysName[COMM_MAX_NAME_LEN];
	char        srcAppName[COMM_MAX_NAME_LEN];
	char        dstSysName[COMM_MAX_NAME_LEN];
	char        dstAppName[COMM_MAX_NAME_LEN];
} IxpcQMsgHeadType;
#define IXPC_HEAD_LEN   sizeof(IxpcQMsgHeadType)

typedef struct ixpc_qmsg {
	IxpcQMsgHeadType    head;
#define MAX_IXPC_QMSG_LEN   ((MAX_GEN_QMSG_LEN)-sizeof(IxpcQMsgHeadType))
	char                body[MAX_IXPC_QMSG_LEN];        /*  MMLReqMsgType   */
} IxpcQMsgType;
#define IXPC_QMSG_LEN(body_len) (sizeof(IxpcQMsgHeadType) + body_len)

#endif /*__COMM_MSGTYPES_H__*/
