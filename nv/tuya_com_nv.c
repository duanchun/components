
#include "tuya_com_nv.h"
#include "string.h"
#include "tuya_log.h"

#define TUYA_INVALID_NV_OFFSET    0XFFFFFFFF

typedef struct {
	unsigned int          start_addr;
	unsigned short        max_size;
	nv_flash_read         read;
	nv_flash_write        write;
	tuya_com_nv_item_t    *nv_items;
	unsigned short        nv_items_size;         
}tuya_core_nv_t;

static tuya_core_nv_t s_core_nv = {0};

static unsigned int tuya_core_all_nv_size_get(void)
{
	int index = 0;
	unsigned int nv_size = 0;

	for(index = 0;index < s_core_nv.nv_items_size;index++)
	{
		nv_size += s_core_nv.nv_items[index].size*s_core_nv.nv_items[index].total;
	}
	TUYA_LOG_I(MOD_NV,"All NV size==%u",nv_size);
	return nv_size;
}

static unsigned int tuya_core_nv_item_offet_get(unsigned short nLID, unsigned short nRecordId)
{
	int index = 0;
	unsigned int offset = 0;

	for(index = 0;index < s_core_nv.nv_items_size;index++)
	{
		if(nLID == s_core_nv.nv_items[index].nv_item_lid)
		{
			offset += s_core_nv.nv_items[index].size*(nRecordId);
			break;
		}
		offset += s_core_nv.nv_items[index].size*s_core_nv.nv_items[index].total;
	}

	if(index == s_core_nv.nv_items_size)
	{
		TUYA_LOG_E(MOD_NV,"invalid nLID input");
		return TUYA_INVALID_NV_OFFSET;
	}
	TUYA_LOG_I(MOD_NV,"offset==%u",offset);
	return offset;
}

static tuya_com_nv_item_t * tuya_core_nv_get_item_info_with_lid(unsigned short nLID)
{
	int index = 0;

	for(index = 0;index < s_core_nv.nv_items_size;index++)
	{
		if(s_core_nv.nv_items[index].nv_item_lid == nLID)
		{
			return &s_core_nv.nv_items[index];
		}
	}
	TUYA_LOG_W(MOD_NV,"map nv failed");
	return NULL;
	
}

unsigned char tuya_com_nv_read(unsigned short nLID, unsigned short nRecordId, void *pBuffer, unsigned short nBufferSize, int *pError)
{
    unsigned char ret = 0;
	tuya_com_nv_item_t *pItem= NULL;
	unsigned int offset = 0;
    int error = 0;

	pItem = tuya_core_nv_get_item_info_with_lid(nLID);

	do
	{
		if(NULL == pItem)
		{
			TUYA_LOG_W(MOD_NV,"nv lid not exist");
			error = TUYA_COM_NV_LID_NOT_EXIST;
			ret = 1;
			break;
		}

		if(nBufferSize > pItem->size)
		{
			TUYA_LOG_W(MOD_NV,"Invalid nv nv size");
			error = TUYA_COM_NV_ITEM_SIZE_INVALID;
			ret = 1;
			break;
		}

		offset = tuya_core_nv_item_offet_get(nLID,nRecordId);
		if(TUYA_INVALID_NV_OFFSET == offset)
		{
			TUYA_LOG_W(MOD_NV,"nv offset parse failed");
			error = TUYA_COM_NV_PARSE_OFFSET_FAILED;
			ret = 1;
			break;
		}

		ret = s_core_nv.read(s_core_nv.start_addr + offset,pBuffer,nBufferSize);
		if(0 != ret)
		{
			TUYA_LOG_E(MOD_NV,"read flash failed");
			error = TUYA_COM_NV_FLASH_OTP_FAILED;
			ret = 1;
			break;
		}
	}while(0);

	if(NULL != pError)
	{
		*pError = error;
	}
	
    return ret;
}


unsigned char tuya_com_nv_write(unsigned short nLID, unsigned short nRecordId, const void *pBuffer, unsigned short nBufferSize, int *pError)
{
    unsigned char ret = 0;
	tuya_com_nv_item_t *pItem= NULL;
	unsigned int offset = 0;
	int error = 0;

	pItem = tuya_core_nv_get_item_info_with_lid(nLID);

	do
	{
		if(NULL == pItem)
		{
			TUYA_LOG_W(MOD_NV,"nv lid not exist");
			error = TUYA_COM_NV_LID_NOT_EXIST;
			ret = 1;
			break;
		}

		if(nBufferSize > pItem->size)
		{
			TUYA_LOG_W(MOD_NV,"Invalid nv nv size");
			error = TUYA_COM_NV_ITEM_SIZE_INVALID;
			ret = 1;
			break;
		}

		offset = tuya_core_nv_item_offet_get(nLID,nRecordId);
		if(TUYA_INVALID_NV_OFFSET == offset)
		{
			TUYA_LOG_W(MOD_NV,"nv offset parse failed");
			error = TUYA_COM_NV_PARSE_OFFSET_FAILED;
			ret = 1;
			break;
		}

		ret = s_core_nv.write(s_core_nv.start_addr + offset,pBuffer,nBufferSize);
		if(0 != ret)
		{
			TUYA_LOG_E(MOD_NV,"write flash failed");
			error = TUYA_COM_NV_FLASH_OTP_FAILED;
			ret = 1;
			break;
		}
	}while(0);

	if(NULL != pError)
	{
		*pError = error;
	}
	
    return ret;
}


unsigned char tuya_com_nv_init(unsigned int start_addr,unsigned int max_size,nv_flash_read read,nv_flash_write write, tuya_com_nv_item_t nv_items[], unsigned short nv_items_size)
{
	int index = 0;

	memset(&s_core_nv, 0, sizeof(s_core_nv));

	s_core_nv.start_addr = start_addr;
	s_core_nv.max_size = max_size;
	s_core_nv.read = read;
	s_core_nv.write = write;
	s_core_nv.nv_items = nv_items;
	s_core_nv.nv_items_size = nv_items_size;

	for(index = 0;index < s_core_nv.nv_items_size;index++)
	{
		TUYA_LOG_I(MOD_NV,"NV itme name==%s",s_core_nv.nv_items[index].item_name);
	}

	TUYA_LOG_I(MOD_NV,"NV ram init Successed");

	return 0;
}



