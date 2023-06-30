#include "MDR32F9Qx_config.h"
#include "MDR32F9Qx_rst_clk.h"
#include <MDR32F9Qx_uart.h>
#include <stddef.h>

#include "uart_func.h"

/* Private define ------------------------------------------------------------*/
#define UART_COUNT 2

/* Private consts ------------------------------------------------------------*/
/*
const uint16_t UART_TX_PORT[ UART_COUNT ]		= { UART1_TX_PORT, UART2_TX_PORT};
const uint16_t UART_TX_PIN[ UART_COUNT ]		= { UART1_TX_PIN, UART2_TX_PIN};
const uint16_t UART_TX_PORT_FUNCTION[ UART_COUNT ]	= { UART1_TX_PORT_FUNCTION, UART2_TX_PORT_FUNCTION};

const uint16_t UART_RX_PORT[ UART_COUNT ]		= { UART1_RX_PORT, UART2_RX_PORT};
const uint16_t UART_RX_PIN[ UART_COUNT ]		= { UART1_RX_PIN, UART2_RX_PIN};
const uint16_t UART_RX_PORT_FUNCTION[ UART_COUNT ]	= { UART1_RX_PORT_FUNCTION, UART2_RX_PORT_FUNCTION };
*/
 #define UART_RX_BUFFER_SIZE 256u	
 #define UART_TX_BUFFER_SIZE 256u	

#if ( UART_RX_BUFFER_SIZE == 0 || (UART_RX_BUFFER_SIZE & (UART_RX_BUFFER_SIZE-1)) != 0 )
//	#error Размер буфера должен быть степенью двойки!
#endif

#if ( UART_TX_BUFFER_SIZE == 0 || (UART_TX_BUFFER_SIZE & (UART_TX_BUFFER_SIZE-1)) != 0 )
	//#error Размер буфера должен быть степенью двойки!
#endif

static MDR_UART_TypeDef* const m_UART_Ports[ 2 ] = { MDR_UART1, MDR_UART2 };

static uint16_t	m_UartRxHead[ 2 ]; //буфферы счётчика для UART1, UART2
static uint16_t	m_UartRxTail[ 2 ];

static uint16_t	m_UartTxHead[ 2 ];
static uint16_t	m_UartTxTail[ 2 ];

static uint8_t m_UartRxBuffer[ 2 ][ UART_RX_BUFFER_SIZE ];
static uint8_t m_UartTxBuffer[ 2 ][ UART_TX_BUFFER_SIZE ];

#pragma push
#pragma O2
#pragma Otime

//-----------------------------------------------------------------------------
void UartOpen( UART_PORT Port, uint32_t BaudRate, UART_WORDLENGTH WordLength, UART_PARITY Parity, UART_STOPBITS StopBits, uint32_t IRQ_Priority )
//-----------------------------------------------------------------------------
{
  UART_InitTypeDef init;	

	RST_CLK_PCLKcmd( PCLK_BIT( m_UART_Ports[ Port ] ), ENABLE );

	UART_ITConfig( m_UART_Ports[ Port ], UART_IT_TX | UART_IT_RX, DISABLE );
	UART_Cmd( m_UART_Ports[ Port ], DISABLE);
	UART_StructInit( &init );

	init.UART_BaudRate = BaudRate;
	init.UART_WordLength = WordLength;
	init.UART_Parity = Parity;
	init.UART_StopBits = StopBits;
	init.UART_FIFOMode = UART_FIFO_OFF; 
	init.UART_HardwareFlowControl = UART_HardwareFlowControl_RXE | UART_HardwareFlowControl_TXE;

	UART_BRGInit( m_UART_Ports[ Port ], UART_HCLKdiv1 );
	UART_Init( m_UART_Ports[ Port ], &init );

	m_UartTxHead[ Port ] = m_UartTxTail[ Port ] = m_UartRxHead[ Port ] = m_UartRxTail[ Port ] = 0;

	UART_ITConfig( m_UART_Ports[ Port ], UART_IT_TX | UART_IT_RX, ENABLE );
	UART_Cmd( m_UART_Ports[ Port ], ENABLE);
	
	if( Port == UART1 )
	{
		NVIC_EnableIRQ( UART1_IRQn );
		NVIC_SetPriority( UART1_IRQn, IRQ_Priority );
	}
	else if( Port == UART2 )
	{
		NVIC_EnableIRQ( UART2_IRQn );
		NVIC_SetPriority( UART2_IRQn, IRQ_Priority );
	}

}

//-----------------------------------------------------------------------------
void UartClose( UART_PORT Port )
//-----------------------------------------------------------------------------
{

	if( Port == UART1 )
	{
		NVIC_DisableIRQ( UART1_IRQn );
	}
	else if( Port == UART2 )
	{
		NVIC_DisableIRQ( UART2_IRQn );
	}

	UART_DeInit( m_UART_Ports[ Port ] );
	UART_Cmd( m_UART_Ports[ Port ], DISABLE );
}

//-----------------------------------------------------------------------------
uint16_t UartRxQue( UART_PORT Port )
//-----------------------------------------------------------------------------
{
  uint16_t count;

	m_UART_Ports[ Port ]->IMSC &= ~UART_IT_RX;
	count = (m_UartRxTail[ Port ] - m_UartRxHead[ Port ]) & (2 * UART_RX_BUFFER_SIZE - 1);
	m_UART_Ports[ Port ]->IMSC |= UART_IT_RX;

	return count;

}

//-----------------------------------------------------------------------------
bool UartRxReady( UART_PORT Port )
//-----------------------------------------------------------------------------
{
	return (m_UartRxTail[ Port ] != m_UartRxHead[ Port ]);
}

//-----------------------------------------------------------------------------
uint16_t UartTxQue( UART_PORT Port )
//-----------------------------------------------------------------------------
{
  uint16_t count;

	m_UART_Ports[ Port ]->IMSC &= ~UART_IT_TX;
	count = (m_UartTxTail[ Port ] - m_UartTxHead[ Port ]) & (2 * UART_TX_BUFFER_SIZE - 1);
	m_UART_Ports[ Port ]->IMSC |= UART_IT_TX;

	return count;

}
//-----------------------------------------------------------------------------
uint16_t UartTxQueRoom( UART_PORT Port )
//-----------------------------------------------------------------------------
{
	return UART_TX_BUFFER_SIZE - UartTxQue( Port );
}


//-----------------------------------------------------------------------------
void UartRxClear( UART_PORT Port )
//-----------------------------------------------------------------------------
{
	m_UART_Ports[ Port ]->IMSC &= ~UART_IT_RX;
	m_UartRxHead[ Port ] = m_UartRxTail[ Port ] = 0; //обнуление счётчиков
	m_UART_Ports[ Port ]->IMSC |= UART_IT_RX;
}

//-----------------------------------------------------------------------------
void UartTxClear( UART_PORT Port )
//-----------------------------------------------------------------------------
{
	m_UART_Ports[ Port ]->IMSC &= ~UART_IT_TX;
	m_UartTxHead[ Port ] = m_UartTxTail[ Port ] = 0; 
}

//-----------------------------------------------------------------------------
void UartPutc( UART_PORT Port, uint8_t Smb )
//-----------------------------------------------------------------------------
{
	m_UartTxBuffer[ Port ][ m_UartTxTail[ Port ] & (UART_TX_BUFFER_SIZE-1) ] = Smb;

	m_UART_Ports[ Port ]->IMSC &= ~UART_IT_TX;

	m_UartTxTail[ Port ]++;
	m_UartTxTail[ Port ] &= ( 2 * UART_TX_BUFFER_SIZE - 1 );

	// Если буферный регистр передатчика пуст, записываем в него первый байт из очереди.
	// Это надо сделать, т.к. прерывание передатчика срабатывает "по фронту", т.е. только тогда, 
	// когда символ переписывается из буферного в сдвиговый регистр.
	// Не айс, Миландр.
	//
	if( m_UART_Ports[ Port ]->FR & UART_FLAG_TXFE )
	{
		m_UART_Ports[ Port ]->DR = m_UartTxBuffer[ Port ][ m_UartTxHead[ Port ] & (UART_TX_BUFFER_SIZE-1) ];
		m_UartTxHead[ Port ]++;
		m_UartTxHead[ Port ] &= ( 2 * UART_TX_BUFFER_SIZE - 1 );
	}
	m_UART_Ports[ Port ]->IMSC |= UART_IT_TX;

}

//-----------------------------------------------------------------------------
void UartPuts( UART_PORT Port, const uint8_t *Src,  uint16_t Count )
//-----------------------------------------------------------------------------
{
  uint16_t i, j;

    i = m_UartTxTail[ Port ] & (UART_TX_BUFFER_SIZE-1);
	j = Count;

    while( j-- )
    {
        m_UartTxBuffer[ Port ][ i & (UART_TX_BUFFER_SIZE-1)] = *Src++;
        i = (i + 1) & (2 * UART_TX_BUFFER_SIZE - 1);
    }

		m_UART_Ports[ Port ]->IMSC &= ~UART_IT_TX;
    m_UartTxTail[ Port ] += Count;
    m_UartTxTail[ Port ] &= (2 * UART_TX_BUFFER_SIZE - 1);

	// См. комментарий к UartPutc
	//
	if( m_UART_Ports[ Port ]->FR & UART_FLAG_TXFE )
	{
		m_UART_Ports[ Port ]->DR = m_UartTxBuffer[ Port ][ m_UartTxHead[ Port ] & (UART_TX_BUFFER_SIZE-1) ];
		m_UartTxHead[ Port ]++;
		m_UartTxHead[ Port ] &= ( 2 * UART_TX_BUFFER_SIZE - 1 );
	}
	m_UART_Ports[ Port ]->IMSC |= UART_IT_TX;
}

//-----------------------------------------------------------------------------
uint8_t UartGetc( UART_PORT Port )
//-----------------------------------------------------------------------------
{
  uint8_t smb;

	smb = m_UartRxBuffer[ Port ][ m_UartRxHead[ Port ] & (UART_RX_BUFFER_SIZE-1) ]; //получение символа

	m_UART_Ports[ Port ]->IMSC &= ~UART_IT_RX;
	m_UartRxHead[ Port ]++; //увеличение счётчика обработанных символов
	m_UartRxHead[ Port ] &= ( 2 * UART_RX_BUFFER_SIZE - 1 );
	m_UART_Ports[ Port ]->IMSC |= UART_IT_RX;

	return smb;
}

//-----------------------------------------------------------------------------
void UartGets( UART_PORT Port, uint8_t *Dst, uint16_t Count )
//-----------------------------------------------------------------------------
{
  uint16_t i, j;

    i = m_UartRxHead[ Port ] & (UART_RX_BUFFER_SIZE-1);
	j = Count;

    while( j-- )
    {
        *Dst++ = m_UartRxBuffer[ Port ][ i & (UART_RX_BUFFER_SIZE-1)];
        i = (i + 1) & (2 * UART_RX_BUFFER_SIZE - 1);
    }


	m_UART_Ports[ Port ]->IMSC &= ~UART_IT_RX;
	m_UartRxHead[ Port ] += Count;
	m_UartRxHead[ Port ] &= ( 2 * UART_RX_BUFFER_SIZE - 1 );
	m_UART_Ports[ Port ]->IMSC |= UART_IT_RX;

}

//-----------------------------------------------------------------------------
void UART1_IRQHandler(void)
//-----------------------------------------------------------------------------
{
  uint16_t rx;

	if( m_UART_Ports[ UART1 ]->MIS & UART_IT_TX )
	{
		if( m_UartTxTail[ UART1 ] != m_UartTxHead[ UART1 ] )
		{
			m_UART_Ports[ UART1 ]->DR = m_UartTxBuffer[ UART1 ][ m_UartTxHead[ UART1 ] & (UART_TX_BUFFER_SIZE-1) ];
			m_UartTxHead[ UART1 ]++;
			m_UartTxHead[ UART1 ] &= ( 2 * UART_TX_BUFFER_SIZE - 1 );
		}
		else
		{
			m_UART_Ports[ UART1 ]->IMSC &= ~UART_IT_TX;
		}
		m_UART_Ports[ UART1 ]->ICR |= UART_IT_TX;
	}

	if( m_UART_Ports[ UART1 ]->MIS & UART_IT_RX )
	{
		rx = m_UART_Ports[ UART1 ]->DR;
		if( m_UartRxTail[ UART1 ] != (m_UartRxHead[ UART1 ] ^ UART_RX_BUFFER_SIZE ))
		{
			m_UartRxBuffer[ UART1 ][ m_UartRxTail[ UART1 ] & (UART_RX_BUFFER_SIZE-1) ] = rx;
			m_UartRxTail[ UART1 ]++; //увеличение счётчика полученных символов
			m_UartRxTail[ UART1 ] &= ( 2 * UART_RX_BUFFER_SIZE - 1 );
		}
		m_UART_Ports[ UART1 ]->ICR |= UART_IT_RX;
	}
}

//-----------------------------------------------------------------------------
void UART2_IRQHandler(void)
//-----------------------------------------------------------------------------
{
  uint16_t rx;

	if( m_UART_Ports[ UART2 ]->MIS & UART_IT_TX )
	{
		if( m_UartTxTail[ UART2 ] != m_UartTxHead[ UART2 ] )
		{
			m_UART_Ports[ UART2 ]->DR = m_UartTxBuffer[ UART2 ][ m_UartTxHead[ UART2 ] & (UART_TX_BUFFER_SIZE-1) ];
			m_UartTxHead[ UART2 ]++;
			m_UartTxHead[ UART2 ] &= ( 2 * UART_TX_BUFFER_SIZE - 1 );
		}
		else
		{
			m_UART_Ports[ UART2 ]->IMSC &= ~UART_IT_TX;
		}
		m_UART_Ports[ UART2 ]->ICR |= UART_IT_TX;
	}

	if( m_UART_Ports[ UART2 ]->MIS & UART_IT_RX )
	{
		rx = m_UART_Ports[ UART2 ]->DR;
		if( m_UartRxTail[ UART2 ] != ( m_UartRxHead[ UART2 ] ^ UART_RX_BUFFER_SIZE ) )
		{
			m_UartRxBuffer[ UART2 ][ m_UartRxTail[ UART2 ] & (UART_RX_BUFFER_SIZE-1) ] = rx;
			m_UartRxTail[ UART2 ]++;
			m_UartRxTail[ UART2 ] &= ( 2 * UART_RX_BUFFER_SIZE - 1 );
		}
		m_UART_Ports[ UART2 ]->ICR |= UART_IT_RX;
	}
}

#pragma pop
