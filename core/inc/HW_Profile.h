#ifndef _HW_Profile_H_
#define _HW_Profile_H_

/* Includes ------------------------------------------------------------------*/
#include "MDR32F9Qx_config.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define CPU_CLOCK_VALUE	(80000000UL)	/* Частота контроллера */

//----------------------------------
#define LED_GREEN_PIN				PORT_Pin_2
#define LED_GREEN_PORT			MDR_PORTF

#define LED_RED_PIN					PORT_Pin_3
#define LED_RED_PORT				MDR_PORTF

//----------------------------------
#define MY_MDR_CAN							MDR_CAN1

#define MY_CAN_RX_PIN						PORT_Pin_7
#define MY_CAN_RX_PORT					MDR_PORTA
#define MY_CAN_RX_PORT_FUNC			PORT_FUNC_ALTER

#define MY_CAN_TX_PIN						PORT_Pin_6
#define MY_CAN_TX_PORT					MDR_PORTA
#define MY_CAN_TX_PORT_FUNC			PORT_FUNC_ALTER

//Связь с gps-модулем----------------------------------
#define UPS_UART					UART1

#define UPS_UART_TX_PORT					MDR_PORTB
#define UPS_UART_TX_PIN						PORT_Pin_0
#define UPS_UART_TX_PIN_FUNCTION	PORT_FUNC_OVERRID

#define UPS_UART_RX_PORT					MDR_PORTB
#define UPS_UART_RX_PIN						PORT_Pin_6			
#define UPS_UART_RX_PIN_FUNCTION	PORT_FUNC_ALTER

// Адрес модуля в кроссе----------------------------------
#define MY_BACKPLANE_ADDR0_PIN		PORT_Pin_0
#define MY_BACKPLANE_ADDR0_PORT		MDR_PORTE
                              
#define MY_BACKPLANE_ADDR1_PIN		PORT_Pin_1        
#define MY_BACKPLANE_ADDR1_PORT		MDR_PORTE             
                              
#define MY_BACKPLANE_ADDR2_PIN		PORT_Pin_2        
#define MY_BACKPLANE_ADDR2_PORT		MDR_PORTE             
                             
#define MY_BACKPLANE_ADDR3_PIN		PORT_Pin_3        
#define MY_BACKPLANE_ADDR3_PORT		MDR_PORTE             
                                        
#define MY_BACKPLANE_ADDR4_PIN		PORT_Pin_6        
#define MY_BACKPLANE_ADDR4_PORT		MDR_PORTE             

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
//-------------------------------------------------------------------------------------------------
	
#endif
