#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <stdbool.h>

#include "MDR32Fx.h"
#include "MDR32F9Qx_port.h"
#include "MDR32F9Qx_rst_clk.h"
#include "MDR32F9Qx_eeprom.h"
#include "MDR32F9Qx_adc.h"
#include "MDR32F9Qx_can.h"
//#include "MDR32F9Qx_can_helper.h"
#include "MDR32F9Qx_uart.h"
#include "MDR32F9Qx_timer.h"
#include "MDR32F9Qx_iwdg.h"
#include "MDR32F9Qx_rst_clk.h"
#include "MDR32F9Qx_eeprom.h"
#include "MDR32F9Qx_port.h"
#include "MDR32F9Qx_iwdg.h"
#include "MDR32F9Qx_config.h"
#include "MDR32F9Qx_bkp.h"

//#include "pins.h"
//#include "HW_Profile.h"
#include "MNP_msg.h"
/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/
extern MNP_MSG_t MNP_PUT_MSG;

/* Exported macro ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/
static void CPUClk80MHz_Init(void);
static void gpio_init(void);
bool ClockConfigure (void);
void InitWatchDog(void);
void TaskSuperviseStatus(void);

/* Private defines -----------------------------------------------------------*/
#define HSE_ON_ATTEMPTS				10


#endif 
