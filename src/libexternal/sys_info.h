#ifndef _SYS_INFO_H_
#define _SYS_INFO_H_

#include <stdint.h>

#define	SHM_SYS_INFO	"SHM_SYS_INFO"

struct sys_info {
	uint8_t	ha_enable;	
	uint8_t	ha_active;
	uint8_t ha_reserve;
};

typedef struct sys_info sys_info_t;

#ifdef __cplusplus
extern "C" {
#endif

struct sys_info* sys_init_shm (int *created);

#ifndef CONFIG_NO_SYS_HA
int sys_ha_check_active (void);
int sys_ha_get_mode (void);
void sys_ha_set_active (void);
void sys_ha_unset_active (void);
void sys_ha_set_mode (uint8_t mode);
int  sys_ha_get_enable (void);
void sys_ha_set_enable (void);
void sys_ha_unset_enable (void);
int  sys_ha_get_reserve (void);
void sys_ha_set_reserve (void);
void sys_ha_unset_reserve (void);
#endif	/* CONFIG_NO_SYS_HA */

#ifdef __cplusplus
}
#endif

#endif	/* _SYS_INFO_H_*/
