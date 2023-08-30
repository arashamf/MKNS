
#ifndef __PINS_H__
#define __PINS_H__

#ifdef __cplusplus
extern "C" {
#endif

// Includes ------------------------------------------------------------------------//
#include "main.h"
#include "HW_Profile.h"

// Private defines------------------------------------------------------------------//

#define GPS_PPS_ENABLE() 	PORT_SetBits(PPS_PULSE_PORT, PPS_PULSE_PIN) 	//включение сигнала секундной метки
#define GPS_PPS_DISABLE() 	PPS_PULSE_PORT->RXTX &= ~PPS_PULSE_PIN 			//выключение сигнала секундной метки
//#define GPS_PPS_DISABLE() 	PORT_ResetBits(PPS_PULSE_PORT, PPS_PULSE_PIN)

#define ON 1
#define OFF 0

//Exported types -----------------------------------------------------------------//
typedef enum { LED_BLACK = 0, LED_RED, LED_GREEN, LED_YELLOW }TBiLEDColor;

//----------------------------------------------------------------------------
typedef struct 
{
	MDR_PORT_TypeDef *PORTx;
	uint16_t PORT_Pin;		

}TPortPin;

//----------------------------------------------------------------------------
typedef struct 
{
	TPortPin	Green;		
	TPortPin 	Red;

}TBiLED;

static TBiLED m_Led = {{LED_GREEN_PORT, LED_GREEN_PIN}, {LED_RED_PORT, LED_RED_PIN}}; 

//Prototypes------------------------------------------------------------------------//
int8_t Get_Module_Address( void );
void SetBiLED( const TBiLED *pBiLed, TBiLEDColor Color ); 
void GPS_Reset (FunctionalState NewState);
void Func_GPIO_Init(void);

//Macro---------------------------------------------------------------------------//
#define SET_BLACK_LED() 	SetBiLED(&m_Led, LED_BLACK);
#define SET_RED_LED() 		SetBiLED(&m_Led, LED_RED);
#define SET_GREEN_LED() 	SetBiLED(&m_Led, LED_GREEN);
#define SET_YELLOW_LED() 	SetBiLED(&m_Led, LED_YELLOW);

#ifdef __cplusplus
}
#endif
#endif /*__ PINS_H__ */

