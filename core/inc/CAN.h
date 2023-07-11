#ifndef _MY_CAN_H_
#define _MY_CAN_H_

#include "main.h"

//- Consts ---------------------------------------
#define DATA_TYPE_CONFIG		0x00	
#define DATA_TYPE_SERVICE		0x07
#define CAN_MSG_TYPE_A1_ID	0x01
#define CAN_MSG_TYPE_B_ID		0x08
#define CAN_MSG_TYPE_C_ID		0x10
#define CAN_MSG_TYPE_D_ID		0x20

#define CAN_MSG_TYPE_C_TX_BUFFER_NUM 0		// Буфер для передачи сообщений C2
#define CAN_MSG_TYPE_C_RX_BUFFER_NUM 1		// Буфер для приема сообщений С (в том числе собственных)
#define CAN_MSG_TYPE_C_RX_BUFFER_NUM2 2		// Дополнительный буфер для приема сообщений С (в том числе собственных)

//- Macro ----------------------------------------

#define MAKE_FRAME_ID( msg_type_id, board_addr) (((((uint32_t)msg_type_id) << 5) | ((board_addr))) << 18)
#define MAKE_MSG_DATA0(__module_id, __data_type) ( ( __module_id << 3 ) | __data_type )
#define GET_MODULE_ADDR( frame_id) (((frame_id) >> 18) & 0x1F)
#define GET_MSG_TYPE( frame_id) (((frame_id) >> 23) & 0x3F)

//- Functions ------------------------------------
void CAN1_Init(void *arg);
void CAN1_RX_Process(void);
void CAN1_TX_Process(void);

#endif
