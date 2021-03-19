#ifndef __SYSCONF_H__
#define __SYSCONF_H__

/*------------------------------------------------------------------------------
// 모든 시스템에서 공통으로 사용되는 Configuration에 관련된 정보를 선언한다.
//------------------------------------------------------------------------------*/

#define	MY_SYS_NAME						"MY_SYS_NAME"
#define IV_HOME							"IV_HOME"
#define	SYS_MP_ID						"SYS_MP_ID"
#define SYSCONF_FILE					"data/sysconfig"
#define ASSOCONF_FILE                   "data/associate_config"
#define DATACONF_FILE					"data/dataconfig"
#define APPCONF_DIR                     "data/CONFIG"
#define EXCONN_CONF_FILE                "data/CONFIG/extconn.conf"
#define DATA_DIR                     	"data"


//#define SYSCONF_MAX_APPL_NUM			64	/* 시스템당 application process 최대 갯수 */
#define SYSCONF_MAX_APPL_NUM			75	/* 시스템당 application process 최대 갯수 */
#define SYSCONF_MAX_ASSO_SYS_NUM		32	/* MPA, MPB, OMP	관리대상 시스템 최대 갯수 */
#define SYSCONF_MAX_SYS_TYPE_NUM		2	/* MP, OMP			시스템 type 최대 갯수 */
#define SYSCONF_MAX_SYS_TYPE_MEMBER		9	/* MP->MPA, MPB	type당 시스템 최대 갯수 */
#define SYSCONF_MAX_GROUP_NUM			2	/* MP, OMP 			cluster등 다중화된 시스템 group 최대 갯수 */
#define SYSCONF_MAX_GROUP_MEMBER		9	/* MP->MPA, MPB 	그룹당 시스템 member 최대 갯수 */

#define SYSCONF_MAX_OMAF_PROC_NUM		5
#define	SYSCONF_MAX_EHTTPF_THREAD		20	/* 2011.12.02. 16->20 */
#define	SYSCONF_MAX_DOMF_THR_NUM		16
#define	SYSCONF_MAX_MGMF_THR_NUM		8
#define	SYSCONF_MAX_MULTIQ_NUM			3

#define PROC_PATH			            "/proc"

/* sysconfig 파일에 등록되는 system_type들을 정의한다.
// - FIMD에서 여기에 정의된 이름만 인식한다.
//
*/
#define SYSCONF_SYSTYPE_OMP				"OMP"
#define SYSCONF_SYSTYPE_MP				"MP"



#define	SYSCONF_GROUPTYPE_OMP			0
#define	SYSCONF_GROUPTYPE_MP			1

#define MODE_ACTIVE						1
#define MODE_STANDBY					2
#define MODE_BACKUP						3
#define MODE_SWITCHED					4
#define MODE_NOT_EQUIP					5
#define SYSCONF_SYSSTATE_ACTIVE         0
#define SYSCONF_SYSSTATE_BACKUP         1
#define SYSCONF_SYSSTATE_SWITCHED       2
#define SYSCONF_SYSSTATE_STR_ACTIVE     "ACTIVE"
#define SYSCONF_SYSSTATE_STR_STANDBY    "STANDBY"
#define SYSCONF_SYSSTATE_STR_BACKUP     "BACKUP"
#define SYSCONF_SYSSTATE_STR_SWITCHED   "SWITCHED"  /* STANDBY --> ACTIVE */

#ifndef HPUX
#define         SHM_FAILED  -1
#define         MAXSIG      64
#endif

#endif /*__SYSCONF_H__*/

