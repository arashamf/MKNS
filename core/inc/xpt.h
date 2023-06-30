/*
 * ВТр:
 * Добавлены XPT_DELAY( xpt, delay ) и XPT_WAIT_FOR( xpt, condition, timeout )
 * Для их работы в системе должна присутствовать функция unsigned long GetTickCount( void ),
 * возвращающая время работы системы в миллисекундах
 * 
 * ВТр 13.07.2021:
 * Переработаны XPT_DELAY( xpt, delay ) и XPT_WAIT_FOR( xpt, condition, timeout )
 * Теперь они выглядят так:
 *   XPT_DELAY( xpt, gettick_funptr, delay ) и XPT_WAIT_FOR( xpt, condition, gettick_funptr, timeout ).
 * При вызове в параметре gettick_funptr надо передавать адрес функции, считающей тики.
 * Аргументы delay и timeout надо передавать в тех единицах, в которых gettick_funptr возвращает значение.
 * Так для GetTickCount() надо задавть в миллисекундах, а для GetXTick() в попугаях.
 * Например:
 *   XPT_DELAY( xpt, &GetTickCount, 1000 ); // пауза 1000 мс
 *   XPT_DELAY( xpt, &GetXTick, 20 * TICKS_PER_MILLISECOND ); // пауза 20 мс
 */

#ifndef __VTR_PT_H__
#define __VTR_PT_H__

#include "pt.h"

struct xpt {
  struct pt pt;
  unsigned long ticks;
  unsigned long flags;
};

#define XPT_WAIT_TIMEOUT 0x00000001

//----------------------------------------------------
// Пауза на "delay" у.е.
//
#define XPT_DELAY( xpt, gettick_funcptr, delay )	\
	do {											\
		(xpt)->ticks = ((*gettick_funcptr)());		\
	    LC_SET((xpt->pt).lc);						\
    	if( ((*gettick_funcptr)()) - (xpt)->ticks < delay )	\
			return PT_WAITING;						\
	} while(0)

//----------------------------------------------------
// Ожидание выполнения условия "condition" с таймаутом 
// "timeout" в у.е.
//
#define XPT_WAIT_FOR( xpt, condition, gettick_funcptr, timeout ) \
	do {											\
		(xpt)->ticks = ((*gettick_funcptr)());		\
		(xpt)->flags &= ~XPT_WAIT_TIMEOUT;			\
	    LC_SET((xpt->pt).lc);						\
		if( condition )								\
			break;									\
		if( ((*gettick_funcptr)()) - (xpt)->ticks >= timeout)\
        { 											\
			(xpt)->flags |= XPT_WAIT_TIMEOUT;		\
			break; 									\
		}											\
		return PT_WAITING;							\
 	 } while(0)


//----------------------------------------------------
// Обертки макросов из pt.h для работы со структурой xpt
//
#define XPT_INIT(xpt)   				PT_INIT(&(xpt)->pt)
#define XPT_THREAD(name_args) 			PT_THREAD(name_args)
#define XPT_BEGIN(xpt) 					PT_BEGIN(&(xpt)->pt)
#define XPT_END(xpt)					PT_END(&(xpt)->pt)
#define XPT_WAIT_UNTIL(xpt, condition)	PT_WAIT_UNTIL(&(xpt)->pt, condition) 
#define XPT_WAIT_WHILE(xpt, cond)  		PT_WAIT_WHILE(&(xpt)->pt, cond)
#define XPT_WAIT_THREAD(xpt, thread)	PT_WAIT_THREAD(&(xpt)->pt, thread)
#define XPT_SPAWN(xpt, child, thread)	PT_SPAWN(&(xpt)->pt, child, thread)	
#define XPT_RESTART(xpt)				PT_RESTART(&(xpt)->pt)
#define XPT_EXIT(xpt)					PT_EXIT(&(xpt)->pt)
#define XPT_SCHEDULE(f)					PT_SCHEDULE(f)
#define XPT_YIELD(xpt)					PT_YIELD(&(xpt)->pt)
#define XPT_YIELD_UNTIL(xpt, cond)		PT_YIELD_UNTIL(&(xpt)->pt, cond)

#endif /* __VTR_PT_H__ */

