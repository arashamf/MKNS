// Includes ------------------------------------------------------------------
#include "main.h"
#include "adc.h"
#include "pins.h"
#include "typedef.h"
#include "HW_Profile.h"

//Macro -----------------------------------------------------------------------------

// объ€вление ф-ий ------------------------------------------------------------------


// переменные------------------------------------------------------------------



//---------------------------------------инициализаци€ ADC---------------------------------------//
void init_ADC (void )
{
	PORT_InitTypeDef PORT_InitStructure;
	ADC_InitTypeDef sADC;
	ADCx_InitTypeDef sADCx;

	RST_CLK_PCLKcmd(RST_CLK_PCLK_PORTD, ENABLE);

	PORT_StructInit(&PORT_InitStructure);
	PORT_InitStructure.PORT_Pin	= PORT_Pin_2;
	PORT_InitStructure.PORT_OE = PORT_OE_IN;
	PORT_InitStructure.PORT_MODE = PORT_MODE_ANALOG;

	PORT_Init(MDR_PORTD, &PORT_InitStructure);

	ADC_DeInit();
	
	// Inti clock ADC
	RST_CLK_ADCclkSelection(RST_CLK_ADCclkCPU_C1);
	RST_CLK_ADCclkPrescaler(RST_CLK_ADCclkDIV8);
	RST_CLK_ADCclkEnable(ENABLE); // Enable clock ADC
	RST_CLK_PCLKcmd(RST_CLK_PCLK_ADC, ENABLE);
	
	ADC_StructInit(&sADC);	
	sADC.ADC_StartDelay = 5;
	sADC.ADC_IntVRefConversion = ADC_VREF_CONVERSION_Disable;
	sADC.ADC_IntVRefTrimming = 0;
	ADC_Init(&sADC);

	// ADC1 Configuration 
	ADCx_StructInit(&sADCx);
	sADCx.ADC_ClockSource = ADC_CLOCK_SOURCE_CPU;
	sADCx.ADC_SamplingMode = ADC_SAMPLING_MODE_CICLIC_CONV;
	sADCx.ADC_ChannelSwitching = ADC_CH_SWITCHING_Disable;
	sADCx.ADC_ChannelNumber = ADC_CH_ADC2;
	sADCx.ADC_Channels = 0;
	sADCx.ADC_LevelControl = ADC_LEVEL_CONTROL_Disable;
	sADCx.ADC_LowLevel = 0;
	sADCx.ADC_HighLevel = 0;
	sADCx.ADC_VRefSource = ADC_VREF_SOURCE_INTERNAL;
	sADCx.ADC_IntVRefSource = ADC_INT_VREF_SOURCE_INEXACT;
	sADCx.ADC_Prescaler = ADC_CLK_div_2;
	sADCx.ADC_DelayGo = 7;
	ADC1_Init(&sADCx);

	ADC1_Cmd(ENABLE); // ADC1 enable 

	ADC1_Start();
}

//---------------------------------------------------------------------------------------------------//






