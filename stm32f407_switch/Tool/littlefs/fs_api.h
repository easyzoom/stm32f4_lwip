#ifndef __FS_API_H__
#define __FS_API_H__


#include <stdint.h>
#include "lfs.h"

#include "config.h"

#define GD32_FLASH_ERASE_GRAN              128*1024
#define GD32_FLASH_NUM_GRAN                4
#define GD32_FLASH_FLLESYS_START_BASE      (0x08000000 + GD32_FLASH_ERASE_GRAN*GD32_FLASH_NUM_GRAN)

#define FILE_SYSTEM_STR             lfs_t
#define FILE_STR                    lfs_file_t

#define FILE_BASIC_INFO             "basic_info.json"
#define FILE_PORT_MANAGE            "port_manage.json"


extern uint8_t fs_api_init( void );
extern void fs_api_lock_init( void );
extern void fs_api_lock( void );
extern void fs_api_unlock( void );



extern int fs_api_lfs_ls( FILE_SYSTEM_STR *lfs, const char *path );
extern int fs_api_ls( void );
extern int fs_api_mount( void );
extern int fs_api_fopen( FILE_STR *fp, char *path, char *mode );
extern int fs_api_fclose( FILE_STR *fp );
extern int32_t fs_api_fread( FILE_STR *fp, uint32_t size, void *dst );
extern int32_t fs_api_fwrite( FILE_STR *fp, uint32_t size, void *src );
extern int32_t fs_api_fseek( FILE_STR *fp, uint32_t offset, uint32_t from );
extern int32_t fs_api_fsize( FILE_STR *fp );
extern int fs_api_fremove( char *path );
extern int fs_api_format(void);
extern int fs_api_truncate(FILE_STR *fp ,uint32_t size);



#endif /*__FS_API_H__*/

