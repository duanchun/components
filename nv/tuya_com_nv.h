

/**
****************************************************************************
* @file      tuya_com_nv.h
* @brief     tuya_com_nv
* @author    xiaojian
* @version   V0.0.1
* @date      2021-11
* @note
******************************************************************************
* @attention
*
* <h2><center>&copy; COPYRIGHT 2021 Tuya </center></h2>
*/

#ifndef __TUYA_COM_NV_H__
#define __TUYA_COM_NV_H__

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum {
	TUYA_COM_NV_LID_NOT_EXIST        = 1,
	TUYA_COM_NV_ITEM_SIZE_INVALID    = 2,
	TUYA_COM_NV_FLASH_OTP_FAILED     = 3,
	TUYA_COM_NV_PARSE_OFFSET_FAILED  = 4
}TUYA_COM_NV_ERROR_E;

typedef struct {
	unsigned short  nv_item_lid;//
	unsigned short  total;//
	unsigned short  size;//
	const char      *verno;//
	const char      *item_name;//
}tuya_com_nv_item_t;

typedef unsigned char (*nv_flash_read)(const unsigned int addr, void* buf, const unsigned int size);
typedef unsigned char (*nv_flash_write)(const unsigned int addr, const void* buf, const unsigned int size);

unsigned char tuya_com_nv_init(unsigned int start_addr,unsigned int max_size,nv_flash_read read,nv_flash_write write, tuya_com_nv_item_t nv_items[], unsigned short nv_items_size);
//nRecordId 从0开始
unsigned char tuya_com_nv_write(unsigned short nLID, unsigned short nRecordId, const void *pBuffer, unsigned short nBufferSize, int *pError);
unsigned char tuya_com_nv_read(unsigned short nLID, unsigned short nRecordId, void *pBuffer, unsigned short nBufferSize, int *pError);

#ifdef __cplusplus
}
#endif

#endif



