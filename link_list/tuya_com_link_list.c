#include "tuya_com_link_list.h"
#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define LIST_DEBUG

#ifdef LIST_DEBUG
#include "tuya_log.h"
#define LIST_LOG_I TUYA_LOG_I
#define LIST_LOG_W TUYA_LOG_W
#define LIST_LOG_E TUYA_LOG_E
#else
#define LIST_LOG_I(mod_id,fmt, ...) 
#define LIST_LOG_W(mod_id,fmt, ...) 
#define LIST_LOG_E(mod_id,fmt, ...) 
#endif



//创建一个新队列  
link_queue_t* tuya_com_link_list_create(void)  
{  
    link_queue_t *q = (link_queue_t*)tuya_ble_malloc(sizeof(link_queue_t)); 
	if(NULL == q)
	{
		LIST_LOG_E(MOD_OS,"malloc failed");
		return NULL;
	}
    q->head = NULL;
    q->tail = NULL;
    return q;
}

void tuya_com_link_list_clear(link_queue_t *q)
{
	link_node_t *node = NULL; 
	link_node_t *nodeCur = NULL; 
	if(q->head == NULL)
	{  
		LIST_LOG_W(MOD_OS,"the queue is empty \n");  
	}
	
	node = q->head;
	while(node != NULL)
	{
		nodeCur = node->next;
		tuya_ble_free(node);
		node = nodeCur;
	}
	tuya_ble_free(q);
	q = NULL;
}
//向队列插入一个结点  
link_node_t* tuya_com_link_list_node_insert(link_queue_t *q,int data)
{  
    link_node_t *node;  
    node = (link_node_t*)tuya_ble_malloc(sizeof(link_node_t));  

	if(NULL == q)
	{
		LIST_LOG_E(MOD_OS,"malloc failed");
		return NULL;
	}
	
    node->value.v = data;  
    node ->next = NULL;  

    #if 1
    if(q->tail == NULL)
    {  
        q->head = node;  
        q->tail = node;  
    }  
    else  
    {  
        q->tail->next = node;  
        q->tail = node;  
    }
    #else
    if(q->head == NULL)
    {  
        q->head = node;  
        q->tail = node;  
    }else{
        node->next = q->head;
        q->head = node;
    }
    #endif
    return node;
}
//从队列删除一个结点  
link_queue_t* tuya_com_link_list_first_node_delete(link_queue_t *q)
{  
    link_node_t *node;  
    if(q->head == NULL)
    {  
        LIST_LOG_E(MOD_OS,"the queue is empty \n");  
        return q;  
    }  
      
    node = q->head;
    if(q->head == q->tail)  
    {
        q->head = NULL;
        q->tail = NULL;
        tuya_ble_free(node);
    }else
    {
        q->head = q->head->next;
        tuya_ble_free(node);
    }  
    return q;
}  



link_queue_t* tuya_com_link_list_node_delete(link_queue_t *q, link_node_t *node)
{
	link_node_t *nodeCur; 
    link_node_t *nodeOld = NULL;
    if(q->head == NULL)
    {  
        LIST_LOG_E(MOD_OS,"the queue is empty \n");  
        return q;
    }  
      
    for(nodeCur = q->head; nodeCur != NULL; nodeCur = nodeCur->next)
		{
			if(nodeCur == node)
			{
				if(q->head == q->tail)  
				{
						q->head = NULL;
						q->tail = NULL;
						tuya_ble_free(node);
				}else
				{
						nodeOld->next = nodeCur->next;
						tuya_ble_free(node);
				}
				break;
			}
			nodeOld = nodeCur;
		}
    return q;
}
 

link_node_t* tuya_com_link_list_first_node_get(link_queue_t *q)
{
	return q->head;
}

link_node_t* tuya_com_link_list_next_node_get(link_node_t *node)
{
	return node->next;
}
