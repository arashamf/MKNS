#ifndef _HW_Profile_H_
#define _HW_Profile_H_

/* Includes ------------------------------------------------------------------*/
#include "MDR32F9Qx_config.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define CPU_CLOCK_VALUE	(80000000UL)	/* ������� ����������� */
#define TICKS_PER_SECOND			1000 
//#define TICKS_PER_SECOND  (TICKS_PER_MILLISECOND*1000)	

//-----------------------------------------------------
#define LED_GREEN_PIN						PORT_Pin_2
#define LED_GREEN_PORT					MDR_PORTF

#define LED_RED_PIN							PORT_Pin_3
#define LED_RED_PORT						MDR_PORTF

//CAN--------------------------------------------------
#define MY_MDR_CAN							MDR_CAN1

#define MY_CAN_RX_PIN						PORT_Pin_7
#define MY_CAN_RX_PORT					MDR_PORTA
#define MY_CAN_RX_PORT_FUNC			PORT_FUNC_ALTER

#define MY_CAN_TX_PIN						PORT_Pin_6
#define MY_CAN_TX_PORT					MDR_PORTA
#define MY_CAN_TX_PORT_FUNC			PORT_FUNC_ALTER

//����� � gps-�������----------------------------------
#define UART_RX									MDR_UART2
#define UART_RX_IRQ							UART2_IRQn
#define UART_RX_CLOCK 					RST_CLK_PCLK_UART2		
#define UART_RX_IRQHandler			UART2_IRQHandler
#define UART_TIMER_IRQHandler		Timer2_IRQHandler

#define UART_TX									MDR_UART1
#define UART_TX_CLOCK 					RST_CLK_PCLK_UART1
	
#define UARTx_BAUD_RATE					115200
	
#define UART_CLOCK_Pin_TX 			RST_CLK_PCLK_PORTB
#define UART_CLOCK_Pin_RX 			RST_CLK_PCLK_PORTB
	
#define UART_PORT_TX						MDR_PORTB
#define UART_PORT_PinTX					PORT_Pin_0
#define UART_PORT_FuncTX  			PORT_FUNC_OVERRID
	
#define UART_PORT_RX						MDR_PORTB	
#define UART_PORT_PinRX					PORT_Pin_1
#define UART_PORT_FuncRX  			PORT_FUNC_OVERRID	

//UART ��� �������--------------------------------------

#define DBG_TX									MDR_UART2
#define DBG_TX_CLOCK 						RST_CLK_PCLK_UART2
	
#define DBG_BAUD_RATE						115200
	
#define DBG_CLOCK_Pin_TX 				RST_CLK_PCLK_PORTF
	
#define DBG_PORT_TX							MDR_PORTF
#define DBG_PORT_PinTX					PORT_Pin_1
#define DBG_PORT_FuncTX  				PORT_FUNC_OVERRID	

// ����� ������ � ������----------------------------------
#define BACKPLANE_PIN_CLOCK 		RST_CLK_PCLK_PORTA

#define BACKPLANE_ADDR0_PIN			PORT_Pin_4
#define BACKPLANE_ADDR0_PORT		MDR_PORTA
                              
#define BACKPLANE_ADDR1_PIN			PORT_Pin_2        
#define BACKPLANE_ADDR1_PORT		MDR_PORTA             
                              
#define BACKPLANE_ADDR2_PIN			PORT_Pin_3        
#define BACKPLANE_ADDR2_PORT		MDR_PORTA             
                             
#define BACKPLANE_ADDR3_PIN			PORT_Pin_0        
#define BACKPLANE_ADDR3_PORT		MDR_PORTA             
                                        
#define BACKPLANE_ADDR4_PIN			PORT_Pin_1        
#define BACKPLANE_ADDR4_PORT		MDR_PORTA             

// ��� ������������ GPS-��������---------------------------
#define GPS_CLOCK_nRST 					RST_CLK_PCLK_PORTC
#define GPS_PORT_nRST 					MDR_PORTC
#define GPS_PIN_nRST 						PORT_Pin_1

// ��� ������ PPS �������---------------------------
#define CLOCK_PPS_PULSE_PIN 		RST_CLK_PCLK_PORTA
#define PPS_PULSE_PIN 					PORT_Pin_5
#define PPS_PULSE_PORT					MDR_PORTA

// GPS Ant Power------------------------------------------
#define GPS_CLOCK_ANT_PW				RST_CLK_PCLK_PORTD
#define GPS_PORT_ANT_PW 				MDR_PORTD
#define GPS_PIN_ANT_PW 					PORT_Pin_3

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
//-------------------------------------------------------------------------------------------------
	
#endif
