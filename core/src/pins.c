
// Includes---------------------------------------------------------------------------------------------//
#include "main.h"
#include "pins.h"
#include "HW_Profile.h"
//-----------------------------------------------------------------------------------------------------//
void Pins_Address_Init(void)
{

}

//-----------------------------------------------------------------------------------------------------//
uint8_t Get_Module_Address( void )
{
 // uint8_t count;
 // uint32_t result = 0;
	uint8_t addr = 0x3;
	
/*	Pins_Address_Init();
	
  const TPortPin pins[] = 
						{
							{ MY_BACKPLANE_ADDR0_PORT, MY_BACKPLANE_ADDR0_PIN },
							{ MY_BACKPLANE_ADDR1_PORT, MY_BACKPLANE_ADDR1_PIN },
              { MY_BACKPLANE_ADDR2_PORT, MY_BACKPLANE_ADDR2_PIN },
							{ MY_BACKPLANE_ADDR3_PORT, MY_BACKPLANE_ADDR3_PIN },
							{ MY_BACKPLANE_ADDR4_PORT, MY_BACKPLANE_ADDR4_PIN }
						};

	for( count = 0 ; count < 5; count++ ) //cчитывание состояния пинов на кросс плате
	{
		addr |= (!(LL_GPIO_IsInputPinSet (pins[count].PORTx, pins[count].Pin)) << count); //инверсия
	}*/

	return addr;
}

//-----------------------------------------------------------------------------------------------------//
void Task_Control_LEDs( void )
{
/*	if( g_MyFlags.CAN_Fail == 1 ) //если на CAN шине ошибка
	{
		LED_GREEN(OFF); //красный включён, зелёный выключен
		LED_RED(ON); 
	}
	else 
	{
		LED_RED(OFF);
		LED_GREEN(ON);
	}*/
}
//-----------------------------------------------------------------------------------------------------//
void SetBiLED( const TBiLED *pBiLed, TBiLEDColor Color )
{
	switch( Color )
	{
		case LED_BLACK:
			PORT_ResetBits( pBiLed->Red.PORTx, pBiLed->Red.PORT_Pin );
			PORT_ResetBits( pBiLed->Green.PORTx, pBiLed->Green.PORT_Pin );
		break;

		case LED_RED:
			PORT_SetBits( pBiLed->Red.PORTx, pBiLed->Red.PORT_Pin );
			PORT_ResetBits( pBiLed->Green.PORTx, pBiLed->Green.PORT_Pin );
		break;

		case LED_GREEN:
			PORT_ResetBits( pBiLed->Red.PORTx, pBiLed->Red.PORT_Pin );
			PORT_SetBits( pBiLed->Green.PORTx, pBiLed->Green.PORT_Pin );
		break;

		case LED_YELLOW:
			PORT_SetBits( pBiLed->Red.PORTx, pBiLed->Red.PORT_Pin );
			PORT_SetBits( pBiLed->Green.PORTx, pBiLed->Green.PORT_Pin );
		break;
	}
}

//----------------------------------------------------------------------------
static void InitLED_Pin( const TLED_PortPin *PortPin )  
{
  PORT_InitTypeDef port_init_struct;

	RST_CLK_PCLKcmd( PCLK_BIT(PortPin->PORTx), ENABLE ); // Enable peripheral clocks for PORT

	PORT_StructInit( &port_init_struct ); // Configure pin

	port_init_struct.PORT_Pin   = PortPin->PORT_Pin;
	port_init_struct.PORT_OE    = PORT_OE_OUT; //пин на выход
	port_init_struct.PORT_FUNC  = PORT_FUNC_PORT; //функция вывода
	port_init_struct.PORT_MODE  = PORT_MODE_DIGITAL; //цифровой режим пина
	
	port_init_struct.PORT_SPEED = PORT_SPEED_SLOW;

	PORT_Init( PortPin->PORTx, &port_init_struct );
}

//----------------------------------------------------------------------------
void InitBiLED( const TBiLED *pBiLed )  
{
	InitLED_Pin( &pBiLed->Red );
	InitLED_Pin( &pBiLed->Green );
}

//----------------------------------------------------------------------------
void GPS_nRST_Init(void)
{
	PORT_InitTypeDef PORT_InitStructure;

	RST_CLK_PCLKcmd(GPS_CLOCK_nRST, ENABLE);
	PORT_StructInit(&PORT_InitStructure);

	PORT_InitStructure.PORT_Pin		= GPS_PIN_nRST; 
	PORT_InitStructure.PORT_OE 		= PORT_OE_OUT;
	PORT_InitStructure.PORT_FUNC 	= PORT_FUNC_PORT;
	PORT_InitStructure.PORT_MODE 	= PORT_MODE_DIGITAL;
	PORT_InitStructure.PORT_SPEED = PORT_SPEED_MAXFAST;
	PORT_InitStructure.PORT_PULL_DOWN = PORT_PULL_DOWN_ON;

	PORT_Init(GPS_PORT_nRST, &PORT_InitStructure);

	GPS_Reset(DISABLE);
}

//----------------------------------аппаратная перезагрузка приемника----------------------------------//
void GPS_Reset(FunctionalState NewState)
{
	if (NewState == ENABLE) 
		{PORT_ResetBits(GPS_PORT_nRST, GPS_PIN_nRST);} 
	else 
		{PORT_SetBits(GPS_PORT_nRST, GPS_PIN_nRST);}
}

