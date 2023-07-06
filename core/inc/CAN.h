#ifndef _MY_CAN_H_
#define _MY_CAN_H_

#pragma push
#pragma pack(1)

#include "main.h"
//- Types ----------------------------------------

	//------------------------------
	//
	typedef union 
	{
		struct
		{
			uint32_t DATAL;
			uint32_t DATAH;
		};

		struct
		{
			unsigned int data_type		: 3;
			unsigned int module_type	: 5;
			unsigned int 				: 4;
			unsigned int state			: 4;
		};

		uint8_t Bytes[ 8 ];

	}TCAN_MSG_TYPE_C_MKIP;

#pragma pop

//- Consts ---------------------------------------

#define CAN_MSG_TYPE_A1_ID	0x01
#define CAN_MSG_TYPE_B_ID	0x08
#define CAN_MSG_TYPE_C_ID	0x10
#define CAN_MSG_TYPE_D_ID	0x20

#define MY_MODULE_TYPE 0x15	// Код типа модуля - МКИП

#define CAN_MSG_TYPE_C_TX_BUFFER_NUM 0		// Буфер для передачи сообщений C2
#define CAN_MSG_TYPE_C_RX_BUFFER_NUM 1		// Буфер для приема сообщений С (в том числе собственных)
#define CAN_MSG_TYPE_C_RX_BUFFER_NUM2 2		// Дополнительный буфер для приема сообщений С (в том числе собственных)

//- Macro ----------------------------------------

#define MAKE_FRAME_ID( msg_type_id, board_addr) (((((uint32_t)msg_type_id) << 5) | ((board_addr))) << 18)
#define GET_MODULE_ADDR( frame_id) (((frame_id) >> 18) & 0x1F)
#define GET_MSG_TYPE( frame_id) (((frame_id) >> 23) & 0x3F)

//- Functions ------------------------------------

void InitCAN( void );
void TaskCAN( void );

#endif
