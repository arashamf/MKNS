/* Includes ------------------------------------------------------------------*/
#include "MDR32F9Qx_config.h"
#include "MDR32F9Qx_bkp.h"
#include "MDR32F9Qx_rst_clk.h"
#include "MDR32F9Qx_eeprom.h"
#include "MDR32F9Qx_port.h"
#include "MDR32F9Qx_iwdg.h"
#include "MDR32F9Qx_can.h"

#include "Application.h"
#include "HW_Profile.h"
#include "XTick.h"
#include "BiLED.h"
#include "CAN.h"
#include "xpt.h"
#include "Retarget.h"
#include "BackplaneAddress.h"


#include <string.h>
#include <stdbool.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Variables -----------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

static const TBiLED m_Led = { { LED_GREEN_PORT, LED_GREEN_PIN }, { LED_RED_PORT, LED_RED_PIN } }; 

/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/


//-----------------------------------------------------------------------------
bool ClockConfigure ( void )
//-----------------------------------------------------------------------------
{
  uint32_t cntr;

	cntr = 0;
	RST_CLK_HSEconfig(RST_CLK_HSE_ON); //switch on HSE clock generator
  while(RST_CLK_HSEstatus() != SUCCESS && cntr++ < 0x40000) //ожидание готовности генератора HSE
		{};

  if(RST_CLK_HSEstatus() != SUCCESS) //получение статуса генератора HSE
		return false;
	
	#define PLL_MULL_VALUE (CPU_CLOCK_VALUE / HSE_Value - 1) //80МГц/8МГц=10
	RST_CLK_CPU_PLLconfig (RST_CLK_CPU_PLLsrcHSEdiv1, PLL_MULL_VALUE ); //Select HSE clock as CPU_PLL input clock source & set PLL multiplier

	cntr = 0;
	RST_CLK_CPU_PLLcmd(ENABLE); //enable CPU_PLL
	
	while(RST_CLK_CPU_PLLstatus() != SUCCESS && cntr++ < 0x40000) //ожидание готовности CPU_PLL 
		{};

  if(RST_CLK_CPU_PLLstatus() != SUCCESS) //получение статуса CPU_PLL 
		return false;

	RST_CLK_PCLKcmd(RST_CLK_PCLK_EEPROM, ENABLE); //enable the RST_CLK_PCLK_EEPROM 

	// Set the code latency value
	#if CPU_CLOCK_VALUE < 25000000UL					// Freqency < 25MHz
		#define EEPROM_LATENCY_VALUE EEPROM_Latency_0
	#elif CPU_CLOCK_VALUE < 50000000UL					// 25MHz <= Freqency < 50MHz
		#define EEPROM_LATENCY_VALUE EEPROM_Latency_1				
	#elif CPU_CLOCK_VALUE < 75000000UL					// 50MHz <= Freqency < 75MHz
		#define EEPROM_LATENCY_VALUE EEPROM_Latency_2				
	#elif CPU_CLOCK_VALUE < 100000000UL					// 75MHz <= Freqency < 100MHz
		#define EEPROM_LATENCY_VALUE EEPROM_Latency_3				
	#elif CPU_CLOCK_VALUE < 125000000UL					// 100MHz <= Freqency < 125MHz
		#define EEPROM_LATENCY_VALUE EEPROM_Latency_4				
	#elif CPU_CLOCK_VALUE < 150000000UL					// 125MHz <= Freqency < 150MHz
		#define EEPROM_LATENCY_VALUE EEPROM_Latency_5				
	#else												// 150MHz <= Freqency
		#define EEPROM_LATENCY_VALUE EEPROM_Latency_7
	#endif
	
	EEPROM_SetLatency(EEPROM_LATENCY_VALUE); //sets the code latency value

	RST_CLK_CPUclkPrescaler(RST_CLK_CPUclkDIV1); //set CPU_C3_prescaler to 1

	RST_CLK_CPU_PLLuse(ENABLE); 	//set CPU_C2_SEL to CPU_PLL output instead of CPU_C1 clock
	
	RST_CLK_CPUclkSelection(RST_CLK_CPUclkCPU_C3); //select CPU_C3 clock on the CPU clock MUX 

	SystemCoreClockUpdate(); //get core clock frequency  
	
	return true;
}

//-----------------------------------------------------------------------------------------------------//
void InitWatchDog( void )
{
	RST_CLK_PCLKcmd(RST_CLK_PCLK_IWDG,ENABLE);
	IWDG_WriteAccessEnable();
	IWDG_SetPrescaler(IWDG_Prescaler_64);	// 625 Гц
	while( IWDG_GetFlagStatus( IWDG_FLAG_PVU ) != 1 )
	{}
	IWDG_SetReload( 2500 );	// 2500 / 652 = 4 сек
	IWDG_Enable();
	IWDG_ReloadCounter();
}
 
//----------------------------------Управление индикаторами состояния----------------------------------//
void TaskSuperviseStatus( void )
{

}


//-----------------------------------------------------------------------------
int main( void )
//-----------------------------------------------------------------------------
{
	RST_CLK_PCLKcmd( RST_CLK_PCLK_BKP, ENABLE ); //включение тактирования регистра RTC

	SystemInit();

	InitBiLED( &m_Led );
	SetBiLED( &m_Led, LED_YELLOW );

	while( ClockConfigure() == false )
		{SetBiLED( &m_Led, LED_RED );}
	else
		{SetBiLED( &m_Led, LED_YELLOW );}

	InitXTick();
	DelayMs( 2000 );



	while( 1 )
	{
		SetBiLED( &m_Led, LED_RED );
		//TaskSuperviseStatus();		
		DelayMs( 1000 );
		SetBiLED( &m_Led, LED_GREEN );
		DelayMs( 1000 );
	//	IWDG_ReloadCounter();
	}
}



//-----------------------------------------------------------------------------
#if (USE_ASSERT_INFO == 1)
void assert_failed(uint32_t file_id, uint32_t line)
{
	while (1)
	{
	}
}
#elif (USE_ASSERT_INFO == 2)
void assert_failed(uint32_t file_id, uint32_t line, const uint8_t* expr)
{
	while (1)
	{
	}
}
#endif /* USE_ASSERT_INFO */

