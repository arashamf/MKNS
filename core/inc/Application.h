#ifndef _APP_H_
#define _APP_H_

#include "MDR32F9Qx_config.h"

#define UPS_OK 			0
#define UPS_NO_LINK 	1
#define UPS_BAT_MODE 	2
#define UPS_LOW_BAT 	3
#define UPS_FAULT 		8
#define UPS_BAT_FAULT 	9

#pragma anon_unions

typedef union _MY_FLAGS
{
	unsigned int Value;

	struct
	{
		unsigned CAN_Fail				: 1;	// отказ CAN	( нет приема собственных сообщений C2 )
		unsigned UPS_state				: 4;
	};

}TMyFlags;

extern TMyFlags g_MyFlags;


#endif
