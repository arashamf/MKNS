#ifndef __TYPEDEF_H
#define __TYPEDEF_H

// Includes ------------------------------------------------------------------------//
#include "MDR32F9Qx_config.h"

// Exported types -----------------------------------------------------------------//
#pragma anon_unions(1)
#pragma pack(1)

//------------------------------------------------------------------
typedef enum {UART_RX_MODE = 0, UART_TX_MODE = 1} UartMode; //����� �����

//------------------------------------------------------------------
typedef struct 
{
	//uint16_t rst_delay; //�������� ��� ������������ ��������
//	uint16_t cfg_msg_delay; //�������� ��� �������� ����. ��������� �������� 
	enum 
	{
		__SYNC_EMPTY = 0,
		__SYNC_SOFTRST,
		__SYNC_HARDRST, //������ ������������ ������
		__SYNC_LOAD_CFG //������ �������� ���������������� ��������� ������
	}	cfg_state;
} MNP_M7_CFG_t;

//------------------------------------------------------------------
typedef struct 
{
	uint32_t 	Time2k; //���������� ������ � 01.01.2000
	float 		Max_gDOP; //����������� ���������� gDOP
	int8_t		TAI_UTC_offset; //������� ����� ������� �������� � �������� UTC
	uint8_t 	ValidTHRESHOLD; 	// "��������� �������" ������������� �������������
	uint8_t 	sum_bad_msg; //���������� ������ �������� "������" ��������� �� �������� (GDOP > 20)
	
	struct 
	{
		uint8_t 										: 3;
		uint8_t		time_data_ready 	: 1; //���� ��������� ������ ������� �� ��������
		uint8_t		put_PPS 					: 1; //���� �������� ����� �������
		uint8_t		LeapS_59 					: 1; //���� "����������" ������� (��������� ������ ����� �������� 59 ������)
		uint8_t		LeapS_61 					: 1; //���� "����������" ������� (��������� ������ ����� �������� 61 �������)
		uint8_t		Valid 						: 1; //���� ������������� ������	
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
	uint8_t ID; 							//��� ������ (����) ��� CAN-���������
	int8_t Addr; 						//����� � �����-�����
	int8_t (*GetAddr)(void); //��������� �� �-� ��������� ������ � �����-�����
	void (*MsgA1Send)(void);
} CAN_CONTEXT_t;

//------------------------------------------------------------------
typedef struct 
{
	TM_CONTEXT_t 		tmContext; //��������� � ������� �������
	FAIL_CONTEXT_t 	fContext; //������� ���� �� ��������� gps-��������
	CAN_CONTEXT_t 	canContext; //��������� � ������� ��� CAN ���������
} MKS2_t;

//��������� CAN-��������� �2----------------------------------------
typedef struct
{
	uint8_t data_type			: 3;
	uint8_t module_id			: 5;
	uint8_t 							: 5;
	uint8_t fail_gps_ant 	: 1; 		// 1 - �� GPS ������  
	uint8_t fail_gps 			: 1; 		// 1 - ����� GPS ���������
	uint8_t fail 					: 1; 		// 1 - ������������ �����, ��� ������� ���� �� ������ ������
	uint8_t 							: 7;
	uint8_t gps_ant_disc	: 1;		// 1 - GPS ������ �������������
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
					uint8_t moscow_tz			: 3;	// � �����
					uint8_t 							: 3;
					uint8_t ls_59					: 1;
					uint8_t ls_61					: 1;				
					int8_t local_tz				: 8;	// � 15 ��� ����������
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

#define BUFFER_SIZE 					512					//������ ������� ������ � GPS-����������

#define DEFAULT_MAX_gDOP		((float)4.0) 	//����������� ���������� gDOP �� ���������
#define DEFAULT_MIN_gDOP		((float)0.1)
#define DEFAULT_MASK_ValidTHRESHOLD		((uint16_t)0x0004) //���������� ���������� ������ ����������� ��������� �� ��������, ����������� ��� �������� CAN-��������� ���� �1

#define FAIL_MASK ((uint16_t)0x0F) //����� ����� ������������� ������

#define GPS_RST_DELAY						5 	//�������� ��� ���������� ������������ ��������
#define GPS_CFG_MSG_DELAY				100  	//�������� ��� �������� ����. ��������� �������� 
#define GPS_PARSE_DELAY					100	 //������������ ��������� �������� �������� ��������� �� ��������

#define MNP_SYNC_CHAR						0x81FF //����������� mnp-��������
//Constants ----------------------------------------------------------------------//

#endif
