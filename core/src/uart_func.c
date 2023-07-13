#include "main.h"
#include "typedef.h"
#include "HW_Profile.h"
#include "ring_buffer.h"
#include "uart_func.h"
#include "pins.h"

/* Private functions prototypes -----------------------------------------------*/
static void UART_TX_Data(MDR_UART_TypeDef* , const uint8_t* , uint16_t );
static void UART_LoLevel_Init(MDR_UART_TypeDef* , uint32_t , uint32_t , UartMode );
static void UART_InitIRQ(IRQn_Type IRQn, uint32_t priority);
static void UARTSetBaud(MDR_UART_TypeDef* , uint32_t , uint32_t );

/* Private define ------------------------------------------------------------*/

/* Private consts ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
char DBG_buffer[64];
RING_buffer_t RING_buffer; //��������� � ��������� ��������
uint8_t uart_buffer[BUFFER_SIZE]; //������ ��� ���������� �������
uint8_t count = 0;

//------------------������������� ������ UART------------------//
static void UART_LoLevel_Init(MDR_UART_TypeDef* UARTx, uint32_t UARTx_CLOCK, uint32_t uartBaudRate, UartMode mode)
{
  PORT_InitTypeDef GPIOInitStruct; // ��������� ��� ������������� ����� �����-������

  UART_InitTypeDef UARTInitStruct;  // ��������� ��� ������������� ������ UART

  RST_CLK_PCLKcmd(UARTx_CLOCK, ENABLE); // ���������� ������������ �����  � ������ UART

  // ����� ������������ ����� �����-������
  PORT_StructInit (&GPIOInitStruct);
  GPIOInitStruct.PORT_SPEED = PORT_SPEED_MAXFAST;
  GPIOInitStruct.PORT_MODE  = PORT_MODE_DIGITAL;

	switch ( mode ) 
	{
		case UART_RX_MODE:
			// ������������ � ������������� ����� ��� ������ ������ 
			RST_CLK_PCLKcmd(UART_CLOCK_Pin_RX , ENABLE);
			GPIOInitStruct.PORT_FUNC  = UART_PORT_FuncRX;
			GPIOInitStruct.PORT_OE    = PORT_OE_IN;
			GPIOInitStruct.PORT_Pin   = UART_PORT_PinRX;
			PORT_Init(UART_PORT_RX, &GPIOInitStruct);
			break;
		
		case UART_TX_MODE:
			// ������������ � ������������� ����� ��� �������� ������ 
			RST_CLK_PCLKcmd(UART_CLOCK_Pin_TX, ENABLE);
			GPIOInitStruct.PORT_FUNC  = UART_PORT_FuncTX;	
			GPIOInitStruct.PORT_OE    = PORT_OE_OUT;
			GPIOInitStruct.PORT_Pin   = UART_PORT_PinTX;
			PORT_Init(UART_PORT_TX, &GPIOInitStruct);
			break;	
		
		default:
			break;
	}

  // ������������ ������ UART
  UARTInitStruct.UART_BaudRate            = uartBaudRate;                  // �������� �������� ������
  UARTInitStruct.UART_WordLength          = UART_WordLength8b;             // ���������� ����� ������ � ���������
  UARTInitStruct.UART_StopBits            = UART_StopBits1;                // ���������� STOP-�����
  UARTInitStruct.UART_Parity              = UART_Parity_No;                // �������� ��������
  UARTInitStruct.UART_FIFOMode            = UART_FIFO_OFF;                 // ���������/���������� ������
  UARTInitStruct.UART_HardwareFlowControl = UART_HardwareFlowControl_TXE |
																						UART_HardwareFlowControl_RXE;   // ���������� �������� �� ��������� � ������� ������


  UART_Init(UARTx, &UARTInitStruct);  // ������������� ������ UART

  UART_BRGInit(UARTx, UART_HCLKdiv1);  // ����� ������������ �������� ������� ������ UART

	if ( mode == UART_RX_MODE) 
	{
		UART_ITConfig(UARTx, UART_IT_RX, ENABLE); // ����� ���������� ���������� (����� ������)
		UART_InitIRQ(UART_RX_IRQ, 1);
	}
	
  UART_Cmd(UARTx, ENABLE); // ���������� ������ ������ UART
}

//----------------------------------------------------------------------------------------
void DBG_LoLevel_Init(MDR_UART_TypeDef* UARTx, uint32_t UARTx_CLOCK, uint32_t uartBaudRate)
{
  PORT_InitTypeDef GPIOInitStruct; // ��������� ��� ������������� ����� �����-������

  UART_InitTypeDef UARTInitStruct;  // ��������� ��� ������������� ������ UART

  RST_CLK_PCLKcmd(UARTx_CLOCK, ENABLE); // ���������� ������������ �����  � ������ UART

  // ����� ������������ ����� �����-������
  PORT_StructInit (&GPIOInitStruct);
  GPIOInitStruct.PORT_SPEED = PORT_SPEED_MAXFAST;
  GPIOInitStruct.PORT_MODE  = PORT_MODE_DIGITAL;

			// ������������ � ������������� ����� ��� �������� ������ 
	RST_CLK_PCLKcmd(DBG_CLOCK_Pin_TX , ENABLE);
	GPIOInitStruct.PORT_FUNC  = DBG_PORT_FuncTX ;	
	GPIOInitStruct.PORT_OE    = PORT_OE_OUT;
	GPIOInitStruct.PORT_Pin   = DBG_PORT_PinTX;
	PORT_Init(UART_PORT_TX, &GPIOInitStruct);	

  // ������������ ������ UART
  UARTInitStruct.UART_BaudRate            = uartBaudRate;                  // �������� �������� ������
  UARTInitStruct.UART_WordLength          = UART_WordLength8b;             // ���������� ����� ������ � ���������
  UARTInitStruct.UART_StopBits            = UART_StopBits1;                // ���������� STOP-�����
  UARTInitStruct.UART_Parity              = UART_Parity_No;                // �������� ��������
  UARTInitStruct.UART_FIFOMode            = UART_FIFO_OFF;                 // ���������/���������� ������
  UARTInitStruct.UART_HardwareFlowControl = UART_HardwareFlowControl_TXE;   // ���������� �������� �� ��������� � ������� ������
                                          

  UART_Init(UARTx, &UARTInitStruct);  // ������������� ������ UART

  UART_BRGInit(UARTx, UART_HCLKdiv1);  // ����� ������������ �������� ������� ������ UART
  UART_Cmd(UARTx, ENABLE); // ���������� ������ ������ UART
}

//----------------------------------------------------------------------------------------------//
static void UART_InitIRQ(IRQn_Type IRQn, uint32_t priority)
{
  NVIC_SetPriority(IRQn, priority);  // ���������� ���������� ����������� ���������� �� UART

  NVIC_EnableIRQ(IRQn); // ���������� ���������� ���������� �� UART
}	

//----------------------------------------------------------------------------------------------//
static void UARTSetBaud(MDR_UART_TypeDef* UARTx, uint32_t baudRate, uint32_t freqCPU)
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

//----------------------------------------------------------------------------------------------//
void MNP_UART_Init (void)
{	
	UART_LoLevel_Init(UART_TX, UART_TX_CLOCK, UARTx_BAUD_RATE, UART_TX_MODE);  //������������� UART1 ��� ��������
	UARTSetBaud(UART_TX, UARTx_BAUD_RATE, CPU_CLOCK_VALUE);				
	
	UART_LoLevel_Init(UART_RX, UART_RX_CLOCK, UARTx_BAUD_RATE, UART_RX_MODE); //������������� UART2 ��� ���������
	UARTSetBaud(UART_RX, UARTx_BAUD_RATE, CPU_CLOCK_VALUE);
	
	RING_Init (&RING_buffer, uart_buffer, sizeof (uart_buffer)); //������������� ���������� �������
}

//-------------------------------------�������� ������� �� UART-------------------------------------//
static void UART_TX_Data(MDR_UART_TypeDef* UARTx, const uint8_t* data, uint16_t len)
{
	uint16_t count = 0;
	
	for (count = 0; count < len; count ++) 
	{
		while(UART_GetFlagStatus(UARTx, UART_FLAG_BUSY) == SET) {}
		UART_SendData(UARTx, data[count]);
	}
}

//----------------------------------------------------------------------------------------------//
void MNP_UART_MSG_Puts (const uint8_t* data, uint16_t len)
{
	UART_TX_Data(UART_TX, data, len);
}

//----------------------------------------------------------------------------------------------//
void DBG_PutString (char * str)
{
	char smb;
	
	while  ((smb = *str++) != 0)
	{
		while(UART_GetFlagStatus(DBG_TX, UART_FLAG_BUSY) == SET) {}
		UART_SendData(DBG_TX, smb);
	}
}

//-------------------------------��������� ������� �� UART1-----------------------------------//
void UART_CharReception_Callback (void)
{
	auto uint8_t smb;
	if ( UART_GetITStatusMasked(UART_RX, UART_IT_RX) == SET ) 
	{
    UART_ClearITPendingBit(UART_RX, UART_IT_RX);
		smb = UART_ReceiveData(UART_RX); // ���� ����� ������ �� GPS ���������  
		RING_Put(&RING_buffer, smb); //�������� ����� � ��������� ������
		count++;
 	}
}

