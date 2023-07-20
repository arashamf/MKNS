#include "timers.h"
#include "MNP_msg.h"
#include "pins.h"
#include "main.h"

static xTIMER xTimerList[MAX_xTIMERS];
static portTickType xTimeNow;

//static void vTimerGPSAntADCCallback(xTimerHandle xTimer);				// детектирование антены GPS приемника (не поднлючена, подключена, КЗ)
//static void vTimerGPSAntPowerCallback(xTimerHandle xTimer);			// таймер управления питанием антены GPS приемника
static void vTimerGPSPPSCtrlCallback(xTimerHandle xTimer);			// таймер на разрешение выдачу сигнала PPS
static void vTimerGPSPPSTimeoutCallback(xTimerHandle xTimer); 	// таймаут на отсутствие PPS от GPS приемника
static void vTimerGPSUARTTimeoutCallback(xTimerHandle xTimer); 	// таймаут на отсутствие обмена по UART с GPS приемником
static void vTimerGPSCfgCallback(xTimerHandle xTimer); 

//static xTimerHandle xTimerGPSAntADC;			// детектирование антены GPS приемника (не поднлючена, подключена, КЗ)
//static xTimerHandle xTimerGPSAntPower;		// таймер управления питанием антены GPS приемника
static xTimerHandle xTimerGPSPPSCtrl;			// таймер на разрешение выдачу сигнала PPS 
static xTimerHandle xTimerGPSPPSTimeout; 	// таймаут на отсутствие PPS от GPS приемника
static xTimerHandle xTimerGPSUARTTimeout; // таймаут на отсутствие обмена по UART с GPS приемником
static xTimerHandle xTimerGPSCfg;					// таймер настройки GPS приемника

//-------------------------------------------------------------------------------------------------------------//
void xTimer_Init(uint32_t (*GetSysTick)(void))
{
	xTimeNow = GetSysTick;
}

//-------------------------------------------------------------------------------------------------------------//
xTimerHandle xTimer_Create(uint32_t xTimerPeriodInTicks, FunctionalState AutoReload, 
tmrTIMER_CALLBACK CallbackFunction, FunctionalState NewState)
{
	xTimerHandle NewTimer = NULL;
	uint16_t i;
	
	for (i = 0; i < MAX_xTIMERS; i++) {
		if (xTimerList[i].CallbackFunction == NULL) 
		{
			xTimerList[i].periodic = xTimerPeriodInTicks;
			xTimerList[i].AutoReload = AutoReload;
			xTimerList[i].CallbackFunction = CallbackFunction;
			
			if (NewState != DISABLE) 
			{
				xTimerList[i].expiry = xTimeNow() + xTimerPeriodInTicks; 
				xTimerList[i].State = __ACTIVE;
			} 
			else 
			{
				xTimerList[i].State = __IDLE;
			}		
			NewTimer = (xTimerHandle)(i + 1);
			break;
    }
  }
	return NewTimer;
}

//-------------------------------------------------------------------------------------------------------------//
void xTimer_SetPeriod(xTimerHandle xTimer, uint32_t xTimerPeriodInTicks) 
{
	if ( xTimer != NULL ) 
		{xTimerList[(uint32_t)xTimer-1].periodic = xTimerPeriodInTicks;}
}

//-------------------------------------------------------------------------------------------------------------//
void xTimer_Reload(xTimerHandle xTimer) 
{
	if ( xTimer != NULL ) 
	{
		xTimerList[(uint32_t)xTimer-1].expiry = xTimeNow() + xTimerList[(uint32_t)xTimer-1].periodic;
		xTimerList[(uint32_t)xTimer-1].State = __ACTIVE;
	}
}

//-------------------------------------------------------------------------------------------------------------//
void xTimer_Delete(xTimerHandle xTimer)
{
	if ( xTimer != NULL ) 
	{
		xTimerList[(uint32_t)xTimer-1].CallbackFunction = NULL;
		xTimerList[(uint32_t)xTimer-1].State = __IDLE;
		xTimer = NULL;
	}		
}

//-------------------------------------------------------------------------------------------------------------//
void xTimer_Task(uint32_t portTick)
{
	uint16_t i;
	
	for (i = 0; i < MAX_xTIMERS; i++) {
		switch (xTimerList[i].State) 
		{
			case __ACTIVE:
				if ( portTick >= xTimerList[i].expiry ) 
				{				
					if ( xTimerList[i].AutoReload != DISABLE ) 
						{xTimerList[i].expiry = portTick + xTimerList[i].periodic;} 
					else 
						{xTimerList[i].State = __IDLE;}			
					xTimerList[i].CallbackFunction((xTimerHandle)(i + 1));
				}					
				break;
				
			default:
				break;
		}
	}	
}

//-------------------------------------------------------------------------------------------------------------//
static void UART_TIMER_Init(void)
{
	TIMER_CntInitTypeDef TIMER_CntInitStructure;

	RST_CLK_PCLKcmd(RST_CLK_PCLK_TIMER2, ENABLE); //тактирование таймера

	TIMER_DeInit(MDR_TIMER2); //сброс настроек таймера

	TIMER_CntStructInit(&TIMER_CntInitStructure);

	TIMER_BRGInit(MDR_TIMER2, TIMER_HCLKdiv1);	// МК clock

	TIMER_CntInitStructure.TIMER_Prescaler				= 160 - 1; // 500kГц
	TIMER_CntInitStructure.TIMER_Period						= 50000 - 1;	// 100мС
	TIMER_CntInitStructure.TIMER_CounterMode			= TIMER_CntMode_ClkFixedDir;
	TIMER_CntInitStructure.TIMER_CounterDirection	= TIMER_CntDir_Up;	 
	TIMER_CntInitStructure.TIMER_ARR_UpdateMode		= TIMER_ARR_Update_Immediately;   
	TIMER_CntInitStructure.TIMER_IniCounter 			= 0;

	TIMER_CntInit(MDR_TIMER2, &TIMER_CntInitStructure);

	TIMER_ITConfig(MDR_TIMER2, TIMER_STATUS_CNT_ARR, ENABLE);
	NVIC_SetPriority(Timer2_IRQn, 10);
	NVIC_EnableIRQ(Timer2_IRQn);

	TIMER_Cmd(MDR_TIMER2, ENABLE);
}

//-----------------Прерывание таймера ожидания прихода новых данных от GPS приемника-----------------//
void UART_TIMER_Callback(void)
{
	if(TIMER_GetITStatus(MDR_TIMER2, TIMER_STATUS_CNT_ARR) == SET)
		{GPS_wait_data_Callback ();} //обрабока данных от приёмника	
	TIMER_ClearFlag(MDR_TIMER2, TIMER_STATUS_CNT_ARR);
}

//----------------------------------инициализация таймера 3 и таймера 1----------------------------------//
static void GPS_PPS_EXTI_Init(void)
{
	PORT_InitTypeDef 					PORT_InitStructure;
	
	TIMER_CntInitTypeDef 			TIMER_CntInitStructure;
	TIMER_ChnInitTypeDef			TIMER_ChnInitStructure;
	
	RST_CLK_PCLKcmd(RST_CLK_PCLK_TIMER1 | RST_CLK_PCLK_TIMER3 | RST_CLK_PCLK_PORTB, ENABLE);
	
	//инициализация канала 4 таймера 3 для захвата секундной метки приёмника
	TIMER_DeInit(MDR_TIMER3);
	
	TIMER_CntStructInit(&TIMER_CntInitStructure);	
	TIMER_CntInitStructure.TIMER_CounterDirection	= TIMER_CntDir_Up;
	TIMER_CntInitStructure.TIMER_ARR_UpdateMode		= TIMER_ARR_Update_Immediately; 
	
	TIMER_CntInitStructure.TIMER_Prescaler 				= 0;
	TIMER_CntInitStructure.TIMER_Period 					= 0xFFFF;
	TIMER_CntInitStructure.TIMER_IniCounter 			= 0;
	TIMER_CntInitStructure.TIMER_FilterSampling 	= TIMER_FDTS_TIMER_CLK_div_1;
	
	TIMER_CntInit(MDR_TIMER3, &TIMER_CntInitStructure);
	
	//CAPTURE CHANNEL4
	TIMER_ChnStructInit(&TIMER_ChnInitStructure);
	TIMER_ChnInitStructure.TIMER_CH_Mode 				= TIMER_CH_MODE_CAPTURE;
	TIMER_ChnInitStructure.TIMER_CH_ETR_Ena 		= DISABLE;
	TIMER_ChnInitStructure.TIMER_CH_EventSource = TIMER_CH_EvSrc_PE;
	TIMER_ChnInitStructure.TIMER_CH_CCR1_Ena 		= DISABLE;
	TIMER_ChnInitStructure.TIMER_CH_CCR1_EventSource = TIMER_CH_CCR1EvSrc_NE;
	TIMER_ChnInitStructure.TIMER_CH_FilterConf 	= TIMER_Filter_4FF_at_TIMER_CLK;
	TIMER_ChnInitStructure.TIMER_CH_Number 			= TIMER_CHANNEL4;
	
	TIMER_ChnInit(MDR_TIMER3, &TIMER_ChnInitStructure);

	//CAPTURE TIMER3 CHANNEL4 - PB7 
	PORT_StructInit(&PORT_InitStructure);
	PORT_InitStructure.PORT_Pin 	= PORT_Pin_7;
  PORT_InitStructure.PORT_FUNC 	= PORT_FUNC_OVERRID;
	PORT_InitStructure.PORT_OE 		= PORT_OE_IN;
	PORT_InitStructure.PORT_MODE 	= PORT_MODE_DIGITAL;
	PORT_InitStructure.PORT_SPEED = PORT_SPEED_MAXFAST;
  PORT_Init(MDR_PORTB, &PORT_InitStructure);
	
	TIMER_BRGInit(MDR_TIMER3, TIMER_HCLKdiv1);
	
	TIMER_ITConfig(MDR_TIMER3, TIMER_STATUS_CCR_CAP_CH4, ENABLE);
	NVIC_SetPriority(Timer3_IRQn, 9);
	NVIC_EnableIRQ(Timer3_IRQn);
	
	TIMER_Cmd(MDR_TIMER3, ENABLE);
	
	//инициализация таймера 1 для управления длительностью сигнала PPS от GPS приемника
	TIMER_DeInit(MDR_TIMER1);
	
	TIMER_CntStructInit(&TIMER_CntInitStructure);	
		
	TIMER_CntInitStructure.TIMER_Prescaler 				= 8 - 1; // 10МГц
	TIMER_CntInitStructure.TIMER_Period 					= 60 - 1;
	TIMER_CntInitStructure.TIMER_CounterMode			= TIMER_CntMode_ClkFixedDir;
	TIMER_CntInitStructure.TIMER_CounterDirection	= TIMER_CntDir_Up;
	TIMER_CntInitStructure.TIMER_ARR_UpdateMode		= TIMER_ARR_Update_Immediately; 
	TIMER_CntInitStructure.TIMER_IniCounter 			= 0;
	
	TIMER_CntInit(MDR_TIMER1, &TIMER_CntInitStructure);
	
	TIMER_BRGInit(MDR_TIMER1, TIMER_HCLKdiv1);
	
	TIMER_ITConfig(MDR_TIMER1, TIMER_STATUS_CNT_ARR, ENABLE);
	NVIC_SetPriority(Timer1_IRQn, 8);
	NVIC_EnableIRQ(Timer1_IRQn);
}

//-----------------------------Прерывание от сигнала PPS GPS приемника-----------------------------//
void GPS_PPS_IRQ_Callback(void)
{
	if (TIMER_GetITStatus(MDR_TIMER3, TIMER_STATUS_CCR_CAP_CH4) == SET) 
	{
		TIMER_ClearFlag(MDR_TIMER3, TIMER_STATUS_CCR_CAP_CH4); //сброс флага захвата канала4 таймера3
		
		if (MKS2.tmContext.Valid == 1) // проверка достоверности данных времени
		{
			TIMER_SetCounter(MDR_TIMER1, 0); //сброс счётчика таймера 1
			TIMER_Cmd(MDR_TIMER1, ENABLE); // запуск таймера 1, ограничивающего длительность импульса PPS			
			xTimerGPSPPSCtrl = xTimer_Create(850, DISABLE, &vTimerGPSPPSCtrlCallback, ENABLE); // запуск таймера на выдачу сигнала PPS  
		}
		//xTimer_Reload(xTimerGPSPPSTimeout); //перезагрузка таймера таймаута получения сигнала PPS
		xTimer_Delete(xTimerGPSPPSTimeout);  //удаление таймера таймаута получения сигнала PPS
	}	
}

//-------------------------Прерывание таймера1, ограничивающее длительность импульса PPS-------------------------//
void GPS_PPS_DISABLE_IRQ_Callback(void)
{
	if ( TIMER_GetITStatus(MDR_TIMER1, TIMER_STATUS_CNT_ARR) == SET ) 
	{
		TIMER_ClearFlag(MDR_TIMER1, TIMER_STATUS_CNT_ARR); //сброс флага	
		GPS_PPS_DISABLE(); // отключение сигнала PPS
		TIMER_Cmd(MDR_TIMER1, DISABLE); //выключение таймера 1
		
		#ifdef __USE_DBG
			printf ("PPS_end\r\n");
		#endif
		
		MKS2.tmContext.put_PPS = 1;
	}	
}

//-----------------------------------------Таймер на выдачу сигнала PPS-----------------------------------------//
static void vTimerGPSPPSCtrlCallback(xTimerHandle xTimer)
{
	if (MKS2.tmContext.Valid ) //проверка достоверности данных времени
	{
		GPS_PPS_ENABLE(); // включение сигнала PPS
		//создание таймера таймаута при отсутствии сигнала PPS от GPS приемника
		xTimerGPSPPSTimeout = xTimer_Create(500, DISABLE, &vTimerGPSPPSTimeoutCallback, ENABLE); 
	} 	
	else 
		{GPS_PPS_DISABLE();} // выключение сигнала PPS	

	xTimer_Delete(xTimerGPSPPSCtrl); //удаление таймера	
}

//-------------------------------Таймер таймаута получения сигнала PPS от приемника-------------------------------//
static void vTimerGPSPPSTimeoutCallback(xTimerHandle xTimer)
{
	GPS_PPS_DISABLE(); //выключение выдачи сигнала PPS
	
	MKS2.tmContext.ValidTHRESHOLD = 0; //сброс накопленной доствоерности
	MKS2.tmContext.Valid = 0;
	
	#ifdef __USE_DBG
		printf ("PPS_timeout\r\n");
	#endif
}

//-----------------------Обработка таймаута при отсутствии обмена по UART с GPS-приемником-----------------------//
static void vTimerGPSUARTTimeoutCallback(xTimerHandle xTimer)
{
	GPS_PPS_DISABLE(); //выключение выдачи сигнала PPS
	
	MKS2.tmContext.ValidTHRESHOLD = 0; //сброс накопленной доствоерности
	MKS2.tmContext.Valid = 0;
	
	MKS2.fContext.GPS = 1; //ошибка связи с gps-приемником
	
	#ifdef __USE_DBG
		printf ("GPS_timeout\r\n");
	#endif
	
	GPS_Config(); //перезагрузка и инициализация таймера 
}

//------------Перезагрузка таймера таймаута при отсутствии обмена по UART с GPS-приемником------------//
void Reload_Timer_GPS_UART_Timeout(void)					
{
	xTimer_Reload(xTimerGPSUARTTimeout);
}

//-------------------------Создание таймера перезагрузки и настройки приёмника-------------------------//
void Create_Timer_configure_GPS (void)
{
	xTimerGPSCfg = xTimer_Create(GPS_RST_DELAY, DISABLE, &vTimerGPSCfgCallback, ENABLE); 
}

//-------------------------коллбек таймера перезагрузки и настройки приёмника-------------------------//
static void vTimerGPSCfgCallback(xTimerHandle xTimer)
{
	switch(MNP_M7_CFG.cfg_state) 
	{
		case __SYNC_RST: //если стадия аппартной перезагрузки модуля
			MNP_M7_CFG.cfg_state = __SYNC_LOAD_CFG; //установка стадии отправки конфигурационных сообщений
			GPS_Reset (DISABLE); //отключение пина аппаратной перезагрузки модуля			
			xTimer_SetPeriod(xTimerGPSCfg, GPS_CFG_MSG_DELAY); //установка периода таймера настройки приёмника 75 мс
			xTimer_Reload(xTimerGPSCfg); //перезагрузка таймера настройки приемника
			break;
		
		case __SYNC_LOAD_CFG: //стадия отправки конф. сообщений приёмнику
			GPS_Init();
			xTimer_Delete(xTimerGPSCfg); //удаление таймера	
			break;

		default:
			break;
	}
}

//--------------------------------инициализация аппаратных и программных таймеров--------------------------------//
void timers_ini (void)
{
	UART_TIMER_Init(); //инициализация таймера таймаута при отсутствии обмена по UART с GPS-приемником
	GPS_PPS_EXTI_Init (); //инициализация таймера 3 и таймера 1
	
	//таймер таймаута при отсутствии сигнала PPS от GPS приемника
	//xTimerGPSPPSTimeout = xTimer_Create(1100, ENABLE, &vTimerGPSPPSTimeoutCallback, ENABLE);
 	
	//таймер таймаута при отсутствии получения сообщений по UART от GPS приемника
	xTimerGPSUARTTimeout = xTimer_Create(5000, ENABLE, &vTimerGPSUARTTimeoutCallback, ENABLE); //перезагрузка и инициализация GPS-модуля через 5с 
	
	//таймер детектирования антены (не подключена, подключена, КЗ)
	//xTimerGPSAntADC = xTimer_Create(250, ENABLE, &vTimerGPSAntADCCallback, ENABLE); 						
}
