#ifndef VTR_UART_HELPER_H
#define VTR_UART_HELPER_H

#include <stdbool.h>
#include <stdint.h>

//-----------------------------------------------------------------------------
typedef enum { UART1 = 0, UART2 } UART_PORT;

typedef enum { WORDLENGTH_8 = 0x60, WORDLENGTH_7 = 0x40, WORDLENGTH_6 = 0x20, WORDLENGTH_5 = 0 } UART_WORDLENGTH;
typedef enum { PARITY_NO = 0, PARITY_ODD = 0x02, PAITY_EVEN = 0x06, PARITY_MARK = 0x82, PARITY_SPACE = 0x86 } UART_PARITY;
typedef enum { STOPBITS_1 = 0, STOPBITS_2 = 0x08 } UART_STOPBITS;

void UartOpen( UART_PORT Port, uint32_t BaudRate, UART_WORDLENGTH, UART_PARITY, UART_STOPBITS, uint32_t IRQ_Priority );
void UartClose( UART_PORT Port );
uint16_t UartRxQue( UART_PORT Port ); // Возвращает количество символов, находящееся в приемном буфере
uint16_t UartTxQue( UART_PORT Port );
uint16_t UartTxQueRoom( UART_PORT Port ); // Возвращает размер свободного места в буфере передачи
void UartRxClear( UART_PORT Port ); // Очистка буфера приема
void UartTxClear( UART_PORT Port );
bool UartRxReady( UART_PORT Port ); 
void UartPutc( UART_PORT Port, uint8_t Smb );
void UartPuts( UART_PORT Port, const uint8_t *Src,  uint16_t Count );
uint8_t UartGetc( UART_PORT Port );
void UartGets( UART_PORT Port, uint8_t *Dst, uint16_t Count );


#endif
