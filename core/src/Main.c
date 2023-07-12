/* Includes ------------------------------------------------------------------*/

#include "main.h"
#include "can.h"
#include "uart_func.h"
#include "HW_Profile.h"
#include "MNP_msg.h"
#include "systick.h"
#include "timers.h"
#include "pins.h"
#include "typedef.h"
#include "protocol.h"
#include <MDR32Fx.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Variables -----------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/


/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
#ifdef __USE_DBG
 #define ITM_STIM_U32 (*(volatile unsigned int*)0xE0000000)    // Stimulus Port Register word acces
 #define ITM_STIM_U8  (*(volatile         char*)0xE0000000)    // Stimulus Port Register byte acces
 #define ITM_ENA      (*(volatile unsigned int*)0xE0000E00)    // Trace Enable Ports Register
 #define ITM_TCR      (*(volatile unsigned int*)0xE0000E80)    // Trace control register

int fputc( int c, FILE *f ) 
{
	if( (ITM_TCR & 1) && (ITM_ENA & 1) ) 	// Check if ITM_TCR.ITMENA is set, Check if stimulus port is enabled
	{ 		
		while ((ITM_STIM_U8 & 1) == 0) {}; //Wait until STIMx is ready,
		ITM_STIM_U8 = (char)c; // then send data
	}
	return( c );
}
#endif

//-----------------------------------------------------------------------------
int main( void )
//-----------------------------------------------------------------------------
{
	RST_CLK_PCLKcmd( RST_CLK_PCLK_BKP, ENABLE ); //включение тактирования регистра RTC

	SystemInit();
	
	CPUClk80MHz_Init();
	
	xTimer_Init(&Get_SysTick);
	SysTick_Init(&xTimer_Task);	
	
	Func_GPIO_Init();
	
	MNP_UART_Init (); //инициализация UART
	MKS_context_ini (); //инициализация структур 
	CAN1_Init((void*)&MKS2);
	timers_ini (); //инициализация таймеров
	GPS_Init(); //отправка конфигурационного сообщения приёмнику
	
	#ifdef __USE_IWDG
		IWDG_Init(); //инициализация сторожевого таймера
	#endif
	
while(1)
	{
		CAN1_RX_Process();
		
		if ( (MKS2.fContext.Fail & FAIL_MASK) != 0 ) 
			{SET_RED_LED();} 
		else 
		{	
			if ( MKS2.tmContext.Valid ) 
				{SET_GREEN_LED();} 
			else
				{SET_YELLOW_LED();}
		}
		
		#ifdef __USE_IWDG	
			IWDG_ReloadCounter();
		#endif

	}
}

//-----------------------------------------------------------------------------
static void CPUClk80MHz_Init(void)
{
	uint8_t n = 0;
	ErrorStatus ret;
	
	/* Enable HSE */
  RST_CLK_HSEconfig(RST_CLK_HSE_ON);
  while ( n < HSE_ON_ATTEMPTS )
  {
		ret = RST_CLK_HSEstatus();
		
		if ( ret == SUCCESS ) 
			{break;}	
		else
			{n++;}
  }
	
	if ( ret != SUCCESS ) 
		{SET_RED_LED();}

  /* CPU_C1_SEL = HSE */
  RST_CLK_CPU_PLLconfig(RST_CLK_CPU_PLLsrcHSEdiv1, RST_CLK_CPU_PLLmul10); //Select HSE clock as CPU_PLL input clock source & set PLL multiplier
 
		RST_CLK_CPU_PLLcmd(ENABLE); //enable CPU_PLL
  while (RST_CLK_CPU_PLLstatus() != SUCCESS) {}; //ожидание готовности CPU_PLL 

	RST_CLK_PCLKcmd(RST_CLK_PCLK_EEPROM, ENABLE); 	// Enables the RST_CLK_PCLK_EEPROM 

  EEPROM_SetLatency(EEPROM_Latency_3);   // Sets the code latency value 

  RST_CLK_CPUclkPrescaler(RST_CLK_CPUclkDIV1);   // CPU_C3_SEL = CPU_C2_SEL 

  RST_CLK_CPU_PLLuse(ENABLE);   // CPU_C2_SEL = PLL 

  RST_CLK_CPUclkSelection(RST_CLK_CPUclkCPU_C3);   // HCLK_SEL = CPU_C3_SEL 
}

//-----------------------------------------------------------------------------
bool ClockConfigure ( void )
{
  uint32_t cntr = 0;

	cntr = 0;
	RST_CLK_HSEconfig(RST_CLK_HSE_ON); //switch on HSE clock generator
  while(RST_CLK_HSEstatus() != SUCCESS && cntr++ < 0x40000) {};//ожидание готовности генератора HSE

  if(RST_CLK_HSEstatus() != SUCCESS) //получение статуса генератора HSE
		return false;
	
	#define PLL_MULL_VALUE (CPU_CLOCK_VALUE / HSE_Value - 1) //80МГц/8МГц=10
	RST_CLK_CPU_PLLconfig (RST_CLK_CPU_PLLsrcHSEdiv1, PLL_MULL_VALUE ); //Select HSE clock as CPU_PLL input clock source & set PLL multiplier

	RST_CLK_CPU_PLLcmd(ENABLE); //enable CPU_PLL
	
	while(RST_CLK_CPU_PLLstatus() != SUCCESS && cntr++ < 0x40000) {};//ожидание готовности CPU_PLL 

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
#ifdef __USE_IWDG
void InitWatchDog( void )
{
	RST_CLK_PCLKcmd(RST_CLK_PCLK_IWDG,ENABLE);
	IWDG_WriteAccessEnable();
	IWDG_SetPrescaler(IWDG_Prescaler_64);	// 40000/64=625 Гц
	while( IWDG_GetFlagStatus( IWDG_FLAG_PVU ) != 1 )
	{}
	IWDG_SetReload( 2500 );	// 2500 / 652 = 4 сек
	IWDG_Enable();
	IWDG_ReloadCounter();
}
#endif

/*void HardFault_Handler(void)
{
	NVIC_SystemReset();
}

void MemManage_Handler(void)
{
	NVIC_SystemReset();	
}*/

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

