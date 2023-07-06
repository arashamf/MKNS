#ifndef __TYPEDEF_H
#define __TYPEDEF_H

// Includes ------------------------------------------------------------------------//
#include "MDR32F9Qx_config.h"

// Exported types -----------------------------------------------------------------//
#pragma anon_unions(1)
#pragma pack(1)

typedef enum {UART_RX_MODE = 0, UART_TX_MODE = 1} UartMode; //����� �����

typedef union _MY_FLAGS
{
	unsigned int Value;
	struct //������� ����
	{
		unsigned CAN_Fail				: 1;	//��� unsigned, ����� ���� 1 ���, ������ CAN	( ��� ������ ����������� ��������� C2 )
		unsigned UART_state			: 1; //��� unsigned, ����� ���� 1 ���
	};
}TMyFlags;

//------------------------------------------------------------------
typedef struct 
{
	//uint16_t rst_delay; //�������� ��� ������������ ��������
//	uint16_t cfg_msg_delay; //�������� ��� �������� ����. ��������� �������� 
//	uint16_t parse_delay; //������������ ��������� �������� �������� ��������� �� ��������
	enum 
	{
		__SYNC_RST = 1, //������ ������������ ������
		__SYNC_LOAD_CFG //������ �������� ���������������� ��������� ������
	}	cfg_state;
} MNP_M7_CFG_t;

//------------------------------------------------------------------
typedef struct 
{
	uint32_t 	Time2k; //���������� ������ � 01.01.2000
	float Max_gDOP; //����������� ���������� gDOP
	int8_t		TAI_UTC_offset; // //������� ����� ������� �������� � �������� UTC
	uint8_t 	ValidTHRESHOLD; 	// "��������� �������" ������������� �������������
	struct 
	{
		uint8_t 						: 5;
		uint8_t		LeapS_59 	: 1; //���� "����������" ������� (��������� ������ ����� �������� 59 ������)
		uint8_t		LeapS_61 	: 1; //���� "����������" ������� (��������� ������ ����� �������� 61 �������)
		uint8_t		Valid 		: 1; //���� ������������� ������	
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
	uint8_t ID; 							//��� ������ ��� CAN-���������
	uint8_t Addr; 						//����� � �����-�����
	uint8_t (*GetAddr)(void); //��������� �� �-� ��������� ������ �����
	uint32_t (*MsgA1Send)(void);
} CAN_CONTEXT_t;

//------------------------------------------------------------------
typedef struct 
{
	TM_CONTEXT_t 		tmContext; //��������� � ���������� ����������
	FAIL_CONTEXT_t 	fContext; //������� ���� �� ��������� gps-��������
	CAN_CONTEXT_t 	canContext; //��������� � ������� ��� CAN ���������
} MKS2_t;

#pragma pack()
#pragma anon_unions()

//Private defines ------------------------------------------------------------------//

#define BUFFER_SIZE 					512					//������ ������� ������ � GPS-����������

#define DEFAULT_MAX_gDOP		((float)4.0) //����������� ���������� gDOP �� ���������
#define DEFAULT_MASK_ValidTHRESHOLD		((uint16_t)0x0004) //���������� ���������� ����������� ������� �� ��������

#define FAIL_MASK ((uint16_t)0xE000)

#define GPS_RST_DELAY						250 	//�������� ��� ������������ ��������
#define GPS_CFG_MSG_DELAY				75  	//�������� ��� �������� ����. ��������� �������� 
#define GPS_PARSE_DELAY					100	 //������������ ��������� �������� �������� ��������� �� ��������

#define MNP_SYNC_CHAR						0x81FF //����������� mnp-��������
//Constants ----------------------------------------------------------------------//

#endif
