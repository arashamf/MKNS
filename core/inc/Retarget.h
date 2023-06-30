#ifndef _DEBUG_PRINTF_H
#define _DEBUG_PRINTF_H

	#if defined (_USE_DEBUG_PRINTF_UART1_) || defined (_USE_DEBUG_PRINTF_UART2_) || defined (_USE_DEBUG_PRINTF_ITM_) || defined (_USE_DEBUG_PRINTF_MEM_)
		#include <stdio.h>
    	#define DEBUG_PRINTF(...) printf(__VA_ARGS__)
	#elif  defined (_USE_DEBUG_PRINTF_RTT_)
		#include "SEGGER_RTT.h"
    	#define DEBUG_PRINTF(...) SEGGER_RTT_printf(0,__VA_ARGS__)
	#else
    	#define DEBUG_PRINTF(...)
	#endif 

#endif
