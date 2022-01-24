
#ifndef __TUYA_LINK_LIST_H__
#define __TUYA_LINK_LIST_H__
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
typedef struct dataNode  
{  
	union{
        uint32_t v; 
		void * p;
	}value;
    struct dataNode *next;
}link_node_t; 

typedef struct
{  
    link_node_t *head;  
    link_node_t *tail;  
}link_queue_t;  

link_queue_t * tuya_com_link_list_create(void);
link_node_t* tuya_com_link_list_node_insert(link_queue_t *q,int data);
link_queue_t* tuya_com_link_list_first_node_delete(link_queue_t *q);
link_queue_t* tuya_com_link_list_node_delete(link_queue_t *q, link_node_t *node);
void tuya_com_link_list_clear(link_queue_t *q);
link_node_t* tuya_com_link_list_first_node_get(link_queue_t *q);
link_node_t* tuya_com_link_list_next_node_get(link_node_t *node);

#endif

