#include "systick.h"
#include "HW_Profile.h"

//переменные---------------------------------------------------------------------
static uint32_t TicksCounter = 0;
static SysTick_CALLBACK SysTick_CallbackFunction = NULL;

//инициализация SysTick с периодом 1 мс------------------------------------------ 
void SysTick_Init(SysTick_CALLBACK CallbackFunction)
{
	SysTick->CTRL = 0;
	SysTick->LOAD = (uint32_t)(CPU_CLOCK_VALUE / TICKS_PER_SECOND) - 1;
	SysTick->CTRL = SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk | \
                  SysTick_CTRL_CLKSOURCE_Msk;
	
	SysTick_CallbackFunction = CallbackFunction;
}

// увеличивает значение SysTick при срабатывании прерывания----------------------
void SysTick_Handler(void)
{
    TicksCounter++;
		if ( SysTick_CallbackFunction != NULL ) 
			{SysTick_CallbackFunction(TicksCounter);}
}

// возвращает значение SysTick----------------------------------------------------
uint32_t Get_SysTick(void)
{
	return TicksCounter;
}
 
// задержка в мс------------------------------------------------------------------
void Delay_MS(uint32_t ms)
{	
  uint32_t ticks;

	ticks = Get_SysTick();
	while( ( Get_SysTick() - ticks ) < ms )
	{
		__NOP();
	}
}
