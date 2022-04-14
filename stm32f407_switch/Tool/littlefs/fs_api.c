#include "string.h"
#include "fs_api.h"
#include "cmsis_os.h"
#include "semphr.h"
#include "kubot_debug.h"

#define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) /* Base @ of Sector 0, 16 Kbyte */
#define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08004000) /* Base @ of Sector 1, 16 Kbyte */
#define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08008000) /* Base @ of Sector 2, 16 Kbyte */
#define ADDR_FLASH_SECTOR_3     ((uint32_t)0x0800C000) /* Base @ of Sector 3, 16 Kbyte */
#define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08010000) /* Base @ of Sector 4, 64 Kbyte */
#define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08020000) /* Base @ of Sector 5, 128 Kbyte */
#define ADDR_FLASH_SECTOR_6     ((uint32_t)0x08040000) /* Base @ of Sector 6, 128 Kbyte */
#define ADDR_FLASH_SECTOR_7     ((uint32_t)0x08060000) /* Base @ of Sector 7, 128 Kbyte */
#define ADDR_FLASH_SECTOR_8     ((uint32_t)0x08080000) /* Base @ of Sector 8, 128 Kbyte */
#define ADDR_FLASH_SECTOR_9     ((uint32_t)0x080A0000) /* Base @ of Sector 9, 128 Kbyte */
#define ADDR_FLASH_SECTOR_10    ((uint32_t)0x080C0000) /* Base @ of Sector 10, 128 Kbyte */
#define ADDR_FLASH_SECTOR_11    ((uint32_t)0x080E0000) /* Base @ of Sector 11, 128 Kbyte */

static int32_t fs_api_flash_write( const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer,
                             lfs_size_t size );
static int32_t fs_api_flash_erase( const struct lfs_config *c, lfs_block_t block );
static int32_t fs_api_flash_sync( const struct lfs_config *c );
int32_t fs_api_flash_read( const struct lfs_config *c, lfs_block_t block,  lfs_off_t off, void *buffer, lfs_size_t size );

static SemaphoreHandle_t    fs_api_sem;
static FILE_SYSTEM_STR      fs_api_fs;

__align(4) static uint8_t read_buffer[16];
__align(4) static uint8_t prog_buffer[16];
__align(4) static uint8_t lookahead_buffer[16];

const struct lfs_config lfs_cfg =
{
    // block device operations
    .read  = fs_api_flash_read,
    .prog  = fs_api_flash_write,
    .erase = fs_api_flash_erase,
    .sync  = fs_api_flash_sync,

    // block device configuration
    .read_size = 16,
    .prog_size = 16,
    .block_size = GD32_FLASH_ERASE_GRAN,
    .block_count = GD32_FLASH_NUM_GRAN,
    .cache_size = 16,
    .lookahead_size =  16,
    .block_cycles = 500,
    
    .read_buffer = read_buffer,
    .prog_buffer = prog_buffer,
    .lookahead_buffer = lookahead_buffer,
};


uint16_t gd32_flash_read_halfword(uint32_t addr)
{
    return *(volatile uint16_t*)addr; 
}


void gd32_flash_write(uint32_t writeaddr, uint16_t *pbuffer, uint32_t num)   
{
    for(uint32_t i=0; i < num; i++)
    {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, writeaddr, pbuffer[i]);
        writeaddr+=2;
    }
}


/**
 * @brief ��ʼ���ļ�ϵͳ�ĵײ�ӿ�
 * 
 * @param void
 * @return void
 */
uint8_t fs_api_init( void )
{
    int8_t state = 0;
    
    fs_api_lock_init();
    state = fs_api_mount();

    if(state >= 0)
    {
        state = fs_api_ls();
    }
    if(state >= 0)
    {
        state = 0;
        LOG_PRINT_INFO("File system init success\r\n");
    }
    else
    {
        state = 1;
        LOG_PRINT_ERROR("File system init failure\r\n");
    }

    return state;
}

/**
 * @brief �����ٽ绥���ź���
 * 
 * @param void
 * @return void
 */
void fs_api_lock_init( void )
{
  fs_api_sem = xSemaphoreCreateMutex();
}

/**
 * @brief �ٽ���
 * 
 * @param void
 * @return void
 */
void fs_api_lock( void )
{
    xSemaphoreTake( fs_api_sem, portMAX_DELAY );
}

/**
 * @brief �ٽ����
 * 
 * @param void
 * @return void
 */
void fs_api_unlock( void )
{
    xSemaphoreGive( fs_api_sem );
}

/**
 * @brief ��flash��������
 * 
 * @param [in] c lfs_config���ݽṹ
 * @param [in] block Ҫ���Ŀ�
 * @param [in] off �ڵ�ǰ���ƫ��
 * @param [out] buffer ��ȡ��������
 * @param [in] size Ҫ��ȡ���ֽ���
 * @return 0 �ɹ� <0 ����
 */
int32_t fs_api_flash_read( const struct lfs_config *c, lfs_block_t block,  lfs_off_t off, void *buffer, lfs_size_t size )
{
    uint16_t *temp = buffer;
    uint32_t addr = 0;

    addr = GD32_FLASH_FLLESYS_START_BASE + c->block_size * block + off;
    
    for(int i =0; i < (size/2); i++)
    {
        temp[i] = gd32_flash_read_halfword(addr);
        addr+=2;
    }
    
    return 0;
}

/**
 * @brief ����д��flash
 * 
 * @param [in] c lfs_config���ݽṹ
 * @param [in] block Ҫ���Ŀ�
 * @param [in] off �ڵ�ǰ���ƫ��
 * @param [out] buffer ��ȡ��������
 * @param [in] size Ҫ��ȡ���ֽ���
 * @return 0 �ɹ� <0 ����
 */
static int32_t fs_api_flash_write( const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer,
                             lfs_size_t size )
{
    uint32_t addr = 0;

    addr = GD32_FLASH_FLLESYS_START_BASE + c->block_size * block + off;
    HAL_FLASH_Unlock();
    gd32_flash_write(addr, (uint16_t *)buffer, size/2);
    HAL_FLASH_Lock();
    
    return 0;
}

/**
 * @brief ����flash��һ��block
 * 
 * @param [in] c lfs_config���ݽṹ
 * @param [in] block Ҫ�����Ŀ�
 * @return 0 �ɹ� <0 ����
 */
static int32_t fs_api_flash_erase( const struct lfs_config *c, lfs_block_t block )
{
    uint32_t sector = block + 8;
    
    printf("bk:%d\r\n", block);
    switch(sector)
    {
        case 8:
            FLASH_Erase_Sector(ADDR_FLASH_SECTOR_8, FLASH_VOLTAGE_RANGE_3);
        break;
        case 9:
            FLASH_Erase_Sector(ADDR_FLASH_SECTOR_9, FLASH_VOLTAGE_RANGE_3);
        break;
        case 10:
            FLASH_Erase_Sector(ADDR_FLASH_SECTOR_10, FLASH_VOLTAGE_RANGE_3);
        break;
        case 11:
            FLASH_Erase_Sector(ADDR_FLASH_SECTOR_11, FLASH_VOLTAGE_RANGE_3);
        break;
    }
    
    return 0;
}

/**
 * @brief ͬ���洢�ӿ�
 * 
 * @param [in] c lfs_config���ݽṹ
 * @return 0 �ɹ� <0 ����
 */
static int32_t fs_api_flash_sync( const struct lfs_config *c )
{
    return 0;
}

/**
 * @brief �����ļ�����ʾ��Ŀ
 * 
 * @param [in] *lfs �ļ�ϵͳ���
 * @param [in] *path �ļ�·��
 * @return 0 �ɹ� <0 ����
 */
int fs_api_lfs_ls( FILE_SYSTEM_STR *lfs, const char *path )
{
        struct lfs_info info;
        lfs_dir_t fs_api_dir;
    
    int err = lfs_dir_open( lfs, &fs_api_dir, path );
    if( err )
    {
        return err;
    }
        
    while( true )
    {
        int res = lfs_dir_read( lfs, &fs_api_dir, &info );
        if( res < 0 )
        {
            return res;
        }
        if( res == 0 )
        {
            break;
        }
        LOG_PRINT_INFO( "\t%s", info.name );
        switch( info.type )
        {
            case LFS_TYPE_REG:
                LOG_PRINT_INFO( "\t\t\t\t%u Byte \r\n", info.size );
                break;
            case LFS_TYPE_DIR:
                LOG_PRINT_INFO( "\t\t\t\t%s Dir\r\n" ,info.name);
                break;
            default:
                LOG_PRINT_INFO( "?\r\n" );
                break;
        }
    }
    err = lfs_dir_close( lfs, &fs_api_dir );
    if( err )
    {
        return err;
    }
        
    return 0;
}

/**
 * @brief �ļ�ϵͳ����
 * 
 * @param void
 * @return void
 */
int fs_api_mount( void )
{
    FILE_STR fp;
    
    int err;

    err = lfs_mount( &fs_api_fs, &lfs_cfg );
    if( !err )
    {
        err = fs_api_fopen( &fp, FILE_BASIC_INFO, "r" );
        if( !err )
        {
                err = fs_api_fclose(&fp);
        }
        
        err = fs_api_fopen( &fp, FILE_PORT_MANAGE, "r" );
        if( !err )
        {
                err = fs_api_fclose(&fp);
        }
    }
        //ϵͳ�״�������Ϊ�����ڸ�Ŀ¼���ʧ�ܣ��ȸ�ʽ���ٳ��Դ�
    if( err )
    {
        LOG_PRINT_WARIN( "File system start Format!!\r\n" );
        err = lfs_format( &fs_api_fs, &lfs_cfg );
        if(err < 0)
        {
            LOG_PRINT_ERROR( "File system format failure!!\r\n");
            return -1;
        }
        err = lfs_mount( &fs_api_fs, &lfs_cfg );
        if(err < 0)
        {
            LOG_PRINT_ERROR( "File system mount failure!!\r\n");
            return -2;
        }
        err =fs_api_fopen( &fp, FILE_BASIC_INFO, "c" );
        if(err < 0)
        {
            LOG_PRINT_ERROR( "FILE_BASIC_INFO creat failure!!\r\n");
            return -3;
        }
        err = fs_api_fclose(&fp);
        if(err < 0)
        {
            LOG_PRINT_ERROR( "FILE_BASIC_INFO close failure!!\r\n");
            return -4;
        }
        err =fs_api_fopen( &fp, FILE_PORT_MANAGE, "c" );
        if(err < 0)
        {
            LOG_PRINT_ERROR( "FILE_PORT_MANAGE creat failure!!\r\n");
            return -5;
        }
        err = fs_api_fclose(&fp);
        if(err < 0)
        {
            LOG_PRINT_ERROR( "FILE_PORT_MANAGE close failure!!\r\n");
            return -6;
        }
    }
    LOG_PRINT_INFO( "File system mounted\r\n" );
    
    return 0;
}

/**
 * @brief ��ʾĿ¼
 * 
 * @param void
 * @return void
 */
#define ls    fs_api_ls
int fs_api_ls( void )
{
        int err = 0;
    
    LOG_PRINT_INFO( "\r\nfiles on [\\]\r\n" );
    fs_api_lock( );
    err = fs_api_lfs_ls( &fs_api_fs,  "/" );
    fs_api_unlock( );
    LOG_PRINT_INFO( "\r\n\r\n" );
    
        return err;
}

/**
 * @brief ���ļ�
 * 
 * @param [in] *fp �ļ�ϵͳ���
 * @param [in] *path �ļ�·��
 * @param [in] *mode ����ģʽ
 * @return void
 */
int fs_api_fopen( FILE_STR *fp, char *path, char *mode )
{
    int res = 0;
    uint32_t create = 0, modify = 0, state = 0;
    uint32_t wr = 0;
    
    if( strstr( mode, "c" ) != 0 )
        create = 1;
    if( strstr( mode, "+" ) != 0 )
        modify = 1;
    if( strstr( mode, "wr" ) != 0 )
        wr = 1;
    
    if( create )
        state |= (LFS_O_RDWR |LFS_O_CREAT);
    
    if( modify )
        state |= LFS_O_APPEND;
    
    if( wr )
        state |= LFS_O_RDWR;
    
    if( state == 0 )
        state |= LFS_O_RDONLY;

    
    
    fs_api_lock();
    res = lfs_file_open( &fs_api_fs, fp, path, state );
    fs_api_unlock();
    if( res )
        res = -1;

        LOG_PRINT_INFO("\nFOPEN(%s, %s) = %d\r\n", path, mode, res);
        
    return res;
}

/**
 * @brief �ر��ļ�
 * 
 * @param [in] *fp �ļ�ϵͳ���
 * @return 0 �ɹ� <0 ����
 */
int fs_api_fclose( FILE_STR *fp )
{
    int res = 0;
    LOG_PRINT_INFO("\nFCLOSE(fp:%d)\r\n", *fp);

    fs_api_lock();
    res = lfs_file_close( &fs_api_fs, fp );
    fs_api_unlock();

    return res;
}

/**
 * @brief ���ļ�
 * 
 * @param [in] *fp �ļ�ϵͳ���
 * @param [in] size ���������ݴ�С��
 * @param [out] *dst ���������ݻ���
 * @return 0 �ɹ� <0 ����
 */
int32_t fs_api_fread( FILE_STR *fp, uint32_t size, void *dst )
{
    int32_t ret = 0;

    fs_api_lock();
    ret = lfs_file_read( &fs_api_fs, fp, dst, size );
    fs_api_unlock();

    LOG_PRINT_INFO("\r\nFREAD(fp:%08x, %u) = %d b\r\n", fp, size, ret);
    return ret;
}

/**
 * @brief д�ļ�
 * 
 * @param [in] *fp �ļ�ϵͳ���
 * @param [in] size д������ݴ�С��
 * @param [in] *src д������ݻ���
 * @return 0 �ɹ� <0 ����
 */
int32_t fs_api_fwrite( FILE_STR *fp, uint32_t size, void *src )
{
    uint32_t ret = 0;

    fs_api_lock();
    ret = lfs_file_write( &fs_api_fs, fp, src, size );
    fs_api_unlock();

    LOG_PRINT_INFO("\nFWRITE(fp:%08x, %d) = %d\n", fp, size, ret);
    
    return ret;
}

/**
 * @brief �޸��ļ����λ��
 * 
 * @param [in] *fp �ļ�ϵͳ���
 * @param [in] offset ƫ����
 * @param [in] from ���Ŀ�ʼ
 * @return 0 �ɹ� <0 ����
 */
int32_t fs_api_fseek( FILE_STR *fp, uint32_t offset, uint32_t from )
{
    uint32_t p;

    fs_api_lock();
    p = lfs_file_seek( &fs_api_fs, fp, offset, from );
    fs_api_unlock();
    LOG_PRINT_INFO("\nFSEEK(fp:0x%08x, %d, %d) = %d\r\n", fp, offset, from, p);
    
    return p;
}

/**
 * @brief ��ȡ�ļ���С
 * 
 * @param [in] *fp �ļ�ϵͳ���
 * @return 0 �ɹ� <0 ����
 */
int32_t fs_api_fsize( FILE_STR *fp )
{
    uint32_t ret;
    
// ret = fs_api_fseek( fp, 0, LFS_SEEK_END );
    ret = lfs_file_size(&fs_api_fs, fp);
    LOG_PRINT_INFO("\nFSIZE(fp:0x%08x) = %d\r\n", fp, ret);
    
    return ret;
}

/**
 * @brief ɾ���ļ���Ŀ¼
 * 
 * @param [in] *path �ļ�·��
 * @return 0 �ɹ� <0 ����
 */
int fs_api_fremove( char *path )
{
    int res = 0;

    fs_api_lock();
    res = lfs_remove( &fs_api_fs, path );
    fs_api_unlock();

    return res;
}

/**
 * @brief ��ʽ���ļ�ϵͳ
 * 
 * @param [in] *path �ļ�·��
 * @return 0 �ɹ� <0 ����
 */
int fs_api_format(void)
{
    int res = 0;

    res = lfs_format( &fs_api_fs, &lfs_cfg );

    if(res < 0)
    {
        res = -1;
    }
    
    return res;
}

/**
 * @brief �ض��ļ�
 * 
 * @param [in] sise �ضϺ���ļ���С
 * @return 0 �ɹ� <0 ����
 */
int fs_api_truncate(FILE_STR *fp ,uint32_t size)
{
    int res = 0;

    res = lfs_file_truncate( &fs_api_fs, fp ,size);

    if(res < 0)
    {
        res = -1;
    }
    
    return res;
}

