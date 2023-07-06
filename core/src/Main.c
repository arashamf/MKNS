/* Includes ------------------------------------------------------------------*/

#include "main.h"
#include "uart_func.h"
#include "HW_Profile.h"
#include "MNP_msg.h"
#include "systick.h"
#include "timers.h"
#include "pins.h"
#include "typedef.h"
#include "protocol.h"
//#include "BackplaneAddress.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Variables -----------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
TBiLED m_Led = {{LED_GREEN_PORT, LED_GREEN_PIN}, {LED_RED_PORT, LED_RED_PIN}}; 

uint8_t uart_buffer[BUFFER_SIZE]; //массив дл€ кольцевого буффера

MNP_MSG_t MNP_PUT_MSG; //иницализаци€ шаблона сообщени€ дл€ отправки приЄмнику

/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
 #include <MDR32Fx.h>

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

//-----------------------------------------------------------------------------
int main( void )
//-----------------------------------------------------------------------------
{
	RST_CLK_PCLKcmd( RST_CLK_PCLK_BKP, ENABLE ); //включение тактировани€ регистра RTC

	SystemInit();
	
	InitBiLED(&m_Led); //инициализаци€ светодиодов
	SetBiLED(&m_Led, LED_BLACK );
	
	/*if (ClockConfigure() == false )
		{SetBiLED( &m_Led, LED_RED );}
	else
		{SetBiLED( &m_Led, LED_YELLOW );}*/
	CPUClk80MHz_Init();
	
	xTimer_Init(&Get_SysTick);
	SysTick_Init(&xTimer_Task);	
	
	GPS_nRST_Init(); //инициализи€ пина аппаратной перезагрузки приЄмника
	
	UART_LoLevel_Init(UART_TX, UART_TX_CLOCK, UARTx_BAUD_RATE, UART_TX_MODE);  //инициализаци€ UART1 дл€ передачи
	UARTSetBaud(UART_TX, UARTx_BAUD_RATE, CPU_CLOCK_VALUE);				
	
	UART_LoLevel_Init(UART_RX, UART_RX_CLOCK, UARTx_BAUD_RATE, UART_RX_MODE); //инициализаци€ UART2 дл€ получени€
	UARTSetBaud(UART_RX, UARTx_BAUD_RATE, CPU_CLOCK_VALUE);
	
	RING_Init (&RING_buffer, uart_buffer, sizeof (uart_buffer));	

	MKS_context_ini ();
	timers_ini ();
	GPS_Init(&MNP_PUT_MSG); //отправка конфигурационного сообщени€ приЄмнику
	Set_GNSS_interval (&MNP_PUT_MSG, 2000); //2000=1c	
	
	SetBiLED(&m_Led, LED_GREEN);
	
	Delay_MS(100);



	while(1)
	{
		SetBiLED(&m_Led, LED_RED);
		//TaskSuperviseStatus();
		Delay_MS( 1000 );

		//put_msg2000 (&MNP_PUT_MSG);
		SetBiLED(&m_Led, LED_GREEN);
		Delay_MS	(1000);
	//	IWDG_ReloadCounter();
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
		{SetBiLED(&m_Led, LED_RED);}

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

//----------------------------------------------------------------------
/*static void gpio_init(void)
{
	PORT_InitTypeDef PORT_InitStructure;

	RST_CLK_PCLKcmd(RST_CLK_PCLK_PORTF, ENABLE);
	PORT_StructInit(&PORT_InitStructure);

	PORT_InitStructure.PORT_Pin	= (PORT_Pin_0 | PORT_Pin_1); 
	PORT_InitStructure.PORT_OE = PORT_OE_OUT;
	PORT_InitStructure.PORT_FUNC = PORT_FUNC_PORT;
	PORT_InitStructure.PORT_MODE = PORT_MODE_DIGITAL;
	PORT_InitStructure.PORT_SPEED = PORT_SPEED_MAXFAST;

	PORT_Init(MDR_PORTF, &PORT_InitStructure);

	PORT_ResetBits(MDR_PORTE, (PORT_Pin_3 | PORT_Pin_6));
}*/

//-----------------------------------------------------------------------------
bool ClockConfigure ( void )
{
  uint32_t cntr = 0;

	cntr = 0;
	RST_CLK_HSEconfig(RST_CLK_HSE_ON); //switch on HSE clock generator
  while(RST_CLK_HSEstatus() != SUCCESS && cntr++ < 0x40000) {};//ожидание готовности генератора HSE

  if(RST_CLK_HSEstatus() != SUCCESS) //получение статуса генератора HSE
		return false;
	
	#define PLL_MULL_VALUE (CPU_CLOCK_VALUE / HSE_Value - 1) //80ћ√ц/8ћ√ц=10
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
void InitWatchDog( void )
{
	RST_CLK_PCLKcmd(RST_CLK_PCLK_IWDG,ENABLE);
	IWDG_WriteAccessEnable();
	IWDG_SetPrescaler(IWDG_Prescaler_64);	// 625 √ц
	while( IWDG_GetFlagStatus( IWDG_FLAG_PVU ) != 1 )
	{}
	IWDG_SetReload( 2500 );	// 2500 / 652 = 4 сек
	IWDG_Enable();
	IWDG_ReloadCounter();
}
 
//----------------------------------”правление индикаторами состо€ни€----------------------------------//
/*void TaskSuperviseStatus( void )
{

}

void HardFault_Handler(void)
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

