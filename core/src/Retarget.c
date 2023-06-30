#if defined (_USE_DEBUG_PRINTF_UART1_) || (_USE_DEBUG_PRINTF_UART2_) || defined (_USE_DEBUG_PRINTF_ITM_) || defined (_USE_DEBUG_PRINTF_MEM_)

#include <stdio.h>

#pragma import(__use_no_semihosting_swi)

struct __FILE { int handle; };
FILE __stdout;
FILE __stdin;


#if defined (_USE_DEBUG_PRINTF_UART1_) 

 #include "MDR32F9Qx_uart.h"

 int fputc(int ch, FILE *f) 
 {
	while( UART_GetFlagStatus( MDR_UART1, UART_FLAG_TXFE ) != SET );

	UART_SendData( MDR_UART1, ch );

	return ch;
 }

#elif defined (_USE_DEBUG_PRINTF_UART2_) 

 #include "MDR32F9Qx_uart.h"

 int fputc(int ch, FILE *f) 
 {
	while( UART_GetFlagStatus( MDR_UART2, UART_FLAG_TXFE ) != SET );

	UART_SendData( MDR_UART2, ch );

	return ch;
 }

#elif defined (_USE_DEBUG_PRINTF_ITM_)

 #include <MDR32Fx.h>

 #define ITM_STIM_U32 (*(volatile unsigned int*)0xE0000000)    // Stimulus Port Register word acces
 #define ITM_STIM_U8  (*(volatile         char*)0xE0000000)    // Stimulus Port Register byte acces
 #define ITM_ENA      (*(volatile unsigned int*)0xE0000E00)    // Trace Enable Ports Register
 #define ITM_TCR      (*(volatile unsigned int*)0xE0000E80)    // Trace control register

 int fputc( int c, FILE *f ) 
 {
	// Check if ITM_TCR.ITMENA is set, Check if stimulus port is enabled
	//
	if( (ITM_TCR & 1) && (ITM_ENA & 1) )
	{
		// Wait until STIMx is ready,
		// then send data
		//
		while ((ITM_STIM_U8 & 1) == 0);

		ITM_STIM_U8 = (char)c;
	}
	return( c );
 }

#elif defined (_USE_DEBUG_PRINTF_MEM_) 

 #include <MDR32Fx.h>

 #define DEBUG_PRINTF_MEMORY_SIZE 4096
 char debug_printf_mem[ DEBUG_PRINTF_MEMORY_SIZE ];
 uint32_t debug_printf_mem_index = 0;

 int fputc(int ch, FILE *f) 
 {
	if( debug_printf_mem_index < DEBUG_PRINTF_MEMORY_SIZE-1 )
		debug_printf_mem[ debug_printf_mem_index++ ] = ch;

	return ch;
 }
#endif


int fgetc(FILE *f) {
  return EOF;
}


int ferror(FILE *f) {
  return EOF;
}


void _ttywrch(int ch) 
{
  
}


void _sys_exit(int return_code) {
  while (1);    /* endless loop */
}

#endif
