/* Includes ------------------------------------------------------------------*/
#include "MDR32F9Qx_config.h"
#include "MDR32F9Qx_rst_clk.h"
#include "MDR32F9Qx_port.h"
#include "MDR32F9Qx_can.h"

#include <stdbool.h>
#include "Application.h"
#include "HW_Profile.h"
#include "XTick.h"
#include "CAN.h"

/* Private typedef -----------------------------------------------------------*/
typedef enum { RX_NONE, RX_C1, RX_OWN_C2 } TRxResult;
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Variables -----------------------------------------------------------------*/

extern uint32_t g_MyBackplaneAddress;		// адрес в кроссе

/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

//-----------------------------------------------------------------------------
void InitCAN( void )
//-----------------------------------------------------------------------------
{
  PORT_InitTypeDef sPort;
  CAN_InitTypeDef  sCAN;

	// Configure pins as CAN pins
	//
	PORT_StructInit( &sPort );

	sPort.PORT_MODE  = PORT_MODE_DIGITAL;
	sPort.PORT_SPEED = PORT_SPEED_SLOW;

	RST_CLK_PCLKcmd( PCLK_BIT(MY_CAN_RX_PORT), ENABLE );
	sPort.PORT_Pin   = MY_CAN_RX_PIN;	// CAN_RXx
	sPort.PORT_OE    = PORT_OE_IN;
	sPort.PORT_FUNC  = MY_CAN_RX_PORT_FUNC;
	PORT_Init( MY_CAN_RX_PORT, &sPort );

	RST_CLK_PCLKcmd( PCLK_BIT(MY_CAN_TX_PORT), ENABLE );
	sPort.PORT_Pin   = MY_CAN_TX_PIN;	// CAN_TXx
	sPort.PORT_OE    = PORT_OE_OUT;
	sPort.PORT_FUNC  = MY_CAN_TX_PORT_FUNC;
	PORT_Init( MY_CAN_TX_PORT, &sPort );

	// Configure CAN
	//

	// Enable peripheral clocks
	//
	RST_CLK_PCLKcmd( PCLK_BIT(MY_MDR_CAN) , ENABLE);

	CAN_BRGInit( MY_MDR_CAN, CAN_HCLKdiv1 );	// Set the HCLK division factor = 1 for CAN2
	CAN_DeInit( MY_MDR_CAN );					// Reset CAN to POR
	CAN_StructInit (&sCAN);

	sCAN.CAN_ROP  = ENABLE;		// Enable own packets reception. 
	sCAN.CAN_SAP  = ENABLE;		// Enable sending ACK on own packets.
	sCAN.CAN_STM  = DISABLE;	// Disable Self Test mode
	sCAN.CAN_ROM  = DISABLE;	// Disable Read Only mode

	// Nominal Bit Time (NBT) = 8 мкс (125Кбит/с);
	// Принимаем Tq = 0.5 мкс;
	// т.к. NBT = Tq * (Sync_Seg + PSEG + SEG1 + SEG2), где Sync_Seg = 1 (константа),
	// тогда PSEG + SEG1 + SEG2 = NBT/Tq - 1. Но надо соблюсти условие:  (8 * Tq) <= NBT <= (25 * Tq).
	// Дополнительные требования:
	// • PSEG + SEG1  >=  SEG2
	// • SEG2  >=  SJW (Sync Jump Width)
	//
	sCAN.CAN_PSEG = CAN_PSEG_Mul_2TQ;
	sCAN.CAN_SEG1 = CAN_SEG1_Mul_7TQ;
	sCAN.CAN_SEG2 = CAN_SEG2_Mul_6TQ;
	sCAN.CAN_SJW  = CAN_SJW_Mul_2TQ;
	sCAN.CAN_SB   = CAN_SB_3_SAMPLE;

	// Вычисления PSEG, SEG1 и SEG2 нужно производить вручную, однако, т.к. CPU_CLOCK_VALUE в процессе разработки
	// может меняться, будем автоматически вычислять делитель BRP ( MDR_CANx->BITMNG[ 15:0 ] ). Данный делитель делит
	// частоту CAN_CLOCK (равную CPU_CLOCK_VALUE, т.к. коэфф. деления HCLK = 1. См. выше вызов CAN_BRGInit. ). 
	// Для вычисления BRP используем след. формулу из datasheet:
	// • Tq (µs) = ((BRP+1))/CLK (MHz) 
	//
	// Значение Tq мы приняли равным 0.5 мкс, тогда...
	#define CAN_BRP_VALUE ( (CPU_CLOCK_VALUE / 1000000UL / 2UL) - 1 )
	sCAN.CAN_BRP = CAN_BRP_VALUE;				
	CAN_Init( MY_MDR_CAN, &sCAN );

	// Init CAN buffers

	// Передача сообщений C
	//
	MY_MDR_CAN->CAN_BUF[ CAN_MSG_TYPE_C_TX_BUFFER_NUM ].ID = MAKE_FRAME_ID( CAN_MSG_TYPE_C_ID, g_MyBackplaneAddress );
	MY_MDR_CAN->CAN_BUF[ CAN_MSG_TYPE_C_TX_BUFFER_NUM ].DLC = 8;
	MY_MDR_CAN->BUF_CON[ CAN_MSG_TYPE_C_TX_BUFFER_NUM ] = (CAN_STATUS_EN);

	// Прием сообщений С
	//
	MY_MDR_CAN->CAN_BUF_FILTER[ CAN_MSG_TYPE_C_RX_BUFFER_NUM ].MASK = 0x7FF << 18; // Проверяем все биты
	MY_MDR_CAN->CAN_BUF_FILTER[ CAN_MSG_TYPE_C_RX_BUFFER_NUM ].FILTER = MAKE_FRAME_ID( CAN_MSG_TYPE_C_ID, g_MyBackplaneAddress );
	MY_MDR_CAN->BUF_CON[ CAN_MSG_TYPE_C_RX_BUFFER_NUM ] = (CAN_STATUS_RX_TXn | CAN_STATUS_EN);

	MY_MDR_CAN->CAN_BUF_FILTER[ CAN_MSG_TYPE_C_RX_BUFFER_NUM2 ].MASK = 0x7FF << 18; // Проверяем все биты
	MY_MDR_CAN->CAN_BUF_FILTER[ CAN_MSG_TYPE_C_RX_BUFFER_NUM2 ].FILTER = MAKE_FRAME_ID( CAN_MSG_TYPE_C_ID, g_MyBackplaneAddress );
	MY_MDR_CAN->BUF_CON[ CAN_MSG_TYPE_C_RX_BUFFER_NUM2 ] = (CAN_STATUS_RX_TXn | CAN_STATUS_EN);

	// Enable CAN peripheral
	//
	CAN_Cmd( MY_MDR_CAN, ENABLE );

}

//-----------------------------------------------------------------------------
// Чтение сообщений C1 (от МИУ) и C2 (собственных)
//
static TRxResult RxMsgC( void )
//-----------------------------------------------------------------------------
{
  uint32_t rx_buffer_num;
  TCAN_MSG_TYPE_C_MKIP msg_c;
  uint32_t msg_dlc_value;
 
	if( MY_MDR_CAN->BUF_CON[ CAN_MSG_TYPE_C_RX_BUFFER_NUM ] & CAN_STATUS_RX_FULL ) //проверка флага получения сообщения
	{
		rx_buffer_num = CAN_MSG_TYPE_C_RX_BUFFER_NUM;
	}
	else if( MY_MDR_CAN->BUF_CON[ CAN_MSG_TYPE_C_RX_BUFFER_NUM2 ] & CAN_STATUS_RX_FULL )
	{
		rx_buffer_num = CAN_MSG_TYPE_C_RX_BUFFER_NUM2;
	}
	else
	{
		return RX_NONE;
	}

	// Считаем необходимые данные...
	//
	msg_dlc_value = MY_MDR_CAN->CAN_BUF[ rx_buffer_num ].DLC; //длина сообщения
	msg_c.DATAL = MY_MDR_CAN->CAN_BUF[ rx_buffer_num ].DATAL; //младшие 4 байта сообщения
	msg_c.DATAH = MY_MDR_CAN->CAN_BUF[ rx_buffer_num ].DATAH;  //старшие 4 байта сообщения

	// ... и освободим приемный буфер
	//
	MY_MDR_CAN->BUF_CON[ rx_buffer_num ] &= ~CAN_STATUS_RX_FULL;

	if( msg_dlc_value & CAN_DLC_RTR )
	{
		// Здесь если установлен RTR
		//
		return RX_C1;
	}
	else
	{
		// Здесь если не установлен RTR - проверим, что это наше собственное сообщение
		//
		if( ( msg_dlc_value & CAN_DLC_DATA_LENGTH ) == 8 && msg_c.data_type == 0 && msg_c.module_type == MY_MODULE_TYPE )
			return RX_OWN_C2;
	}

	return RX_NONE;
}

//-----------------------------------------------------------------------------
//
// Посылка сообщения C2
//
static void TxMsgC2( void )
//-----------------------------------------------------------------------------
{
  TCAN_MSG_TYPE_C_MKIP msg_c = { 0, 0 };

	MY_MDR_CAN->BUF_CON[ CAN_MSG_TYPE_C_TX_BUFFER_NUM ] &= ~CAN_BUF_CON_TX_REQ;

	msg_c.data_type = 0;
	msg_c.module_type = MY_MODULE_TYPE;

	msg_c.state = g_MyFlags.UPS_state;

	MY_MDR_CAN->CAN_BUF[ CAN_MSG_TYPE_C_TX_BUFFER_NUM ].DATAL = msg_c.DATAL;
	MY_MDR_CAN->CAN_BUF[ CAN_MSG_TYPE_C_TX_BUFFER_NUM ].DATAH = msg_c.DATAH;

	// Отправляем
	//
	MY_MDR_CAN->BUF_CON[ CAN_MSG_TYPE_C_TX_BUFFER_NUM ] |= CAN_BUF_CON_TX_REQ;
}

//-----------------------------------------------------------------------------
// Отправка ответов на запросы по CAN (посылка сообщений C2 в ответ на C1) и
// проверка работоспособности CAN
//
//-----------------------------------------------------------------------------
static void Task_ProcCANRequests_And_CheckCANCondition( void )
{
  static bool need_init = true;
  static uint32_t last_c2_tx_ticks, last_c2_rx_ticks;
  uint32_t current_ticks;

	current_ticks = GetXTick();

	if( need_init )
	{
		last_c2_tx_ticks = last_c2_rx_ticks = current_ticks;
		need_init = false;
	}
   	
	switch( RxMsgC() )
	{
		case RX_C1:

			// Получили запрос (сообщение C1) - отправляем сообщение C2
			// 			
			TxMsgC2();
			last_c2_tx_ticks = current_ticks;

		break;

		case RX_OWN_C2:

			// Получено собственное сообщение C2, сбрасываем флаг отказа CAN
			//
			g_MyFlags.CAN_Fail = 0;
			last_c2_rx_ticks = current_ticks;

		break;

		case RX_NONE:
		default:

		break;
	}

	if( current_ticks - last_c2_tx_ticks > 4 * TICKS_PER_SECOND )
	{
		// Если продолжительное время (4 секунды) не отправляли С2 (в ответ на С1),
		// сами отправим с целью контроля работоспособности CAN
		//
		TxMsgC2();
		last_c2_tx_ticks = current_ticks;
	}

	if( current_ticks - last_c2_rx_ticks > 5 * TICKS_PER_SECOND )
	{
		// Если собственное сообщение C2 не получаем уже продолжительное время (5 секунд),
		// устанавливаем флаг отказа CAN
		//
		g_MyFlags.CAN_Fail = 1;
	}
}

//-----------------------------------------------------------------------------
void TaskCAN( void )
{

	Task_ProcCANRequests_And_CheckCANCondition();
}
