#include "main.h"
#include "typedef.h"
#include "HW_Profile.h"
#include "ring_buffer.h"
#include "uart_func.h"
#include "pins.h"

/* Private define ------------------------------------------------------------*/

/* Private consts ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
char DBG_buffer[64];
RING_buffer_t RING_buffer; //структура с кольцевым буффером
//uint8_t uart_buffer[BUFFER_SIZE]; //массив для кольцевого буффера
uint8_t count = 0;

//------------------Инициализация модуля UART------------------//
void UART_LoLevel_Init(MDR_UART_TypeDef* UARTx, uint32_t UARTx_CLOCK, uint32_t uartBaudRate, UartMode mode)
{
  PORT_InitTypeDef GPIOInitStruct; // Структура для инициализации линий ввода-вывода

  UART_InitTypeDef UARTInitStruct;  // Структура для инициализации модуля UART

  RST_CLK_PCLKcmd(UARTx_CLOCK, ENABLE); // Разрешение тактирования порта  и модуля UART

  // Общая конфигурация линий ввода-вывода
  PORT_StructInit (&GPIOInitStruct);
  GPIOInitStruct.PORT_SPEED = PORT_SPEED_MAXFAST;
  GPIOInitStruct.PORT_MODE  = PORT_MODE_DIGITAL;

	switch ( mode ) 
	{
		case UART_RX_MODE:
			// Конфигурация и инициализация линии для приема данных 
			RST_CLK_PCLKcmd(UART_CLOCK_Pin_RX , ENABLE);
			GPIOInitStruct.PORT_FUNC  = UART_PORT_FuncRX;
			GPIOInitStruct.PORT_OE    = PORT_OE_IN;
			GPIOInitStruct.PORT_Pin   = UART_PORT_PinRX;
			PORT_Init(UART_PORT_RX, &GPIOInitStruct);
			break;
		
		case UART_TX_MODE:
			// Конфигурация и инициализация линии для передачи данных 
			RST_CLK_PCLKcmd(UART_CLOCK_Pin_TX, ENABLE);
			GPIOInitStruct.PORT_FUNC  = UART_PORT_FuncTX;	
			GPIOInitStruct.PORT_OE    = PORT_OE_OUT;
			GPIOInitStruct.PORT_Pin   = UART_PORT_PinTX;
			PORT_Init(UART_PORT_TX, &GPIOInitStruct);
			break;	
		
		default:
			break;
	}

  // Конфигурация модуля UART
  UARTInitStruct.UART_BaudRate            = uartBaudRate;                  // Скорость передачи данных
  UARTInitStruct.UART_WordLength          = UART_WordLength8b;             // Количество битов данных в сообщении
  UARTInitStruct.UART_StopBits            = UART_StopBits1;                // Количество STOP-битов
  UARTInitStruct.UART_Parity              = UART_Parity_No;                // Контроль четности
  UARTInitStruct.UART_FIFOMode            = UART_FIFO_OFF;                 // Включение/отключение буфера
  UARTInitStruct.UART_HardwareFlowControl = UART_HardwareFlowControl_TXE |
																						UART_HardwareFlowControl_RXE;   // Аппаратный контроль за передачей и приемом данных


  UART_Init(UARTx, &UARTInitStruct);  // Инициализация модуля UART

  UART_BRGInit(UARTx, UART_HCLKdiv1);  // Выбор предделителя тактовой частоты модуля UART

	if ( mode ==  UART_RX_MODE) 
	{
		UART_ITConfig(UARTx, UART_IT_RX, ENABLE); // Выбор источников прерываний (прием данных)
		UART_InitIRQ(UART_RX_IRQ, 1);
	}
	
  UART_Cmd(UARTx, ENABLE); // Разрешение работы модуля UART
}

//----------------------------------------------------------------------------------------
void DBG_LoLevel_Init(MDR_UART_TypeDef* UARTx, uint32_t UARTx_CLOCK, uint32_t uartBaudRate)
{
  PORT_InitTypeDef GPIOInitStruct; // Структура для инициализации линий ввода-вывода

  UART_InitTypeDef UARTInitStruct;  // Структура для инициализации модуля UART

  RST_CLK_PCLKcmd(UARTx_CLOCK, ENABLE); // Разрешение тактирования порта  и модуля UART

  // Общая конфигурация линий ввода-вывода
  PORT_StructInit (&GPIOInitStruct);
  GPIOInitStruct.PORT_SPEED = PORT_SPEED_MAXFAST;
  GPIOInitStruct.PORT_MODE  = PORT_MODE_DIGITAL;

			// Конфигурация и инициализация линии для передачи данных 
	RST_CLK_PCLKcmd(DBG_CLOCK_Pin_TX , ENABLE);
	GPIOInitStruct.PORT_FUNC  = DBG_PORT_FuncTX ;	
	GPIOInitStruct.PORT_OE    = PORT_OE_OUT;
	GPIOInitStruct.PORT_Pin   = DBG_PORT_PinTX;
	PORT_Init(UART_PORT_TX, &GPIOInitStruct);	

  // Конфигурация модуля UART
  UARTInitStruct.UART_BaudRate            = uartBaudRate;                  // Скорость передачи данных
  UARTInitStruct.UART_WordLength          = UART_WordLength8b;             // Количество битов данных в сообщении
  UARTInitStruct.UART_StopBits            = UART_StopBits1;                // Количество STOP-битов
  UARTInitStruct.UART_Parity              = UART_Parity_No;                // Контроль четности
  UARTInitStruct.UART_FIFOMode            = UART_FIFO_OFF;                 // Включение/отключение буфера
  UARTInitStruct.UART_HardwareFlowControl = UART_HardwareFlowControl_TXE;   // Аппаратный контроль за передачей и приемом данных
                                          

  UART_Init(UARTx, &UARTInitStruct);  // Инициализация модуля UART

  UART_BRGInit(UARTx, UART_HCLKdiv1);  // Выбор предделителя тактовой частоты модуля UART
  UART_Cmd(UARTx, ENABLE); // Разрешение работы модуля UART
}

//------------------
void UART_InitIRQ(IRQn_Type IRQn, uint32_t priority)
{
  NVIC_SetPriority(IRQn, priority);  // Назначение приоритета аппаратного прерывания от UART

  NVIC_EnableIRQ(IRQn); // Разрешение аппаратных прерываний от UART
}	

//------------------
void UARTSetBaud(MDR_UART_TypeDef* UARTx, uint32_t baudRate, uint32_t freqCPU)
{
	uint32_t divider = freqCPU / (baudRate >> 2);
	uint32_t CR_tmp = UARTx->CR;
	uint32_t LCR_tmp = UARTx->LCR_H;
	
	while ( (UARTx->FR & UART_FLAG_BUSY) ) 
		{__NOP();}		

  UARTx->CR = 0;
  UARTx->IBRD = divider >> 6;
  UARTx->FBRD = divider & 0x003F;
  UARTx->LCR_H = LCR_tmp;
  UARTx->CR = CR_tmp;
}

//------------------
void UART_TX_Data(MDR_UART_TypeDef* UARTx, const uint8_t* data, uint16_t len)
{
	uint16_t count = 0;
	
	for (count = 0; count < len; count ++) 
	{
		while(UART_GetFlagStatus(UARTx, UART_FLAG_BUSY) == SET);
		UART_SendData(UARTx, data[count]);
	}
}

//------------------
void MNP_UART_MSG_Puts (const uint8_t* data, uint16_t len)
{
	UART_TX_Data(UART_TX, data, len);
}

//------------------
void DBG_PutString (char * str)
{
	char smb;
	
	while  ((smb = *str++) != 0)
	{
		while(UART_GetFlagStatus(DBG_TX, UART_FLAG_BUSY) == SET) {}
		UART_SendData(DBG_TX, smb);
	}
}

//-------------------------------получение символа по UART1-----------------------------------//
void UART_CharReception_Callback (void)
{
	auto uint8_t smb;
	if ( UART_GetITStatusMasked(UART_RX, UART_IT_RX) == SET ) 
	{
    UART_ClearITPendingBit(UART_RX, UART_IT_RX);
		smb = UART_ReceiveData(UART_RX); // приём байта данных от GPS приемника  
		RING_Put(&RING_buffer, smb); //отправка байта в кольцевой буффер
		count++;
	/*	if (count < 100)
		{
			PORT_ResetBits( LED_RED_PORT, LED_RED_PIN); //GREEN
			PORT_SetBits( LED_GREEN_PORT, LED_GREEN_PIN);
		}
		if (count > 100)
		{
			PORT_SetBits(LED_RED_PORT, LED_RED_PIN); //RED
			PORT_ResetBits(LED_GREEN_PORT, LED_GREEN_PIN);
			count = 0;
		}
		if (count > 200)
			count = 0;*/
 	}
}

