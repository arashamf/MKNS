
#ifndef __PINS_H__
#define __PINS_H__

#ifdef __cplusplus
extern "C" {
#endif

// Includes ------------------------------------------------------------------------//
#include "MDR32F9Qx_config.h"

// Private defines------------------------------------------------------------------//

#define ON 1
#define OFF 0


#define LED_RED(x) ((x)? (LL_GPIO_SetOutputPin (LED_GPIO_Port, LED_Pin)) : (LL_GPIO_ResetOutputPin(LED_GPIO_Port, LED_Pin)))
#define TOOGLE_LED_RED() (LED_RED (!(LL_GPIO_IsOutputPinSet(LED_GPIO_Port, LED_Pin))))

//Exported types -----------------------------------------------------------------//
typedef struct 
{
	GPIO_TypeDef *PORTx;
	uint32_t Pin;		
} TPortPin;

//Prototypes------------------------------------------------------------------------//
void Pins_LEDs_Init(void);
void Pins_Address_Init(void);
uint8_t Get_Module_Address( void );
void Task_Control_LEDs( void );

#ifdef __cplusplus
}
#endif
#endif /*__ PINS_H__ */

