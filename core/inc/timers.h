#ifndef __TIMERS_H
#define __TIMERS_H

//Includes -------------------------------------------------------------------------//
#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

//Private defines -----------------------------------------------------------------//
#define MAX_xTIMERS  12

//Exported types -----------------------------------------------------------------//
typedef void * xTimerHandle;

typedef void (*tmrTIMER_CALLBACK)(xTimerHandle xTimer);	
	
typedef uint32_t (*portTickType)(void);

typedef enum 
{
	__IDLE = 0, 
	__ACTIVE, 
	__DONE
} tmrTIMER_STATE;

typedef struct
{
	uint32_t expiry;
  uint32_t periodic;
	tmrTIMER_STATE State;
	FunctionalState	AutoReload;
	tmrTIMER_CALLBACK	CallbackFunction;
} xTIMER;	

//Exported constants -------------------------------------------------------------//

//Exported macro -----------------------------------------------------------------//


//Private prototypes--------------------------------------------------------------//
void xTimer_Init(uint32_t (*GetSysTick)(void));
xTimerHandle xTimer_Create(uint32_t xTimerPeriodInTicks, FunctionalState AutoReload, tmrTIMER_CALLBACK CallbackFunction, FunctionalState NewState);
void xTimer_SetPeriod(xTimerHandle xTimer, uint32_t xTimerPeriodInTicks);
void xTimer_Reload(xTimerHandle xTimer);
void xTimer_Delete(xTimerHandle xTimer);
void xTimer_Task(uint32_t portTick);
void UART_TIMER_Callback(void);
void GPS_PPS_IRQ_Callback(void);
void GPS_PPS_DISABLE_IRQ_Callback(void);
void Reload_Timer_GPS_UART_Timeout(void);	
void Create_Timer_configure_GPS (void);
void timers_ini (void);
void timer_delay (uint16_t );

#ifdef __cplusplus
}
#endif

#endif
