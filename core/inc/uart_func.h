#ifndef UART_FUNC_H
#define UART_FUNC_H

#include "main.h"
#include "typedef.h"
#include "ring_buffer.h"

//Private prototypes--------------------------------------------------------------//
void DBG_LoLevel_Init(MDR_UART_TypeDef* , uint32_t , uint32_t );
void MNP_UART_Init (void);
void MNP_UART_MSG_Puts(const uint8_t *, uint16_t );
void DBG_PutString (char *);
void UART_CharReception_Callback (void);

extern char DBG_buffer[];
extern RING_buffer_t RING_buffer;
extern uint8_t uart_buffer[];
#endif
