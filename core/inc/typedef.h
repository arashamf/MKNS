#ifndef __TYPEDEF_H
#define __TYPEDEF_H

// Includes ------------------------------------------------------------------------//
#include "MDR32F9Qx_config.h"

// Exported types -----------------------------------------------------------------//
#pragma anon_unions(1)
#pragma pack(1)

//------------------------------------------------------------------
typedef enum {UART_RX_MODE = 0, UART_TX_MODE = 1} UartMode; //режим УАРТа

//------------------------------------------------------------------
typedef struct 
{
	//uint16_t rst_delay; //задержка при перезагрузке приёмника
//	uint16_t cfg_msg_delay; //задержка при отправке конф. сообщения приёмника 
	enum 
	{
		__SYNC_EMPTY = 0,
		__SYNC_SOFTRST,
		__SYNC_HARDRST, //стадия перезагрузки модуля
		__SYNC_LOAD_CFG //стадия отправки конфигурационных сообщений модуля
	}	cfg_state;
} MNP_M7_CFG_t;

//------------------------------------------------------------------
typedef struct 
{
	uint32_t 	Time2k; //количество секунд с 01.01.2000
	float 		Max_gDOP; //максимально допустимый gDOP
	int8_t		TAI_UTC_offset; //разница между атомным временем и временем UTC
	uint8_t 	ValidTHRESHOLD; 	// "сдвиговый регистр" накапливающий достоверность
	uint8_t 	sum_bad_msg; //количество подряд принятых "плохих" сообщений от приёмника (GDOP > 20)
	
	struct 
	{
		uint8_t 										: 3;
		uint8_t		time_data_ready 	: 1; //флаг получения данных времени от приёмника
		uint8_t		put_PPS 					: 1; //флаг отправки метки времени
		uint8_t		LeapS_59 					: 1; //флаг "високосной" секунды (последняя минута суток содержит 59 секунд)
		uint8_t		LeapS_61 					: 1; //флаг "високосной" секунды (последняя минута суток содержит 61 секунду)
		uint8_t		Valid 						: 1; //флаг достоверности данных	
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
	uint8_t ID; 							//тип модуля (МКНС) для CAN-заголовка
	int8_t Addr; 						//адрес в кросс-плате
	int8_t (*GetAddr)(void); //указатель на ф-ю получения адреса в кросс-плате
	void (*MsgA1Send)(void);
} CAN_CONTEXT_t;

//------------------------------------------------------------------
typedef struct 
{
	TM_CONTEXT_t 		tmContext; //структура с данными времени
	FAIL_CONTEXT_t 	fContext; //битовое поле со статусами gps-приёмника
	CAN_CONTEXT_t 	canContext; //структура с данными для CAN заголовка
} MKS2_t;

//структуры CAN-сообщения С2----------------------------------------
typedef struct
{
	uint8_t data_type			: 3;
	uint8_t module_id			: 5;
	uint8_t 							: 5;
	uint8_t fail_gps_ant 	: 1; 		// 1 - КЗ GPS антены  
	uint8_t fail_gps 			: 1; 		// 1 - отказ GPS приемника
	uint8_t fail 					: 1; 		// 1 - интегральный отказ, при наличие хотя бы одного отказа
	uint8_t 							: 7;
	uint8_t gps_ant_disc	: 1;		// 1 - GPS антена неподключенна
	uint8_t 							: 8;
	uint8_t 							: 8;
	uint8_t 							: 8;
	uint8_t 							: 8;
	uint8_t 							: 8;
} MESSAGE_C2_t;

//------------------------------------------------------------------
typedef union
{
	struct
	{
		uint8_t data_type		: 3;
		uint8_t module_type	: 5;
		uint16_t gDOP;
		uint8_t 						: 8;
		uint8_t 						: 8;
		uint8_t 						: 8;
		uint8_t 						: 8;
		uint8_t 						: 8;
	};
	uint8_t RAW[8];
}MESSAGE_B_CONFIG_t;

//------------------------------------------------------------------
typedef union
{
	struct
	{
		uint8_t data_type		: 3;
		uint8_t module_type	: 5;
		uint8_t 			 			: 6;
		uint8_t disable			: 1;
		uint8_t reset				: 1;
		uint8_t 						: 8;
		uint8_t 						: 8;
		uint8_t 						: 8;
		uint8_t 						: 8;
		uint8_t 						: 8;
		uint8_t 					: 8;
	};
	uint8_t RAW[8];
}MESSAGE_B_SERVICE_t;

//------------------------------------------------------------------
typedef union
{
	struct
	{
		uint8_t data_type		: 3;
		uint8_t module_type	: 5;
		union
		{
			struct
			{
				uint8_t year		: 8;
				uint8_t month		: 8;
				uint8_t day			: 8;
				uint8_t hour		: 8;
				uint8_t min			: 8;
				uint8_t sec			: 8;
				uint8_t local_tz	: 5;
				uint8_t moskow_tz	: 3;
			}Type2;
			
			struct
			{
				uint32_t time2k;
				struct
				{
					int8_t ls_tai					: 8; 	// TAI leap seconds (TAI-UTC)
					uint8_t moscow_tz			: 3;	// в часах
					uint8_t 							: 3;
					uint8_t ls_59					: 1;
					uint8_t ls_61					: 1;				
					int8_t local_tz				: 8;	// в 15 мин интервалах
				};
			}Type3;			
		};
	};

	uint8_t RAW[8];

}MESSAGE_A1_t;

#pragma pack()
#pragma anon_unions()

//Private defines ------------------------------------------------------------------//
/* DEBUG information */
#ifdef __USE_DBG
#define DBG(...)  printf(__VA_ARGS__)
#else
#define DBG(...)
#endif

#define BUFFER_SIZE 					512					//размер буффера обмена с GPS-приемником

#define DEFAULT_MAX_gDOP		((float)4.0) 	//максимально допустимый gDOP по умолчанию
#define DEFAULT_MIN_gDOP		((float)0.1)
#define DEFAULT_MASK_ValidTHRESHOLD		((uint16_t)0x0004) //количество полученных подряд достоверных сообщений от приёмника, необходимых для отправки CAN-сообщения типа А1

#define FAIL_MASK ((uint16_t)0xF0) //маска флага интегрального отказа

#define GPS_RST_DELAY						5 	//задержка при аппаратной перезагрузке приёмника
#define GPS_CFG_MSG_DELAY				100  	//задержка при отправке конф. сообщения приёмника 
#define GPS_PARSE_DELAY					100	 //длительность интервала задержки парсинга сообщений от приёмника

#define MNP_SYNC_CHAR						0x81FF //синхрослово mnp-сообщени¤
//Constants ----------------------------------------------------------------------//

#endif
