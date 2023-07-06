#ifndef __SYSTICK_H
#define __SYSTICK_H

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*SysTick_CALLBACK)(uint32_t);

void SysTick_Init(SysTick_CALLBACK CallbackFunction); // инициализация SysTick с периодом 1 мс
uint32_t Get_SysTick(void);	// возвращает значение SysTick
void Delay_MS(uint32_t ms);	// задержка в мс

#ifdef __cplusplus
}
#endif

#endif
