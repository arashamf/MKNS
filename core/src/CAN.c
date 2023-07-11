// Includes ------------------------------------------------------------------
#include "main.h"
#include "can.h"
#include "typedef.h"
#include "protocol.h"
#include "HW_Profile.h"
#include "timers.h"
#include "MDR32F9Qx_can_helper.h"

//Macro -----------------------------------------------------------------------------
#define CAN_BRP_VALUE ( (CPU_CLOCK_VALUE / 1000000UL / 2UL) - 1 )

// ���������� �-�� ------------------------------------------------------------------
static void CAN1_C2_Send(void);
static void CAN1_A1_Send(void);
static void vTimerSelfC2RqstCallback(xTimerHandle xTimer);
static void vTimerRCVSelfC2RqstTimeoutCallback(xTimerHandle xTimer);

// ����������------------------------------------------------------------------
static TM_CONTEXT_t 	*tmContext;				// ��
static FAIL_CONTEXT_t *fContext;				// ������
static CAN_CONTEXT_t 	*canContext;			// CAN

static xTimerHandle xTimerSelfC2Rqst;
static xTimerHandle xTimerRCVSelfC2RqstTimeout;

static CAN_MyRxMsgTypeDef RxMsg; //��������� ��������� CAN-���������

static MESSAGE_C2_t MESSAGE_C2;
static MESSAGE_A1_t MESSAGE_A1;
static MESSAGE_B_CONFIG_t MESSAGE_B_CONFIG;
static MESSAGE_B_SERVICE_t MESSAGE_B_SERVICE;

//---------------------------------------������������� CAN---------------------------------------//
void CAN1_Init(void *arg)
{
	PORT_InitTypeDef sPort;
  CAN_InitTypeDef  sCAN;
	
	tmContext = &(((MKS2_t *)arg)->tmContext);
	fContext = &(((MKS2_t *)arg)->fContext);
	canContext = &(((MKS2_t *)arg)->canContext);
	
	canContext->MsgA1Send = &CAN1_A1_Send;

	//������������� ����� ��� CAN-������
	RST_CLK_PCLKcmd(RST_CLK_PCLK_PORTA, ENABLE);

	PORT_StructInit(&sPort);

	sPort.PORT_FUNC  = PORT_FUNC_ALTER; //GPIO PA6, PA7
	sPort.PORT_MODE  = PORT_MODE_DIGITAL;
	sPort.PORT_SPEED = PORT_SPEED_SLOW;

	sPort.PORT_Pin   = PORT_Pin_7; //CAN_RX1
	sPort.PORT_OE    = PORT_OE_IN;
	PORT_Init(MDR_PORTA, &sPort);

	sPort.PORT_Pin   = PORT_Pin_6; //CAN_TX1
	sPort.PORT_OE    = PORT_OE_OUT;
	PORT_Init(MDR_PORTA, &sPort);	
	
	//������������� CAN1
	RST_CLK_PCLKcmd( RST_CLK_PCLK_CAN1 , ENABLE );
	CAN_BRGInit( MDR_CAN1, CAN_HCLKdiv1);
	CAN_DeInit( MDR_CAN1 );
	CAN_StructInit (&sCAN);

	sCAN.CAN_ROP  = ENABLE; //�������� ����������� ������
	
	canContext->Addr = canContext->GetAddr(); //��������� ������ � �����-�����
	
	if ( canContext->Addr == -1 ) // ���� ������ ������� ��� ������
	{
		canContext->Addr = 0;
		sCAN.CAN_SAP  = ENABLE; //�������� ACK �� ����������� ������	
	} 
	else 
		{sCAN.CAN_SAP  = DISABLE;} // �� �������� ACK �� ����������� ������

	//sCAN.CAN_SAP  = ENABLE; //�������� ACK �� ����������� ������	
	
	sCAN.CAN_STM  = DISABLE; //����� ����������������
	sCAN.CAN_ROM  = DISABLE; //����� "������ ������"

	sCAN.CAN_PSEG = CAN_PSEG_Mul_2TQ;
	sCAN.CAN_SEG1 = CAN_SEG1_Mul_7TQ;
	sCAN.CAN_SEG2 = CAN_SEG2_Mul_6TQ;
	sCAN.CAN_SJW  = CAN_SJW_Mul_2TQ;
	sCAN.CAN_SB   = CAN_SB_3_SAMPLE;
	sCAN.CAN_OVER_ERROR_MAX = 0xFF;
	
	sCAN.CAN_BRP = CAN_BRP_VALUE;				

	CAN_Init(MDR_CAN1, &sCAN);

	//	����� �� ����� ����������� ��������� ��� ������� ���������
	// �������� �� CAN_BUFFER_0	
	CAN_Buffer_RX_Init(MDR_CAN1, 0, DISABLE, (0x7FF << 18), MAKE_FRAME_ID(MSG_TYPE_C, (uint8_t)canContext->Addr)); // ������ �� ��������� C1, C2	
	// �������� �� CAN_BUFFER_1
	CAN_Buffer_RX_Init(MDR_CAN1, 1, DISABLE, (0x7FF << 18), MAKE_FRAME_ID(MSG_TYPE_C, (uint8_t)canContext->Addr)); // ������ �� ��������� C1, C2
	
	//	����� �� �������� ��
	CAN_Buffer_TX_Init(MDR_CAN1, 2); // ����������� �� CAN_BUFFER_2	
	
	//	����� �� �������� ��������� �2 ��������� ������
	CAN_Buffer_TX_Init(MDR_CAN1, 3); 	// ����������� �� CAN_BUFFER_3
	
	//	����� �� ����� ��������� ���������,  �������� �� CAN_BUFFER_4	
	CAN_Buffer_RX_Init(MDR_CAN1, 4, DISABLE, (0x7FF << 18), MAKE_FRAME_ID(MSG_TYPE_B, (uint8_t)canContext->Addr)); // ������ �� ��������� B	

	CAN_Cmd(MDR_CAN1, ENABLE);
	
	xTimerSelfC2Rqst = xTimer_Create(5000, ENABLE, &vTimerSelfC2RqstCallback, ENABLE); //�������� �������
}

//---------------------------------------------------------------------------------------------------//
void CAN1_RX_Process(void)
{	
	if ( CAN_GetTEC(MDR_CAN1) > 127 )  //���� ���������� ������ ������ 127
		{CAN_Cmd(MDR_CAN1, DISABLE);} //���������� CAN1
		
	//	����� ����������� ��������� ��� ������� ���������
	if (CAN_GetBufferStatus(MDR_CAN1, 0) & CAN_STATUS_RX_FULL) //�������� ������� ������ CAN_BUFFER_0 �� ������� �������������� ����� "����� �����"
	{
		CAN_MyGetRawReceivedData(MDR_CAN1, 0, &RxMsg); //��������� ������ �� ������	
		if (RxMsg.Rx_Header.DLC == 0x08 && RxMsg.Rx_Header.RTR == 0 && 
		(uint8_t)RxMsg.Data[0] == MAKE_MSG_DATA0(canContext->ID, 0)) //���� �������� ����������� ��������� C2
		{
			#ifdef __USE_DBG
				printf ("get_C2\r\n");
			#endif
			xTimer_Delete(xTimerRCVSelfC2RqstTimeout); //�������� ������� �������� ��������� ��������� ��N
			fContext->CAN = 0; //������ CAN - ��������
		}	
		// ������ ��������� ������ - ��������� C1
		if (RxMsg.Rx_Header.DLC == 0 && RxMsg.Rx_Header.RTR == 1) //���� �������� ��������� C1
		{
			CAN1_C2_Send(); //�������� ��������� �2
			fContext->CAN = 0; //������ CAN - ��������
			xTimer_Reload(xTimerSelfC2Rqst); //������������ ������� ��������� ������� ���������
			#ifdef __USE_DBG
				printf ("get_C1\r\n");
			#endif
		}		
		CAN_ClearFlag(MDR_CAN1, 0, CAN_STATUS_RX_FULL); //������� ����� "����� �����"
	}

	if ( CAN_GetBufferStatus(MDR_CAN1, 1) & CAN_STATUS_RX_FULL ) 	// �������� ������� ������ CAN_BUFFER_1 
	{
		CAN_MyGetRawReceivedData(MDR_CAN1, 1, &RxMsg); //�������� ������ �� ������			
		if (RxMsg.Rx_Header.DLC == 0x08 && RxMsg.Rx_Header.RTR == 0 && 
				(uint8_t)RxMsg.Data[0] == MAKE_MSG_DATA0(canContext->ID, 0)) // ����������� ��������� C2
		{
			#ifdef __USE_DBG
				printf ("get_C2\r\n");
			#endif
			xTimer_Delete(xTimerRCVSelfC2RqstTimeout); //����� �������
			fContext->CAN = 0; 												//������ CAN - ��������
		}
		
		if ( RxMsg.Rx_Header.DLC == 0 && RxMsg.Rx_Header.RTR == 1 )	// ������ ��������� ������ - ��������� C1
		{
			CAN1_C2_Send(); //�������� ��������� �2
			fContext->CAN = 0; //������ CAN - ��������
			xTimer_Reload(xTimerSelfC2Rqst); //������������ ������� ��������� ������� ���������
			#ifdef __USE_DBG
				printf ("get_C1\r\n");
			#endif
		}		
		CAN_ClearFlag(MDR_CAN1, 1, CAN_STATUS_RX_FULL); //������� ����� "����� �����"
	}
	
	//����� ��������� ���������
	if ( CAN_GetBufferStatus(MDR_CAN1, 4) & CAN_STATUS_RX_FULL ) // �������� ������� ������ CAN_BUFFER_4
	{
		CAN_MyGetRawReceivedData(MDR_CAN1, 4, &RxMsg); //��������� ������ �� ������
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
		CAN_ClearFlag(MDR_CAN1, 4, CAN_STATUS_RX_FULL); //������� ����� "����� �����"
	}		
}

//---------------------------------------------------------------------------------------------------//
static void vTimerSelfC2RqstCallback(xTimerHandle xTimer)
{
	xTimerRCVSelfC2RqstTimeout = xTimer_Create(1100, DISABLE, &vTimerRCVSelfC2RqstTimeoutCallback, ENABLE);
	
	if ( CAN_GetCmdStatus(MDR_CAN1) != ENABLE ) 
		{CAN_Cmd(MDR_CAN1, ENABLE);}	
	
	CAN1_C2_Send();
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
static void CAN1_A1_Send(void)
{
	CAN_TxMsgTypeDef TxMsg;
	TxMsg.IDE     = CAN_ID_STD;
  TxMsg.DLC     = 0x08;
  TxMsg.PRIOR_0 = DISABLE;
  TxMsg.ID      = MAKE_FRAME_ID(MSG_TYPE_A1, (uint8_t)canContext->Addr);
 
	MESSAGE_A1.module_type = canContext->ID;
	MESSAGE_A1.data_type = 1;
	
	MESSAGE_A1.Type3.time2k = tmContext->Time2k;
	
	MESSAGE_A1.Type3.ls_tai = tmContext->TAI_UTC_offset;
	
	MESSAGE_A1.Type3.ls_59 = tmContext->LeapS_59;
	MESSAGE_A1.Type3.ls_61 = tmContext->LeapS_61;
	MESSAGE_A1.Type3.moscow_tz = 0;
	MESSAGE_A1.Type3.local_tz = -128;
	
	memcpy(&TxMsg.Data, &MESSAGE_A1.RAW[0], 8);

	CAN_ITClearErrorPendingBit(MDR_CAN1, CAN_STATUS_IDLOWER | CAN_STATUS_CRC_ERR | 
																			CAN_STATUS_FRAME_ERR | CAN_STATUS_ACK_ERR );
	
	CAN_Transmit(MDR_CAN1, 2, &TxMsg);
}


//---------------------------------------�������� ��������� C2---------------------------------------//
static void CAN1_C2_Send(void)
{
	CAN_TxMsgTypeDef TxMsg;
	TxMsg.IDE     = CAN_ID_STD; //����������� ���������
  TxMsg.DLC     = 0x08;  //8 ����
  TxMsg.PRIOR_0 = DISABLE;
  TxMsg.ID      = MAKE_FRAME_ID(MSG_TYPE_C, (uint8_t)canContext->Addr);
 
	MESSAGE_C2.module_id = canContext->ID;
	MESSAGE_C2.data_type = 0;
	
	if ( (fContext->Fail & FAIL_MASK) != 0 ) //���� ������������� ������, ��� ������� ���� �� ������ ����� ������
		{MESSAGE_C2.fail = 1;} 
	else 
		{MESSAGE_C2.fail = 0;}
	
	MESSAGE_C2.fail_gps = fContext->GPS;
	MESSAGE_C2.fail_gps_ant = fContext->GPSAntShortCircuit;
	MESSAGE_C2.gps_ant_disc = fContext->GPSAntDisconnect; 
	
	memcpy(&TxMsg.Data, &MESSAGE_C2, 8);

	CAN_ITClearErrorPendingBit(MDR_CAN1, CAN_STATUS_IDLOWER | CAN_STATUS_CRC_ERR | 
																		CAN_STATUS_FRAME_ERR | CAN_STATUS_ACK_ERR );
	
	CAN_Transmit(MDR_CAN1, 3, &TxMsg);
}



