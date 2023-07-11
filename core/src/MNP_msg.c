
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "MNP_msg.h"
#include "uart_func.h"
//#include "HW_Profile.h"
#include "typedef.h"
#include "protocol.h"
#include "pins.h"
#include "time64.h"
#include "timers.h"
#include "ring_buffer.h"

//Private defines -------------------------------------------------------------------------------------//

//Constants -------------------------------------------------------------------------------------------//

//Private variables -----------------------------------------------------------------------------------//
MKS2_t MKS2; //инициализация структуры типа MKS2_t
MNP_MSG_t MNP_GET_MSG; //структура для приёма сообщения от приёмника
MNP_M7_CFG_t MNP_M7_CFG; //иницализация шаблона структуры перезагрузки и настройки приёмника

//-----------------------------------------------------------------------------------------------------//
static uint16_t MNP_CalcChkSum	(uint16_t *Array, int WordsCount)
{
  uint16_t chksum = 0;
  uint32_t count;

	for( count = 0 ; count < WordsCount; count++ )
	{
		chksum += Array[count];
	}
	
	return (uint16_t)(0 - chksum);
}

//-----------------------------------------------------------------------------------------------------//
void MNP_PutMessage (MNP_MSG_t *Msg, uint16_t MsgId, uint16_t WordsCount)
{
	
	Msg->msg_header.MNP_header.sync = MNP_SYNC_CHAR;
	Msg->msg_header.MNP_header.msg_id = MsgId; //id MNP-сообщения 
	Msg->msg_header.MNP_header.data_size = WordsCount; //количество 16 битных слов в теле сообщения
	Msg->msg_header.MNP_header.dummy = 0;
	Msg->msg_header.MNP_header.chksum = MNP_CalcChkSum((uint16_t *)&Msg->msg_header.MNP_header, (sizeof(HEAD_MNP_MSG_t)-2)/2);
	
	Msg->payload.raw_words[WordsCount] = MNP_CalcChkSum((uint16_t *)&Msg->payload, WordsCount); //сохранение КС
	
	MNP_UART_MSG_Puts (Msg->msg_header.header_bytes, sizeof(HEAD_MNP_MSG_t));
	MNP_UART_MSG_Puts (Msg->payload.raw_bytes, (WordsCount+1)*2);
	
}


//-----------------------------------------------------------------------------------------------------//
void MNP_M7_init ( MNP_MSG_t *Msg)
{
	Msg->payload.msg3006.command_code.cmd_3006.RAM = 0x01; //источник - RAM приёмника
	Msg->payload.msg3006.command_code.cmd_3006.flash = 0x00; //источник - flash приёмника
	Msg->payload.msg3006.command_code.cmd_3006.write = 0x01; //запись уставок
	Msg->payload.msg3006.command_code.cmd_3006.dummy1 = 0x00;
	Msg->payload.msg3006.command_code.cmd_3006.dummy2 = 0x00;
	Msg->payload.msg3006.command_code.cmd_3006.code = 0x02; //загрузка 6 32-разрядных слов с уставками в конфигурацию приёмника
	Msg->payload.msg3006.command_code.dummy = 0x0; //резерв
	
	Msg->payload.msg3006.config.protocol0 = 0x1; //протокол канала 0 - MNP_binary
	Msg->payload.msg3006.config.baud_divider0 = 460800 / 115200; //предделитель UART0=4, скорость 115200 
	Msg->payload.msg3006.config.protocol1 = 0x1; //протокол канала 1 - MNP_binary
	Msg->payload.msg3006.config.baud_divider1 = 460800 / 115200; //предделитель UART1=4, скорость 115200 
	Msg->payload.msg3006.config.tropo_corr = 0x1; //включение использования модели тропосферы
	Msg->payload.msg3006.config.use_difc = 0x1; //включение использования дифференциальных поправок
	Msg->payload.msg3006.config.dif_only = 0x0; //отключение принудительного использования дифференциального режима
	Msg->payload.msg3006.config.sol_smooth = 0x0; //отключение фиксации координат при скорости менее 1 м/c
	Msg->payload.msg3006.config.sol_filter = 0x1; //включение сглаживание решения
	Msg->payload.msg3006.config.meas_filter = 0x1; //включение совмещенной фильтрации по коду и фазе несущей
	Msg->payload.msg3006.config.iono_corr = 0x1; // Разрешение ионосферной коррекции
	Msg->payload.msg3006.config.disable_2D = 0x0; //отключение запрета двумерной навигации
	Msg->payload.msg3006.config.use_RAIM = 0x1; //включение алгоритма RAIM
	Msg->payload.msg3006.config.enable_warm_startup = 0x1; //включение быстрого «горячего» старта
	Msg->payload.msg3006.config.sys_time = 0x1; // Привязка секундной метки к времени навигационной системы 
	Msg->payload.msg3006.config.glo_time = 0x1; // Секундная метка ГЛОНАСС / UTC(SU)
	Msg->payload.msg3006.config.shift_meas = 0x1; // Привязка измерений к секундной метке
	Msg->payload.msg3006.config.enable_SBAS = 0x0; //Разрешение SBAS
	Msg->payload.msg3006.config.enable_iono_SBAS = 0x0; //Разрешение модели ионосферы SBAS
	Msg->payload.msg3006.config.GPS_compatibility = 0x0; //Режим совместимости с приемниками GPS
	Msg->payload.msg3006.config.wr_alms = 0x1; //включение сохранения в flash альманахов 
	Msg->payload.msg3006.config.wr_ephs = 0x1; //включение сохранения в flash эфемерид 
	Msg->payload.msg3006.config.wr_ionoutc = 0x1; //включение сохранения в flash модели UTC GPS
	Msg->payload.msg3006.config.wr_coords = 0x1; //включение сохранения в flash координат
	Msg->payload.msg3006.config.enable_3000_1000_GGA_0 = 0x1; //включение кадра 3000/1000/«GGA» по каналу 0
	Msg->payload.msg3006.config.enable_3011_1002_GSA_0 = 0x0; //отключение кадра 3011/1002/«GSA» по каналу 0
	Msg->payload.msg3006.config.enable_3002_1003_GSV_0 = 0x0; //отключение кадра 3002/1003/«GSV» по каналу 0
	Msg->payload.msg3006.config.enable_3003_1012_RMC_0 = 0x0; //отключение кадра 3003/1012/«RMC» по каналу 0
	Msg->payload.msg3006.config.enable_3000_1000_GGA_1 = 0x1; //отключение кадра 3000/1000/«GGA» по каналу 1
	Msg->payload.msg3006.config.enable_3011_1002_GSA_1 = 0x0; //отключение кадра 3011/1002/«GSA» по каналу 1
	Msg->payload.msg3006.config.enable_3002_1003_GSV_1 = 0x0; //отключение кадра 3002/1003/«GSV» по каналу 1
	Msg->payload.msg3006.config.enable_3003_1012_RMC_1 = 0x0; //отключение кадра 3003/1012/«RMC» по каналу 1
	
	MNP_PutMessage (Msg, MSG_3006, ((sizeof(Msg->payload.msg3006.config) + sizeof(Msg->payload.msg3006.command_code))/2)); //запись настроек в RAM приёмника
}

//-----------------------------------------------------------------------------------------------------//
void read_config_MNP ( MNP_MSG_t *Msg)
{
	Msg->payload.msg3006.command_code.cmd_3006.RAM = 0x01; //1-источник - RAM приёмника
	Msg->payload.msg3006.command_code.cmd_3006.flash = 0x00; //1-источник - flash приёмника
	Msg->payload.msg3006.command_code.cmd_3006.write = 0x00; //0-чтение уставок, 1-запись уставок
	Msg->payload.msg3006.command_code.cmd_3006.dummy1 = 0x00;
	Msg->payload.msg3006.command_code.cmd_3006.dummy2 = 0x00;
	Msg->payload.msg3006.command_code.cmd_3006.code = 0x02; //6 32-разрядных слов конфигурации
	Msg->payload.msg3006.command_code.dummy = 0x00; //резерв		
	
	MNP_PutMessage (Msg, MSG_3006, (sizeof(Msg->payload.msg3006.command_code)/ 2)); //чтение настроек из RAM приёмника
}

//-----------------------------------------------------------------------------------------------------//
void read_flash_MNP ( MNP_MSG_t *Msg)
{
	Msg->payload.msg3006.command_code.cmd_3006.RAM = 0x00; //1-источник - RAM приёмника
	Msg->payload.msg3006.command_code.cmd_3006.flash = 0x01; //1-источник - flash приёмника
	Msg->payload.msg3006.command_code.cmd_3006.write = 0x00; //0-чтение уставок, 1-запись уставок
	Msg->payload.msg3006.command_code.cmd_3006.dummy1 = 0x00;
	Msg->payload.msg3006.command_code.cmd_3006.dummy2 = 0x00;
	Msg->payload.msg3006.command_code.cmd_3006.code = 0x02; //6 32-разрядных слов конфигурации
	Msg->payload.msg3006.command_code.dummy = 0x00; //резерв
	
	MNP_PutMessage (Msg, MSG_3006, (sizeof(Msg->payload.msg3006.command_code)/ 2)); //чтение настроек из flash приёмника
}

//-----------------------------------------------------------------------------------------------------//
void put_msg2000 (MNP_MSG_t *Msg)
{
	MNP_PutMessage (Msg, MSG_2000, 0);
}

//-----------------------------------------------------------------------------------------------------//
void Set_GNSS_interval (MNP_MSG_t *Msg, uint32_t inerval)
{
	Msg->payload.msg3006.command_code.cmd_3006.RAM = 0x01; //источник - RAM приёмника
	Msg->payload.msg3006.command_code.cmd_3006.flash = 0x00; //источник - flash приёмника
	Msg->payload.msg3006.command_code.cmd_3006.write = 0x01; //запись уставок
	Msg->payload.msg3006.command_code.cmd_3006.dummy1 = 0x00;
	Msg->payload.msg3006.command_code.cmd_3006.dummy2 = 0x00;	
	Msg->payload.msg3006.command_code.cmd_3006.code = 0x07; //настройка длины интервала измерения
	Msg->payload.msg3006.command_code.dummy = 0x00; //резерв
	
	Msg->payload.msg3006.interval = inerval; //Длина интервала измерения, 2000 соответствует 1с
	
	MNP_PutMessage (Msg, MSG_3006, (sizeof(Msg->payload.msg3006.interval) + sizeof(Msg->payload.msg3006.command_code))/2);
}

//-----------------------------------------------------------------------------------------------------//
void Get_GNSS_interval (MNP_MSG_t *Msg, uint32_t inerval)
{
	Msg->payload.msg3006.command_code.cmd_3006.RAM = 0x01; //источник - RAM приёмника
	Msg->payload.msg3006.command_code.cmd_3006.flash = 0x00; //источник - flash приёмника
	Msg->payload.msg3006.command_code.cmd_3006.write = 0x00; //запись уставок
	Msg->payload.msg3006.command_code.cmd_3006.dummy1 = 0x00;
	Msg->payload.msg3006.command_code.cmd_3006.dummy2 = 0x00;	
	Msg->payload.msg3006.command_code.cmd_3006.code = 0x07; //настройка длины интервала измерения
	Msg->payload.msg3006.command_code.dummy = 0x00; //резерв
	
	Msg->payload.msg3006.interval = inerval; //Длина интервала измерения, 2000 соответствует 1с
	
	MNP_PutMessage (Msg, MSG_3006, (sizeof(Msg->payload.msg3006.interval) + sizeof(Msg->payload.msg3006.command_code))/2);
}

//-----------------------------------------------------------------------------------------------------//
void Read_SN (MNP_MSG_t *Msg)
{
	Msg->payload.msg3006.command_code.cmd_3006.RAM = 0x01; //источник - RAM приёмника
	Msg->payload.msg3006.command_code.cmd_3006.flash = 0x00; //источник - flash приёмника
	Msg->payload.msg3006.command_code.cmd_3006.write = 0x00; //чтение уставок
	Msg->payload.msg3006.command_code.cmd_3006.dummy1 = 0x00;
	Msg->payload.msg3006.command_code.cmd_3006.dummy2 = 0x00;	
	Msg->payload.msg3006.command_code.cmd_3006.code = 0x16; //чтение серийного номера приёмника
	Msg->payload.msg3006.command_code.dummy = 0x00; //резерв
	
	
	MNP_PutMessage (Msg, MSG_3006, ((sizeof(Msg->payload.msg3006.command_code))/2));
}

//-------------------------------------------------------------------------------------------------//
void MKS_context_ini (void)
{
	MNP_GET_MSG.rx_state = __SYNC_BYTE1; 

	MNP_M7_CFG.cfg_state = __SYNC_RST ,
//	MNP_M7_CFG.parse_delay = GPS_PARSE_DELAY,
//	MNP_M7_CFG.cfg_msg_delay = GPS_CFG_MSG_DELAY, //
//	MNP_M7_CFG.rst_delay = GPS_RST_DELAY,	//задержка для перезагрузки приёмника

	
	MKS2.tmContext.Time2k = 0, //количество секунд с 01.01.2000
	MKS2.tmContext.TAI_UTC_offset = 0, //разница между атомным временем и временем UTC
	MKS2.tmContext.LeapS_59 = 0,									//високосная секунда 
	MKS2.tmContext.LeapS_61 = 0,									//високосная секунда
	MKS2.tmContext.ValidTHRESHOLD = 0,						//накопленная достоверность данных		
	MKS2.tmContext.Valid = 0,                  		//сброс флага достоверности данных		
	MKS2.tmContext.Max_gDOP = DEFAULT_MAX_gDOP, 	//установка максимально допустимого значения gDOP
		
	MKS2.canContext.ID = MODULE_TYPE_MKNS, 					//инициализация типа модуля
	MKS2.canContext.GetAddr = &Get_Module_Address, //инициализация указателя на функцию получения адреса в кросс-плате
	MKS2.fContext.Fail = 0; //все флаги аппаратных неисправностей сброшены
}

//-----------------------------------------Инициализация МНП-М7-----------------------------------------//
void GPS_Init(MNP_MSG_t *Msg)
{
	MNP_M7_init (Msg);		
}

//---------------------------------сообщение перезагрузки GPS приемника---------------------------------//
void MNP_Reset(MNP_MSG_t *Msg)
{
	memset ((void *)&Msg->payload.msg3006, 0, (sizeof(Msg->payload.msg3006.command_code)+sizeof(Msg->payload.msg3006.reset_mask)));

	Msg->payload.msg3006.command_code.specialcmd_3006.special_code = SPECIAL_CMD_CODE_RESET;
	Msg->payload.msg3006.command_code.specialcmd_3006.id = SPECIAL_CMD_ID; 
	MNP_PutMessage (Msg, MSG_3006, (sizeof(Msg->payload.msg3006.command_code)+sizeof(Msg->payload.msg3006.reset_mask))/ 2);
}

//--------------------------------------Парсер сообщение от МНП-М7--------------------------------------//
static int8_t Parse_MNP_MSG (MNP_MSG_t * Msg)
{
	uint8_t byte; //полученный из кольцевого буффера байт
	static uint16_t byte_i; //счётчик принятых байтов тела сообщения
	int8_t ret = -1; //результат парсинга
//	float tDOP = 0;
	
	Msg->msg_header.MNP_header.sync = MNP_SYNC_CHAR;
	
	while (RING_GetCount(&RING_buffer) > 0)
//	do
	{		
		byte = RING_Pop(&RING_buffer); //получение из кольцевого буфера байта
		switch ( Msg->rx_state ) //проверка стадии получения сообщения
		{
			case __SYNC_BYTE1: //стадия получения младшего байта синхрослова
				if ( byte == (uint8_t)Msg->msg_header.header_bytes[0]) //если полученный байт равен 0xFF (младшая часть синхрослова)
					{Msg->rx_state = __SYNC_BYTE2;}//переход на стадию получения старшего байта синхрослова
				break;
				
			case __SYNC_BYTE2: //стадия получения старшего байта синхрослова
				if ( byte == (uint8_t)Msg->msg_header.header_bytes[1] ) //если полученный байт равен 0x81 (старшая часть синхрослова) 
					{Msg->rx_state = __TYPE_ID_BYTE1;} //переход на стадию получения младшего байта идентификатора типа кадра
				else 
				{
					Msg->rx_state = __SYNC_BYTE1; //переход на стадию получения младшего байта синхрослова
				} 					
				break;	
				
			case __TYPE_ID_BYTE1:  //стадия получения младшего байта идентификатора типа кадра
				Msg->rx_state = __TYPE_ID_BYTE2; //переход на стадию получения старшего байта идентификатора типа кадра
				Msg->msg_header.MNP_header.ID_B = byte; //сохранение младшего байта поля идентификатора типа кадра			
				break;
			
			case __TYPE_ID_BYTE2:  //стадия получения старшего байта идентификатора типа кадра
				Msg->rx_state = __LENGTH_BYTE1; //переход на стадию получения младшего байта байта поля длины сообщения
				Msg->msg_header.MNP_header.ID_A = byte; //сохранение старшего байта идентификатора типа кадра
				break;
			
			case __LENGTH_BYTE1: //стадия получения младшего байта поля длины сообщения
				Msg->rx_state = __LENGTH_BYTE2; //переход на стадию получения старшего байта поля длины сообщения
				Msg->msg_header.MNP_header.length_B = byte; //сохранение младшего байта поля длины сообщения
				break;	
			
			case __LENGTH_BYTE2: //стадия получения старшего байта поля длины сообщения
				Msg->rx_state = __RESERVE_BYTE1;  //переход на стадию получения 1 байта резервного слова заголовка
				Msg->msg_header.MNP_header.length_A = byte; //сохранение старшего байта поля длины сообщения							
				byte_i = 0; //обнуление счётчика принятых байтов полезной части сообщения
			
				break;
			
			case __RESERVE_BYTE1: //стадия получения 1 байта резервного слова заголовка
				Msg->msg_header.MNP_header.dummy_B =  byte; 
				Msg->rx_state = __RESERVE_BYTE2;  //переход на стадию получения 2 байта резервного слова заголовка
				break;
			
			case __RESERVE_BYTE2: //стадия получения 2 байта резервного слова заголовка
				Msg->msg_header.MNP_header.dummy_A =  byte; 
				Msg->rx_state = __CK_HEAD_BYTE1;  //переход на стадию получения младшего байта КС заголовка
				break;
			
			case __CK_HEAD_BYTE1: //стадия получения младшего байта КС заголовка
				Msg->msg_header.MNP_header.chksum_B = byte; //охранение младшего байта КС заголовка
				Msg->rx_state = __CK_HEAD_BYTE2;  //переход на стадию получения старшего байта КС заголовка
				break;
			
			case __CK_HEAD_BYTE2: //стадия получения старшего байта КС заголовка
				Msg->msg_header.MNP_header.chksum_A = byte; //охранение старшего байта КС заголовка
			//	sprintf (DBG_buffer, "get_byte=%u\r\n", RING_GetCount(&RING_buffer));
			//	DBG_PutString (DBG_buffer);
				Msg->rx_state = __PAYLOAD;  //переход на стадию получения полезной части сообщения
				break;
			
			case __PAYLOAD: //стадия получения полезной части сообщения
				if ( byte_i < 2*Msg->msg_header.MNP_header.data_size ) //если счётчик принятых байтов меньше полученной длины сообщения
				{
					Msg->payload.raw_bytes[byte_i] = byte; //сохранения принятого байта					
					byte_i++;
					if (byte_i >= BUFFER_SIZE)
						{byte_i = 0;}
				} 
				else //если полезная часть сообщения получена полностью
				{
					byte_i = 0;
					Msg->rx_state = __PARSER; //переход на стадию парсинга сообщения		
				} 
				break;				
			
			case __PARSER: //стадия парсинга полученного сообщения
				Msg->rx_state = __SYNC_BYTE1;	//стадия получения 1 синхрослова	
				if ( MNP_CalcChkSum((uint16_t*)&Msg->msg_header, (sizeof(HEAD_MNP_MSG_t)-2)/2) == (Msg->msg_header.MNP_header.chksum)) // проверка контрольной суммы			
				{	
					switch ( Msg->msg_header.MNP_header.msg_id ) //проверка  идентификатора типа кадра
					{
						case MSG_3000:
							#ifdef __USE_DBG
							sprintf (DBG_buffer, (char *)"hour=%d,min=%d,sec=%d\r\n", Msg->payload.msg3000.hour,  
							Msg->payload.msg3000.minute, Msg->payload.msg3000.second);
							printf ("%s", DBG_buffer);
							#endif
							ret = 1; //результат парсинга равен 1 
							break;
						
						case MSG_3001: 
							sprintf (DBG_buffer, (char *)"id=%x, data_size=%x\r\n", Msg->msg_header.MNP_header.msg_id, Msg->msg_header.MNP_header.data_size);				
							printf ("%s", DBG_buffer);
							ret = 2; //результат парсинга равен 2
							break;
						
						case MSG_3011: 
							#ifdef __USE_DBG
							sprintf (DBG_buffer, (char *)"id=%x, data_size=%x\r\n", Msg->msg_header.MNP_header.msg_id, Msg->msg_header.MNP_header.data_size);				
							printf ("%s", DBG_buffer);
							#endif
							ret = 3; //результат парсинга равен 3
							break;
						
						case MSG_3002:
							#ifdef __USE_DBG
							sprintf (DBG_buffer, (char *)"id=%x, data_size=%x\r\n", Msg->msg_header.MNP_header.msg_id, Msg->msg_header.MNP_header.data_size);				
							printf ("%s", DBG_buffer);
							#endif
							ret = 4; //результат парсинга равен 4
							break;
						
						case MSG_3003:
							#ifdef __USE_DBG
							sprintf (DBG_buffer, (char *)"id=%x, data_size=%x\r\n", Msg->msg_header.MNP_header.msg_id, Msg->msg_header.MNP_header.data_size);				
							printf ("%s", DBG_buffer);
							#endif
							ret = 5; //результат парсинга равен 5
							break;
						
						case MSG_3006:
							#ifdef __USE_DBG
							sprintf (DBG_buffer, (char *)"get msg3006, data_size=%x, code=%u\r\n", Msg->msg_header.MNP_header.data_size, 
							Msg->payload.msg3006.command_code.cmd_3006.code);				
							printf ("%s", DBG_buffer);							
							if (Msg->payload.msg3006.command_code.cmd_3006.code == 7)
							{
								sprintf (DBG_buffer, (char *)"interval=%u\r\n",Msg->payload.msg3006.interval);
								printf ("%s", DBG_buffer);
							}
							#endif
							ret = 6; //результат парсинга равен 6
							break;
						
						case MSG_2200:
							#ifdef __USE_DBG
							sprintf (DBG_buffer, (char *)"id=%x, data_size=%x, dummy=%x\r\n", Msg->msg_header.MNP_header.msg_id, 
							Msg->msg_header.MNP_header.data_size, Msg->msg_header.MNP_header.dummy);				
							printf ("%s", DBG_buffer);
							#endif
							ret = 7; //результат парсинга равен 7
							break;
					
						default:
							#ifdef __USE_DBG
							sprintf (DBG_buffer, (char *)"parcing_error,id=%x,data_size=%x\r\n", Msg->msg_header.MNP_header.msg_id, Msg->msg_header.MNP_header.data_size);				
							printf ("%s", DBG_buffer);
							#endif
							break;						
					}
				}
				else
				{
					#ifdef __USE_DBG
					sprintf (DBG_buffer, "header_CRC_error, %x!=%x\r\n", MNP_CalcChkSum((uint16_t*)&Msg->msg_header, (sizeof(HEAD_MNP_MSG_t)-2)/2), Msg->msg_header.MNP_header.chksum);
					printf ("%s", DBG_buffer);
					#endif
				}
				RING_Clear(&RING_buffer); 
				break;				
				
			default:
			break;			
		}
	} 
//	while ( RING_GetCount(&RING_buffer) > 0 ); //пока в кольцевом буфере счётчик принятых байтов больше счётчика обработанных байтов
	return ret; //возврат результата парсинга 
}

//------------------------------Обработка временных данных от GPS приемника------------------------------//
static void GPS_Read_Data(MNP_MSG_t *Msg)
{
	struct TM TimeStamp;
	//Time64_T 	UNIX_Time64;
	int64_t UNIX_Time64;
	float gDOP;
	
	gDOP = Msg->payload.msg3000.gDOP;
	// проверка параметра gDOP на допустимое значения
	if ( gDOP < (MKS2.tmContext.Max_gDOP)) //если gDOP находится в диапазоне от 1 до 4
	{				
		TimeStamp.tm_mday = Msg->payload.msg3000.day;
		TimeStamp.tm_mon = Msg->payload.msg3000.month;			
		TimeStamp.tm_year = Msg->payload.msg3000.year - 1900;
		TimeStamp.tm_hour = Msg->payload.msg3000.hour;
		TimeStamp.tm_min = Msg->payload.msg3000.minute;
			
		if ( Msg->payload.msg3000.second == 60 ) //обработка события високосной секунды
			{TimeStamp.tm_sec = 59;} 
		else 
			{TimeStamp.tm_sec = Msg->payload.msg3000.second;}
			
		UNIX_Time64 = mktime64(&TimeStamp); //перевод полученной даты в формат UNIX (01.01.1970)
			
		MKS2.tmContext.Time2k = (uint32_t)(UNIX_Time64 - 946684800); //время прошедшее с 01.01.2000
	//	if (!(Msg->payload.msg3000.flags.GPS) || !(Msg->payload.msg3000.flags.UTC)) // достоверность информации от GPS приемника (если установлена 1 то данные недостоверны)
		{
			if (!(MKS2.tmContext.ValidTHRESHOLD & DEFAULT_MASK_ValidTHRESHOLD )) //DEFAULT_MASK_ValidTHRESHOLD == 0b100
				{MKS2.tmContext.ValidTHRESHOLD++;} //четыре принятые посылки должны быть достоверны
			else 
				{MKS2.tmContext.Valid = 1;} //установка флага достоверности данных
		}
	}
	else
	{
		if (MKS2.tmContext.ValidTHRESHOLD != 0)
			{MKS2.tmContext.ValidTHRESHOLD--;} //уменьшение счётчика достоверности
		MKS2.tmContext.Valid = 0; //сброс флага достоверности времени 
		#ifdef __USE_DBG
		sprintf (DBG_buffer, (char *)"pDOP=%0.2f, gDOP=%0.2f, %u %u\r\n", Msg->payload.msg3000.pDOP, 
		Msg->payload.msg3000.gDOP, Msg->payload.msg3000.flags.GPS, Msg->payload.msg3000.flags.UTC);
		printf ("%s", DBG_buffer);
		#endif
	}
}

//-------------------------------------------------------------------------------------------------------------//
int8_t GPS_wait_data_Callback (void)
{
	int8_t result = -1;
	
	if (RING_GetCount(&RING_buffer) > 0)
	{	
		if ((result = Parse_MNP_MSG (&MNP_GET_MSG)) > 0) //если сообщение от приёмника получено полностью
		{
			MKS2.fContext.GPS = 0; //соединение с gps-приёмником успешно установлено
			GPS_Read_Data(&MNP_GET_MSG);
			
			Reload_Timer_GPS_UART_Timeout(); //перегрузка таймера обработки таймаута
			
			//if (MKS2.tmContext.Valid) // отправка сообщения A при достоверной информации от GPS приемника
			//	{MKS2.canContext.MsgA1Send();} //отправка сообщения типа А1
			if (MKS2.tmContext.ValidTHRESHOLD == 0) //если сообщение получено, но данные недостоверны
			{
				//GPS_Config(); //запуск таймера перезагрузки модуля				
			}			
		}	
		else
		{}
	}
	return result;
}

//-----------------------------------------------------------------------------------------------------//
void GPS_Config(void)
{
	#ifdef __USE_DBG
	printf ("MNP_reset\r\n");
	#endif
	GPS_Reset(ENABLE); //активировать пин аппаратной перезагрузки приёмника
	MNP_M7_CFG.cfg_state = __SYNC_RST; //статус - перезагрузка модуля
	Create_Timer_configure_GPS (); //создание таймера перезагрузки и настройки приёмника
}

//-----------------------------------------------------------------------------------------------------//


