
#ifndef __PINS_H__
#define __PINS_H__

#ifdef __cplusplus
extern "C" {
#endif

// Includes ------------------------------------------------------------------------//
#include "main.h"

// Private defines------------------------------------------------------------------//

#define GPS_PPS_ENABLE(a) 	PORT_SetBits(MDR_PORTB, PORT_Pin_8) 	//включение сигнала секундной метки
#define GPS_PPS_DISABLE(a) 	PORT_ResetBits(MDR_PORTB, PORT_Pin_8) //выключение сигнала секундной метки

#define ON 1
#define OFF 0

//Exported types -----------------------------------------------------------------//
typedef struct 
{
	MDR_PORT_TypeDef    *PORTx;
	uint32_t Pin;		
} TPortPin;

//----------------------------------------------------------------------------
typedef enum { LED_BLACK = 0, LED_RED, LED_GREEN, LED_YELLOW }TBiLEDColor;

//----------------------------------------------------------------------------
typedef struct 
{
	MDR_PORT_TypeDef *PORTx;
	uint16_t PORT_Pin;		

}TLED_PortPin;

//----------------------------------------------------------------------------
typedef struct 
{
	TLED_PortPin	Green;		
	TLED_PortPin 	Red;

}TBiLED;

//Prototypes------------------------------------------------------------------------//
void Pins_Address_Init(void);
uint8_t Get_Module_Address( void );
void Task_Control_LEDs( void );
void InitBiLED( const TBiLED *pBiLed );
void SetBiLED( const TBiLED *pBiLed, TBiLEDColor Color ); 
void GPS_nRST_Init(void);
void GPS_Reset (FunctionalState NewState);

#ifdef __cplusplus
}
#endif
#endif /*__ PINS_H__ */

