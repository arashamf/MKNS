// Includes ------------------------------------------------------------------
#include "main.h"
#include "can.h"
#include "pins.h"
#include "typedef.h"
#include "protocol.h"
#include "HW_Profile.h"
#include "timers.h"

//Macro -----------------------------------------------------------------------------
#define CAN_BRP_VALUE ( (CPU_CLOCK_VALUE / 1000000UL / 2UL) - 1 ) // � Tq (�s) = ((BRP+1))/CLK (MHz) 

// ���������� �-�� ------------------------------------------------------------------
static void CAN_C2_Send(void);
static void CAN_A1_Send(void);

static void vTimerSelfC2RqstCallback(xTimerHandle xTimer);
static void vTimerRCVSelfC2RqstTimeoutCallback(xTimerHandle xTimer);

// ����������------------------------------------------------------------------

static xTimerHandle xTimerSelfC2Rqst;
static xTimerHandle xTimerRCVSelfC2RqstTimeout;

static CAN_MyRxMsgTypeDef RxMsg; //��������� ��������� CAN-���������

static MESSAGE_C2_t MESSAGE_C2;
static MESSAGE_A1_t MESSAGE_A1;
static MESSAGE_B_CONFIG_t MESSAGE_B_CONFIG;
static MESSAGE_B_SERVICE_t MESSAGE_B_SERVICE;

static TM_CONTEXT_t 	*tmContext;			// ��
static FAIL_CONTEXT_t *fContext;			// ������
static CAN_CONTEXT_t 	*canContext;		// CAN

//---------------------------------------������������� CAN---------------------------------------//
void Init_CAN (void *arg)
{
	PORT_InitTypeDef sPort;
  CAN_InitTypeDef  sCAN;
	
	tmContext = &(((MKS2_t *)arg)->tmContext);
	fContext = &(((MKS2_t *)arg)->fContext);
	canContext = &(((MKS2_t *)arg)->canContext);
	
	canContext->MsgA1Send = &CAN_A1_Send;    //��������� �� �-� �������� ��������� ���� �1
	canContext->ID = MODULE_TYPE_MKNS, 					//������������� ���� ������
	canContext->GetAddr = &Get_Module_Address, //������������� ��������� �� ������� ��������� ������ � �����-�����
	
	//������������� ����� ��� CAN-������
	RST_CLK_PCLKcmd(CAN_PIN_CLOCK, ENABLE); //��������� ������������ GPIO

	PORT_StructInit(&sPort);

	sPort.PORT_FUNC  = PORT_FUNC_ALTER; //GPIO PA6, PA7
	sPort.PORT_MODE  = PORT_MODE_DIGITAL;
	sPort.PORT_SPEED = PORT_SPEED_SLOW;

	sPort.PORT_Pin   = CAN_RX_PIN; //CAN_RX1
	sPort.PORT_OE    = PORT_OE_IN;
	PORT_Init(CAN_RX_PORT	, &sPort);

	sPort.PORT_Pin   = CAN_TX_PIN; //CAN_TX1
	sPort.PORT_OE    = PORT_OE_OUT;
	PORT_Init(CAN_TX_PORT	, &sPort);	
	
	//������������� CAN
	RST_CLK_PCLKcmd(CAN_CLOCK, ENABLE );
	CAN_BRGInit( MY_MDR_CAN, CAN_HCLKdiv1); // Set the HCLK division factor = 1 for CAN
	CAN_DeInit( MY_MDR_CAN );		// Reset CAN to POR
	CAN_StructInit (&sCAN);

	sCAN.CAN_ROP  = ENABLE; //�������� ����������� ������
//	sCAN.CAN_SAP  = ENABLE; //�������� ACK �� ����������� ������	
	sCAN.CAN_STM  = DISABLE; //����� ����������������
	sCAN.CAN_ROM  = DISABLE; //����� "������ ������"
	
	canContext->Addr = canContext->GetAddr(); //��������� ������ � �����-�����
	
	if ( MKS2.canContext.Addr == -1 ) // ���� ������ ������� ��� ������
	{
		canContext->Addr = 0;
		sCAN.CAN_SAP  = ENABLE; //�������� ACK �� ����������� ������	
	} 
	else 
		{sCAN.CAN_SAP  = DISABLE;} // �� �������� ACK �� ����������� ������
	
	// Nominal Bit Time (NBT) = 8 ��� (125����/�);
	// ��������� Tq = 0.5 ���;
	// �.�. NBT = Tq * (Sync_Seg + PSEG + SEG1 + SEG2), ��� Sync_Seg = 1 (���������),
	// ����� PSEG + SEG1 + SEG2 = NBT/Tq - 1. �� ���� �������� �������:  (8 * Tq) <= NBT <= (25 * Tq).
	// �������������� ����������:
	// � PSEG + SEG1  >=  SEG2
	// � SEG2  >=  SJW (Sync Jump Width)		
	sCAN.CAN_PSEG = CAN_PSEG_Mul_2TQ;
	sCAN.CAN_SEG1 = CAN_SEG1_Mul_7TQ;
	sCAN.CAN_SEG2 = CAN_SEG2_Mul_6TQ;
	sCAN.CAN_SJW  = CAN_SJW_Mul_2TQ;
	sCAN.CAN_SB   = CAN_SB_3_SAMPLE;
	sCAN.CAN_OVER_ERROR_MAX = 0xFF;
	
	sCAN.CAN_BRP = CAN_BRP_VALUE;				

	CAN_Init(MY_MDR_CAN, &sCAN);

	//	����� �� ����� ����������� ��������� ��� ������� ���������
	// �������� �� CAN_BUFFER_0	
	CAN_Buffer_RX_Init(MY_MDR_CAN, 0, DISABLE, (0x7FF << 18), MAKE_FRAME_ID(MSG_TYPE_C, (uint8_t)canContext->Addr)); // ������ �� ��������� C1, C2	
	//CAN_Buffer_RX_Init(MY_MDR_CAN, 0, DISABLE, (0x7FF << 18), 0xFFFF0000);
	
	// �������� �� CAN_BUFFER_1
	CAN_Buffer_RX_Init(MY_MDR_CAN, 1, DISABLE, (0x7FF << 18), MAKE_FRAME_ID(MSG_TYPE_C, (uint8_t)canContext->Addr)); // ������ �� ��������� C1, C2
	
	//	����� �� �������� ����� �������
	CAN_Buffer_TX_Init(MY_MDR_CAN, 2); // ����������� �� CAN_BUFFER_2	
	
	//	����� �� �������� ��������� �2 ��������� ������
	CAN_Buffer_TX_Init(MY_MDR_CAN, 3); 	// ����������� �� CAN_BUFFER_3
	
	//	����� �� ����� ��������� ���������,  �������� �� CAN_BUFFER_4	
	CAN_Buffer_RX_Init(MY_MDR_CAN, 4, DISABLE, (0x7FF << 18), MAKE_FRAME_ID(MSG_TYPE_B, (uint8_t)canContext->Addr)); // ������ �� ��������� B	
	
	CAN_Cmd(MY_MDR_CAN, ENABLE);
	
	xTimerSelfC2Rqst = xTimer_Create(5000, ENABLE, &vTimerSelfC2RqstCallback, ENABLE); //�������� ������� ���������� ���������� �� CAN
}

//---------------------------------------------------------------------------------------------------//
void CAN_RX_Process(void)
{	
	if ( CAN_GetTEC(MY_MDR_CAN) > 127 )  //���� ���������� ������ ������ 127
		{CAN_Cmd(MY_MDR_CAN, DISABLE);} //���������� CAN
		
	//	����� ����������� ��������� ��� ������� ���������
	if (CAN_GetBufferStatus(MY_MDR_CAN, 0) & CAN_STATUS_RX_FULL) //�������� ������� ������ CAN_BUFFER_0 �� ������� �������������� ����� "����� �����"
	{
		CAN_MyGetRawReceivedData(MY_MDR_CAN, 0, &RxMsg); //��������� ������ �� ������	
		if (RxMsg.Rx_Header.DLC == 0x08 && RxMsg.Rx_Header.RTR == 0 && 
		(uint8_t)RxMsg.Data[0] == MAKE_MSG_DATA0(canContext->ID, 0)) //���� �������� ����������� ��������� C2
		{
			xTimer_Delete(xTimerRCVSelfC2RqstTimeout); //�������� ������� �������� ��������� ��������� ��N
			fContext->CAN = 0; //������ CAN - ��������
		}	
		// ������ ��������� ������ - ��������� C1
		if (RxMsg.Rx_Header.DLC == 0 && RxMsg.Rx_Header.RTR == 1) //���� �������� ��������� C1
		{
			CAN_C2_Send(); //�������� ��������� �2
			fContext->CAN = 0; //������ CAN - ��������
			xTimer_Reload(xTimerSelfC2Rqst); //������������ ������� 
		/*	#ifdef __USE_DBG
				printf ("CAN0-get_C1\r\n");
			#endif*/
		}		
		CAN_ClearFlag(MY_MDR_CAN, 0, CAN_STATUS_RX_FULL); //������� ����� "����� �����"
	}

	if ( CAN_GetBufferStatus(MY_MDR_CAN, 1) & CAN_STATUS_RX_FULL ) 	// �������� ������� ������ CAN_BUFFER_1 
	{
		CAN_MyGetRawReceivedData(MY_MDR_CAN, 1, &RxMsg); //�������� ������ �� ������			
		if (RxMsg.Rx_Header.DLC == 0x08 && RxMsg.Rx_Header.RTR == 0 && 
				(uint8_t)RxMsg.Data[0] == MAKE_MSG_DATA0(canContext->ID, 0)) // ����������� ��������� C2
		{
			xTimer_Delete(xTimerRCVSelfC2RqstTimeout); //����� �������
			fContext->CAN = 0; 												//������ CAN - ��������
		}
		
		if ( RxMsg.Rx_Header.DLC == 0 && RxMsg.Rx_Header.RTR == 1 )	// ������ ��������� ������ - ��������� C1
		{
			CAN_C2_Send(); //�������� ��������� �2
			fContext->CAN = 0; //������ CAN - ��������
			xTimer_Reload(xTimerSelfC2Rqst); //������������ ������� 
			/*#ifdef __USE_DBG
				printf ("CAN-get_C1\r\n");
			#endif*/
		}		
		CAN_ClearFlag(MY_MDR_CAN, 1, CAN_STATUS_RX_FULL); //������� ����� "����� �����"
	}
	
	//����� ��������� ���������
	if ( CAN_GetBufferStatus(MY_MDR_CAN, 4) & CAN_STATUS_RX_FULL ) // �������� ������� ������ CAN_BUFFER_4
	{
		CAN_MyGetRawReceivedData(MY_MDR_CAN, 4, &RxMsg); //��������� ������ �� ������
		if ( (uint8_t)RxMsg.Data[0] == MAKE_MSG_DATA0(canContext->ID, DATA_TYPE_CONFIG) ) //�������� 1 ����� ���������
		{
			memcpy(&MESSAGE_B_CONFIG.RAW[0], &RxMsg.Data[0], 8);		
			tmContext->Max_gDOP = ((float)MESSAGE_B_CONFIG.gDOP) / 100; //��������� ������ �������� gDOP
		}

		if ((uint8_t)RxMsg.Data[0] == MAKE_MSG_DATA0(canContext->ID, DATA_TYPE_SERVICE))//��������� ������������
		{
			memcpy(&MESSAGE_B_SERVICE.RAW[0], &RxMsg.Data[0], 8);
			if (MESSAGE_B_SERVICE.reset) 
				{NVIC_SystemReset();}		
		}	
		CAN_ClearFlag(MY_MDR_CAN, 4, CAN_STATUS_RX_FULL); //������� ����� "����� �����"
	}		
}

//---------------------------------------------------------------------------------------------------//
static void vTimerSelfC2RqstCallback(xTimerHandle xTimer)
{
	xTimerRCVSelfC2RqstTimeout = xTimer_Create(1100, DISABLE, &vTimerRCVSelfC2RqstTimeoutCallback, ENABLE);
	
	if ( CAN_GetCmdStatus(MY_MDR_CAN) != ENABLE ) //���� CAN ��� ��������
		{CAN_Cmd(MY_MDR_CAN, ENABLE);}	//��������� CAN
	
	CAN_C2_Send(); //�������� ��������� �2
}

//---------------------------------������� ��������� ��������� �� CAN---------------------------------//
static void vTimerRCVSelfC2RqstTimeoutCallback(xTimerHandle xTimer)
{
	fContext->CAN = 1; //CAN ����������
	#ifdef __USE_DBG
		printf ("CAN_error\r\n");
	#endif
	xTimer_Delete(xTimer);
}


//---------------------------------------�������� ��������� A1---------------------------------------//
static void CAN_A1_Send(void)
{
	CAN_TxMsgTypeDef TxMsg;
	TxMsg.IDE     = CAN_ID_STD;
  TxMsg.DLC     = 0x08;
  TxMsg.PRIOR_0 = DISABLE;
  TxMsg.ID      = MAKE_FRAME_ID(MSG_TYPE_A1, (uint8_t)MKS2.canContext.Addr);
 
	MESSAGE_A1.module_type = canContext->ID;
	MESSAGE_A1.data_type = 1;
	
	MESSAGE_A1.Type3.time2k = tmContext->Time2k;
	
	MESSAGE_A1.Type3.ls_tai = tmContext->TAI_UTC_offset;
	
	MESSAGE_A1.Type3.ls_59 = tmContext->LeapS_59;
	MESSAGE_A1.Type3.ls_61 = tmContext->LeapS_61;
	MESSAGE_A1.Type3.moscow_tz = 0;
	MESSAGE_A1.Type3.local_tz = -128;
	
	memcpy(&TxMsg.Data, &MESSAGE_A1.RAW[0], 8);

	CAN_ITClearErrorPendingBit(MY_MDR_CAN, CAN_STATUS_IDLOWER | CAN_STATUS_CRC_ERR | 
																			CAN_STATUS_FRAME_ERR | CAN_STATUS_ACK_ERR );
	
	CAN_Transmit(MY_MDR_CAN, 2, &TxMsg);
	#ifdef __USE_DBG
		printf ("put_A1\r\n");
	#endif
}


//---------------------------------------�������� ��������� C2---------------------------------------//
static void CAN_C2_Send(void)
{
	CAN_TxMsgTypeDef TxMsg;
	TxMsg.IDE     = CAN_ID_STD; //����������� ���������
  TxMsg.DLC     = 0x08;  //8 ����
  TxMsg.PRIOR_0 = DISABLE;
  TxMsg.ID      = MAKE_FRAME_ID(MSG_TYPE_C, (uint8_t)MKS2.canContext.Addr);
 
	MESSAGE_C2.module_id = canContext->ID;
	MESSAGE_C2.data_type = 0;
	
	if ((fContext->Fail & FAIL_MASK) != 0 ) //���� ������������� ������, ��� ������� ���� �� ������ ����� ������
	{
		MESSAGE_C2.fail = 1;
	} 
	else 
		{MESSAGE_C2.fail = 0;}
		
	MESSAGE_C2.fail_gps = fContext->GPS; //������ ���������� � GPS-�������
	MESSAGE_C2.fail_gps_ant = fContext->GPSAntShortCircuit;
	MESSAGE_C2.gps_ant_disc = fContext->GPSAntDisconnect; 
	
	memcpy(&TxMsg.Data, &MESSAGE_C2, 8);

	CAN_ITClearErrorPendingBit(MY_MDR_CAN, CAN_STATUS_IDLOWER | CAN_STATUS_CRC_ERR | 
																		CAN_STATUS_FRAME_ERR | CAN_STATUS_ACK_ERR );
	
	CAN_Transmit(MY_MDR_CAN, 3, &TxMsg);
}



