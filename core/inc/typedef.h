#ifndef __TYPEDEF_H
#define __TYPEDEF_H

// Includes ------------------------------------------------------------------------//
#include "MDR32F9Qx_config.h"

// Exported types -----------------------------------------------------------------//
#pragma anon_unions(1)
#pragma pack(1)

typedef enum {UART_RX_MODE = 0, UART_TX_MODE = 1} UartMode; //режим ”ј–“а

typedef union _MY_FLAGS
{
	unsigned int Value;
	struct //Ѕитовые пол€
	{
		unsigned CAN_Fail				: 1;	//тип unsigned, длина пол€ 1 бит, статус CAN	( нет приема собственных сообщений C2 )
		unsigned UART_state			: 1; //тип unsigned, длина пол€ 1 бит
	};
}TMyFlags;

//------------------------------------------------------------------
typedef struct 
{
	//uint16_t rst_delay; //задержка при перезагрузке приЄмника
//	uint16_t cfg_msg_delay; //задержка при отправке конф. сообщени€ приЄмника 
//	uint16_t parse_delay; //длительность интервала задержки парсинга сообщений от приЄмника
	enum 
	{
		__SYNC_RST = 1, //стади€ перезагрузки модул€
		__SYNC_LOAD_CFG //стади€ отправки конфигурационных сообщений модул€
	}	cfg_state;
} MNP_M7_CFG_t;

//------------------------------------------------------------------
typedef struct 
{
	uint32_t 	Time2k; //количество секунд с 01.01.2000
	float Max_gDOP; //максимально допустимый gDOP
	int8_t		TAI_UTC_offset; // //разница между атомным временем и временем UTC
	uint8_t 	ValidTHRESHOLD; 	// "сдвиговый регистр" накапливающий достоверность
	struct 
	{
		uint8_t 						: 5;
		uint8_t		LeapS_59 	: 1; //флаг "високосной" секунды (последн€€ минута суток содержит 59 секунд)
		uint8_t		LeapS_61 	: 1; //флаг "високосной" секунды (последн€€ минута суток содержит 61 секунду)
		uint8_t		Valid 		: 1; //флаг достоверности данных	
	};
} TM_CONTEXT_t;

//------------------------------------------------------------------
typedef union 
{	
	struct 
	{
		uint8_t											: 4;
		uint8_t GPSAntDisconnect 		: 1;
		uint8_t GPSAntShortCircuit 	: 1;
		uint8_t GPS 								: 1;
		uint8_t CAN 								: 1;
	};
	uint8_t Fail;
} FAIL_CONTEXT_t;

//------------------------------------------------------------------
typedef struct 
{
	uint8_t ID; 							//тип модул€ дл€ CAN-заголовка
	uint8_t Addr; 						//адрес в кросс-плате
	uint8_t (*GetAddr)(void); //указатель на ф-ю получени€ адреса платы
	uint32_t (*MsgA1Send)(void);
} CAN_CONTEXT_t;

//------------------------------------------------------------------
typedef struct 
{
	TM_CONTEXT_t 		tmContext; //структура с временными смещени€ми
	FAIL_CONTEXT_t 	fContext; //битовое поле со статусами gps-приЄмника
	CAN_CONTEXT_t 	canContext; //структура с данными дл€ CAN заголовка
} MKS2_t;

#pragma pack()
#pragma anon_unions()

//Private defines ------------------------------------------------------------------//

#define BUFFER_SIZE 					512					//размер буффера обмена с GPS-приемником

#define DEFAULT_MAX_gDOP		((float)4.0) //максимально допустимый gDOP по умолчанию
#define DEFAULT_MASK_ValidTHRESHOLD		((uint16_t)0x0004) //количество полученных достоверных передач от приЄмника

#define FAIL_MASK ((uint16_t)0xE000)

#define GPS_RST_DELAY						250 	//задержка при перезагрузке приЄмника
#define GPS_CFG_MSG_DELAY				75  	//задержка при отправке конф. сообщени€ приЄмника 
#define GPS_PARSE_DELAY					100	 //длительность интервала задержки парсинга сообщений от приЄмника

#define MNP_SYNC_CHAR						0x81FF //синхрослово mnp-сообщени§
//Constants ----------------------------------------------------------------------//

#endif
