
// Includes---------------------------------------------------------------------------------------//
#include "main.h"
#include "pins.h"
#include "HW_Profile.h"
#include "systick.h"

static void Pins_Address_Init(const TPortPin *, uint8_t );
static void InitLED_Pin( const TPortPin *PortPin );
static void InitBiLED( const TBiLED *pBiLed );
static void GPS_nRST_Init(void);
static void PPS_Pin_Init(void);

//--------------------инициализация пинов получения адреса модуля в кроссплате--------------------//
static void Pins_Address_Init(const TPortPin * pins, uint8_t number_pins)
{
	uint8_t count;	
	
	PORT_InitTypeDef sPort;
	RST_CLK_PCLKcmd(BACKPLANE_PIN_CLOCK, ENABLE);
	PORT_StructInit( &sPort );
	
	sPort.PORT_OE    = PORT_OE_IN; //режим - вход
	sPort.PORT_FUNC  = PORT_FUNC_PORT;  //функция вывода
	sPort.PORT_MODE  = PORT_MODE_DIGITAL;  //цифровой режим пина
	sPort.PORT_SPEED = PORT_SPEED_SLOW;
	
	for(count = 0 ; count < number_pins; count++ )
	{		
		//RST_CLK_PCLKcmd( PCLK_BIT( pins[count].PORTx ), ENABLE ); // Enable peripheral clock for Port
		sPort.PORT_Pin   = pins[count].PORT_Pin; // Configure Pin
		PORT_Init( pins[count].PORTx, &sPort );
	}
}

//------------------------------получение адреса модуля в кроссплате------------------------------//
int8_t Get_Module_Address(void)
{
	uint8_t count = 0;
	int8_t addr = 0;
	uint8_t number_pins = 0;
	
	const TPortPin pins[] = 
	{
		{ BACKPLANE_ADDR0_PORT, BACKPLANE_ADDR0_PIN },
		{ BACKPLANE_ADDR1_PORT, BACKPLANE_ADDR1_PIN },
		{ BACKPLANE_ADDR2_PORT, BACKPLANE_ADDR2_PIN },
		{ BACKPLANE_ADDR3_PORT, BACKPLANE_ADDR3_PIN },
		{ BACKPLANE_ADDR4_PORT, BACKPLANE_ADDR4_PIN }
	};
	
	number_pins = sizeof(pins)/sizeof (pins[0]); //количество пинов получения адреса
	Pins_Address_Init(pins, number_pins);
	
	Delay_MS(500);

	for(count = 0 ; count < number_pins; count++ )
	//for( count = 0 ; count < 5; count++ )
	{
		addr |= ( (!PORT_ReadInputDataBit( pins[count].PORTx, pins[count].PORT_Pin )) << count); 
	}
	//addr = 0x6;
	#ifdef __USE_DBG
		printf ("my_adress=%d\r\n", addr);
	#endif		 
	if ( addr != 0x00 ) 
		{return addr;} //если получен адрес
	else 
		{return -1;}
}

//-----------------------------активация режима включения светодиода-----------------------------//
void SetBiLED( const TBiLED *pBiLed, TBiLEDColor Color )
{
	switch( Color )
	{
		case LED_BLACK: //бесцветный
			PORT_ResetBits( pBiLed->Red.PORTx, pBiLed->Red.PORT_Pin );
			PORT_ResetBits( pBiLed->Green.PORTx, pBiLed->Green.PORT_Pin );
		break;

		case LED_GREEN: //зелёный
			PORT_SetBits( pBiLed->Red.PORTx, pBiLed->Red.PORT_Pin );
			PORT_ResetBits( pBiLed->Green.PORTx, pBiLed->Green.PORT_Pin );
		break;

		case LED_RED: //красный
			PORT_ResetBits( pBiLed->Red.PORTx, pBiLed->Red.PORT_Pin );
			PORT_SetBits( pBiLed->Green.PORTx, pBiLed->Green.PORT_Pin );
		break;

		case LED_YELLOW: //жёлтый
			PORT_SetBits( pBiLed->Red.PORTx, pBiLed->Red.PORT_Pin );
			PORT_SetBits( pBiLed->Green.PORTx, pBiLed->Green.PORT_Pin );
		break;
	}
}

//---------------------------инициализация пина управления светодиодом---------------------------//
static void InitLED_Pin( const TPortPin *PortPin )  
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

//---------------------------инициализация пинов управления светодиодом---------------------------//
static void InitBiLED( const TBiLED *pBiLed )  
{
	InitLED_Pin( &pBiLed->Red );
	InitLED_Pin( &pBiLed->Green );
}

//-----------------------------инициализация пина сброса GPS-приёмника-----------------------------//
static void GPS_nRST_Init(void)
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
}

//----------------------------------аппаратная перезагрузка приемника----------------------------------//
void GPS_Reset(FunctionalState NewState)
{
	if (NewState == ENABLE) 
		{PORT_ResetBits(GPS_PORT_nRST, GPS_PIN_nRST);} 
	else 
		{PORT_SetBits(GPS_PORT_nRST, GPS_PIN_nRST);}
}

//----------------------------инициализация пина получения сигнала PPS----------------------------//
static void PPS_Pin_Init(void)
{
	PORT_InitTypeDef PORT_InitStructure;
	RST_CLK_PCLKcmd(CLOCK_PPS_PULSE_PIN, ENABLE);
	PORT_StructInit(&PORT_InitStructure);

	PORT_InitStructure.PORT_Pin		= PPS_PULSE_PIN; 
	PORT_InitStructure.PORT_OE 		= PORT_OE_OUT;
	PORT_InitStructure.PORT_FUNC 	= PORT_FUNC_PORT;;
	PORT_InitStructure.PORT_MODE 	= PORT_MODE_DIGITAL;
	PORT_InitStructure.PORT_SPEED = PORT_SPEED_MAXFAST;
	
	PORT_Init(PPS_PULSE_PORT, &PORT_InitStructure);
}

//------------------------------------------------------------------------------------------------//
void Func_GPIO_Init(void)
{
	InitBiLED(&m_Led);
	SetBiLED(&m_Led, LED_YELLOW); //установка бесцветного режима светодиода
	
	PPS_Pin_Init (); //инициализация пина получения сигнала PPS
	GPS_PPS_DISABLE(); //отключение выдачи секундной метки
	
	GPS_nRST_Init(); //инициализация пина сброса GPS-приёмника
	GPS_Reset(DISABLE); //включение GPS-модуля
}
