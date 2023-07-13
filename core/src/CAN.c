// Includes ------------------------------------------------------------------
#include "main.h"
#include "can.h"
#include "pins.h"
#include "typedef.h"
#include "protocol.h"
#include "HW_Profile.h"
#include "timers.h"

//Macro -----------------------------------------------------------------------------
#define CAN_BRP_VALUE ( (CPU_CLOCK_VALUE / 1000000UL / 2UL) - 1 ) // • Tq (µs) = ((BRP+1))/CLK (MHz) 

// объявление ф-ий ------------------------------------------------------------------
static void CAN_C2_Send(void);
static void CAN_A1_Send(void);

static void vTimerSelfC2RqstCallback(xTimerHandle xTimer);
static void vTimerRCVSelfC2RqstTimeoutCallback(xTimerHandle xTimer);

// переменные------------------------------------------------------------------

static xTimerHandle xTimerSelfC2Rqst;
static xTimerHandle xTimerRCVSelfC2RqstTimeout;

static CAN_MyRxMsgTypeDef RxMsg; //структура принятого CAN-сообщения

static MESSAGE_C2_t MESSAGE_C2;
static MESSAGE_A1_t MESSAGE_A1;
static MESSAGE_B_CONFIG_t MESSAGE_B_CONFIG;
static MESSAGE_B_SERVICE_t MESSAGE_B_SERVICE;

static TM_CONTEXT_t 	*tmContext;			// ШВ
static FAIL_CONTEXT_t *fContext;			// отказы
static CAN_CONTEXT_t 	*canContext;		// CAN

//---------------------------------------инициализация CAN---------------------------------------//
void Init_CAN (void *arg)
{
	PORT_InitTypeDef sPort;
  CAN_InitTypeDef  sCAN;
	
	tmContext = &(((MKS2_t *)arg)->tmContext);
	fContext = &(((MKS2_t *)arg)->fContext);
	canContext = &(((MKS2_t *)arg)->canContext);
	
	canContext->MsgA1Send = &CAN_A1_Send;    //указатель на ф-ю отправки сообщения типа А1
	canContext->ID = MODULE_TYPE_MKNS, 					//инициализация типа модуля
	canContext->GetAddr = &Get_Module_Address, //инициализация указателя на функцию получения адреса в кросс-плате
	
	//инициализация пинов для CAN-модуля
	RST_CLK_PCLKcmd(CAN_PIN_CLOCK, ENABLE); //включение тактирования GPIO

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
	
	//инициализация CAN
	RST_CLK_PCLKcmd(CAN_CLOCK, ENABLE );
	CAN_BRGInit( MY_MDR_CAN, CAN_HCLKdiv1); // Set the HCLK division factor = 1 for CAN
	CAN_DeInit( MY_MDR_CAN );		// Reset CAN to POR
	CAN_StructInit (&sCAN);

	sCAN.CAN_ROP  = ENABLE; //получать собственные пакеты
//	sCAN.CAN_SAP  = ENABLE; //отвечать ACK на собственные пакеты	
	sCAN.CAN_STM  = DISABLE; //режим самотестирования
	sCAN.CAN_ROM  = DISABLE; //режим "только чтение"
	
	canContext->Addr = canContext->GetAddr(); //получение адреса в кросс-плате
	
	if ( MKS2.canContext.Addr == -1 ) // если модуль запущен без кросса
	{
		canContext->Addr = 0;
		sCAN.CAN_SAP  = ENABLE; //отвечать ACK на собственные пакеты	
	} 
	else 
		{sCAN.CAN_SAP  = DISABLE;} // не отвечать ACK на собственные пакеты
	
	// Nominal Bit Time (NBT) = 8 мкс (125Кбит/с);
	// Принимаем Tq = 0.5 мкс;
	// т.к. NBT = Tq * (Sync_Seg + PSEG + SEG1 + SEG2), где Sync_Seg = 1 (константа),
	// тогда PSEG + SEG1 + SEG2 = NBT/Tq - 1. Но надо соблюсти условие:  (8 * Tq) <= NBT <= (25 * Tq).
	// Дополнительные требования:
	// • PSEG + SEG1  >=  SEG2
	// • SEG2  >=  SJW (Sync Jump Width)		
	sCAN.CAN_PSEG = CAN_PSEG_Mul_2TQ;
	sCAN.CAN_SEG1 = CAN_SEG1_Mul_7TQ;
	sCAN.CAN_SEG2 = CAN_SEG2_Mul_6TQ;
	sCAN.CAN_SJW  = CAN_SJW_Mul_2TQ;
	sCAN.CAN_SB   = CAN_SB_3_SAMPLE;
	sCAN.CAN_OVER_ERROR_MAX = 0xFF;
	
	sCAN.CAN_BRP = CAN_BRP_VALUE;				

	CAN_Init(MY_MDR_CAN, &sCAN);

	//	буфер на прием собственных сообщений или запроса состояния
	// приемник на CAN_BUFFER_0	
	CAN_Buffer_RX_Init(MY_MDR_CAN, 0, DISABLE, (0x7FF << 18), MAKE_FRAME_ID(MSG_TYPE_C, (uint8_t)canContext->Addr)); // фильтр на сообщения C1, C2	
	//CAN_Buffer_RX_Init(MY_MDR_CAN, 0, DISABLE, (0x7FF << 18), 0xFFFF0000);
	
	// приемник на CAN_BUFFER_1
	CAN_Buffer_RX_Init(MY_MDR_CAN, 1, DISABLE, (0x7FF << 18), MAKE_FRAME_ID(MSG_TYPE_C, (uint8_t)canContext->Addr)); // фильтр на сообщения C1, C2
	
	//	буфер на передачу шкалы времени
	CAN_Buffer_TX_Init(MY_MDR_CAN, 2); // прередатчик на CAN_BUFFER_2	
	
	//	буфер на передачу сообщений С2 состояния модуля
	CAN_Buffer_TX_Init(MY_MDR_CAN, 3); 	// прередатчик на CAN_BUFFER_3
	
	//	буфер на прием сервисных сообщений,  приемник на CAN_BUFFER_4	
	CAN_Buffer_RX_Init(MY_MDR_CAN, 4, DISABLE, (0x7FF << 18), MAKE_FRAME_ID(MSG_TYPE_B, (uint8_t)canContext->Addr)); // фильтр на сообщения B	
	
	CAN_Cmd(MY_MDR_CAN, ENABLE);
	
	xTimerSelfC2Rqst = xTimer_Create(5000, ENABLE, &vTimerSelfC2RqstCallback, ENABLE); //создание таймера отсутствия соединения по CAN
}

//---------------------------------------------------------------------------------------------------//
void CAN_RX_Process(void)
{	
	if ( CAN_GetTEC(MY_MDR_CAN) > 127 )  //если количество ощибок больше 127
		{CAN_Cmd(MY_MDR_CAN, DISABLE);} //отключение CAN
		
	//	прием собственных сообщений или запроса состояния
	if (CAN_GetBufferStatus(MY_MDR_CAN, 0) & CAN_STATUS_RX_FULL) //проверка статуса буфера CAN_BUFFER_0 на наличие установленного флага "буфер полон"
	{
		CAN_MyGetRawReceivedData(MY_MDR_CAN, 0, &RxMsg); //получение данных из буфера	
		if (RxMsg.Rx_Header.DLC == 0x08 && RxMsg.Rx_Header.RTR == 0 && 
		(uint8_t)RxMsg.Data[0] == MAKE_MSG_DATA0(canContext->ID, 0)) //если получено собственное сообщение C2
		{
			xTimer_Delete(xTimerRCVSelfC2RqstTimeout); //удаление таймера таймаута получения сообщения САN
			fContext->CAN = 0; //статус CAN - исправно
		}	
		// запрос состояния модуля - сообщение C1
		if (RxMsg.Rx_Header.DLC == 0 && RxMsg.Rx_Header.RTR == 1) //если получено сообщение C1
		{
			CAN_C2_Send(); //отправка сообщения С2
			fContext->CAN = 0; //статус CAN - исправно
			xTimer_Reload(xTimerSelfC2Rqst); //перезагрузка таймера 
		/*	#ifdef __USE_DBG
				printf ("CAN0-get_C1\r\n");
			#endif*/
		}		
		CAN_ClearFlag(MY_MDR_CAN, 0, CAN_STATUS_RX_FULL); //очистка флага "буфер полон"
	}

	if ( CAN_GetBufferStatus(MY_MDR_CAN, 1) & CAN_STATUS_RX_FULL ) 	// проверка статуса буфера CAN_BUFFER_1 
	{
		CAN_MyGetRawReceivedData(MY_MDR_CAN, 1, &RxMsg); //получаем данные из буфера			
		if (RxMsg.Rx_Header.DLC == 0x08 && RxMsg.Rx_Header.RTR == 0 && 
				(uint8_t)RxMsg.Data[0] == MAKE_MSG_DATA0(canContext->ID, 0)) // собственное сообщение C2
		{
			xTimer_Delete(xTimerRCVSelfC2RqstTimeout); //сброс таймера
			fContext->CAN = 0; 												//статус CAN - исправно
		}
		
		if ( RxMsg.Rx_Header.DLC == 0 && RxMsg.Rx_Header.RTR == 1 )	// запрос состояния модуля - сообщение C1
		{
			CAN_C2_Send(); //отправка сообщения С2
			fContext->CAN = 0; //статус CAN - исправно
			xTimer_Reload(xTimerSelfC2Rqst); //перезагрузка таймера 
			/*#ifdef __USE_DBG
				printf ("CAN-get_C1\r\n");
			#endif*/
		}		
		CAN_ClearFlag(MY_MDR_CAN, 1, CAN_STATUS_RX_FULL); //очистка флага "буфер полон"
	}
	
	//прием сервисных сообщений
	if ( CAN_GetBufferStatus(MY_MDR_CAN, 4) & CAN_STATUS_RX_FULL ) // проверка статуса буфера CAN_BUFFER_4
	{
		CAN_MyGetRawReceivedData(MY_MDR_CAN, 4, &RxMsg); //получение данных из буфера
		if ( (uint8_t)RxMsg.Data[0] == MAKE_MSG_DATA0(canContext->ID, DATA_TYPE_CONFIG) ) //проверка 1 байта сообщения
		{
			memcpy(&MESSAGE_B_CONFIG.RAW[0], &RxMsg.Data[0], 8);		
			tmContext->Max_gDOP = ((float)MESSAGE_B_CONFIG.gDOP) / 100; //установка нового значения gDOP
		}

		if ((uint8_t)RxMsg.Data[0] == MAKE_MSG_DATA0(canContext->ID, DATA_TYPE_SERVICE))//сервисная перезагрузка
		{
			memcpy(&MESSAGE_B_SERVICE.RAW[0], &RxMsg.Data[0], 8);
			if (MESSAGE_B_SERVICE.reset) 
				{NVIC_SystemReset();}		
		}	
		CAN_ClearFlag(MY_MDR_CAN, 4, CAN_STATUS_RX_FULL); //очистка флага "буфер полон"
	}		
}

//---------------------------------------------------------------------------------------------------//
static void vTimerSelfC2RqstCallback(xTimerHandle xTimer)
{
	xTimerRCVSelfC2RqstTimeout = xTimer_Create(1100, DISABLE, &vTimerRCVSelfC2RqstTimeoutCallback, ENABLE);
	
	if ( CAN_GetCmdStatus(MY_MDR_CAN) != ENABLE ) //если CAN был отключён
		{CAN_Cmd(MY_MDR_CAN, ENABLE);}	//включения CAN
	
	CAN_C2_Send(); //передача сообщения С2
}

//---------------------------------таймаут получения сообщения по CAN---------------------------------//
static void vTimerRCVSelfC2RqstTimeoutCallback(xTimerHandle xTimer)
{
	fContext->CAN = 1; //CAN неиспрваен
	#ifdef __USE_DBG
		printf ("CAN_error\r\n");
	#endif
	xTimer_Delete(xTimer);
}


//---------------------------------------Отправка сообщения A1---------------------------------------//
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


//---------------------------------------Отправка сообщения C2---------------------------------------//
static void CAN_C2_Send(void)
{
	CAN_TxMsgTypeDef TxMsg;
	TxMsg.IDE     = CAN_ID_STD; //стандартный зоголовок
  TxMsg.DLC     = 0x08;  //8 байт
  TxMsg.PRIOR_0 = DISABLE;
  TxMsg.ID      = MAKE_FRAME_ID(MSG_TYPE_C, (uint8_t)MKS2.canContext.Addr);
 
	MESSAGE_C2.module_id = canContext->ID;
	MESSAGE_C2.data_type = 0;
	
	if ((fContext->Fail & FAIL_MASK) != 0 ) //флаг интегрального отказа, при наличии хотя бы одного флага отказа
	{
		MESSAGE_C2.fail = 1;
	} 
	else 
		{MESSAGE_C2.fail = 0;}
		
	MESSAGE_C2.fail_gps = fContext->GPS; //статус соедининея с GPS-модулем
	MESSAGE_C2.fail_gps_ant = fContext->GPSAntShortCircuit;
	MESSAGE_C2.gps_ant_disc = fContext->GPSAntDisconnect; 
	
	memcpy(&TxMsg.Data, &MESSAGE_C2, 8);

	CAN_ITClearErrorPendingBit(MY_MDR_CAN, CAN_STATUS_IDLOWER | CAN_STATUS_CRC_ERR | 
																		CAN_STATUS_FRAME_ERR | CAN_STATUS_ACK_ERR );
	
	CAN_Transmit(MY_MDR_CAN, 3, &TxMsg);
}



