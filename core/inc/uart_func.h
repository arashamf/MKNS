#ifndef UART_FUNC_H
#define UART_FUNC_H

#include "main.h"
#include "typedef.h"
#include "ring_buffer.h"

//Private prototypes--------------------------------------------------------------//
void UART_LoLevel_Init(MDR_UART_TypeDef* , uint32_t , uint32_t , UartMode );
void DBG_LoLevel_Init(MDR_UART_TypeDef* , uint32_t , uint32_t );
void UART_InitIRQ(IRQn_Type IRQn, uint32_t priority);
void UARTSetBaud(MDR_UART_TypeDef* , uint32_t , uint32_t );
void UART_TX_Data(MDR_UART_TypeDef* , const uint8_t* , uint16_t );
void MNP_UART_MSG_Puts(const uint8_t *, uint16_t );
void DBG_PutString (char *);
void UART_CharReception_Callback (void);

extern char DBG_buffer[];
extern RING_buffer_t RING_buffer;
#endif
