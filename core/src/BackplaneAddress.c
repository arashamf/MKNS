#include "main.h"

#include "HW_Profile.h"

//-----------------------------------------------------------------------------
// Возвращает адрес модуля в кроссе
//
uint32_t GetBackplaneAddress( void )
//-----------------------------------------------------------------------------
{

  typedef struct 
  {
	MDR_PORT_TypeDef *PORTx;
	uint16_t PORT_Pin;		

  }TPortPin;

  const TPortPin pins[] = {
							{ MY_BACKPLANE_ADDR0_PORT, MY_BACKPLANE_ADDR0_PIN },
							{ MY_BACKPLANE_ADDR1_PORT, MY_BACKPLANE_ADDR1_PIN },
              { MY_BACKPLANE_ADDR2_PORT, MY_BACKPLANE_ADDR2_PIN },
							{ MY_BACKPLANE_ADDR3_PORT, MY_BACKPLANE_ADDR3_PIN },
							{ MY_BACKPLANE_ADDR4_PORT, MY_BACKPLANE_ADDR4_PIN }
						  };
  PORT_InitTypeDef sPort;
  int i;
  uint32_t result = 0;

	PORT_StructInit( &sPort );

	sPort.PORT_OE    = PORT_OE_IN;
	sPort.PORT_FUNC  = PORT_FUNC_PORT;
	sPort.PORT_MODE  = PORT_MODE_DIGITAL;
	sPort.PORT_SPEED = PORT_SPEED_SLOW;

	for( i = 0 ; i < sizeof( pins )/sizeof( pins[ 0 ] ) ; i++ )
	{
		// Enable peripheral clock for Port
		//
		RST_CLK_PCLKcmd( PCLK_BIT( pins[ i ].PORTx ), ENABLE );

		// Configure Pin
		//
		sPort.PORT_Pin   = pins[ i ].PORT_Pin;
		PORT_Init( pins[ i ].PORTx, &sPort );
	}

	// Short delay
	//
	for( i = 0 ; i < (CPU_CLOCK_VALUE / 1000UL) ; i++ )
		__NOP();


	for( i = 0 ; i < sizeof( pins )/sizeof( pins[ 0 ] ) ; i++ )
	{
		result |= ( (!PORT_ReadInputDataBit( pins[ i ].PORTx, pins[ i ].PORT_Pin )) << i ); 
	}

	for( i = 0 ; i < sizeof( pins )/sizeof( pins[ 0 ] ) ; i++ )
	{
		// Disable peripheral clock for Port
		//
		RST_CLK_PCLKcmd( PCLK_BIT( pins[ i ].PORTx ), DISABLE );
	}

	return result;
}

