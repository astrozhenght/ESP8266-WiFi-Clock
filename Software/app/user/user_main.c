#include "stdlib.h"
#include "user_config.h"		// �û�����
#include "c_types.h"			// ��������
#include "eagle_soc.h"			// GPIO�������궨��
#include "ip_addr.h"			// ��"espconn.h"ʹ��
#include "espconn.h"			// TCP/UDP�ӿ�
#include "ets_sys.h"			// �ص�����
#include "mem.h"				// �ڴ�����Ⱥ���
#include "os_type.h"			// os_XXX
#include "osapi.h"  			// os_XXX�������ʱ��
#include "sntp.h"				// SNTP
#include "user_interface.h" 	// ϵͳ�ӿڡ�system_param_xxx�ӿڡ�WIFI��RateContro
#include "smartconfig.h"

#include "driver/lcd_init.h"
#include "driver/lcd.h"
#include "driver/pic.h"

extern u8 locx_now, locy_now;  			//ȡʵ�����ص��ֵ
extern u16 segColor;
extern u8 locx[];
extern u8 block_len;

u8 hour = 19, minute = 59, second = 55; //��ǰ��ʱ��ֵ
u8 _hour, _minute, _second;  			//�ϴε�ʱ��ֵ
u8 flag = 1;							//0:ˢ�¶���; 1:��ʼ������;
u8 _str_date[10] = {0};					//�ϴθ��µ���������
u8 num = 0;

#define		Sector_STA_INFO		0x90	//STA������������

os_timer_t 	OS_Timer_IP;				//�����ʱ��
os_timer_t 	OS_Timer_SNTP;				//��ʱ��_SNTP

void ICACHE_FLASH_ATTR OS_Timer_SNTP_cb(void * arg)	// SNTP��ʱ�ص�����
{
	u8 cnt = 0;					// �ַ����ֽڼ���
	u8 index = 0;				// ��Ӧstr_date�±�
	uint32 TimeStamp;			// ʱ���
	char tmp[5] = {0};			// ��������
	char str_clock[10] = {0};	// ʱ������
	char str_date[10] = {0};	// ��������
	char * p_week;
	char * p_month;
	char * p_day;
	char * p_clock;
	char * Str_RealTime;		// ʵ��ʱ����ַ���

	TimeStamp = sntp_get_current_timestamp();	// ��ѯ��ǰ�����׼ʱ��(1970.01.01 00:00:00 GMT+8)��ʱ���(��λ:��)

	if(TimeStamp)		// �ж��Ƿ��ȡ��ƫ��ʱ��
	{
		Str_RealTime = sntp_get_real_time(TimeStamp);// ��ѯʵ��ʱ��(GMT+8):����������ʱ�� "�� �� �� ʱ:��:�� ��"
//		os_printf("SNTP_InternetTime = %s\n", Str_RealTime);	// ʱ���ʽ�� Wed Jun 17 18:16:00 2020

		p_week 	= Str_RealTime;						// ��ȡ"����" �ַ������׵�ַ
		p_month = os_strstr(p_week,	" ") + 1;		// ��ȡ"�·�" �ַ������׵�ַ
		p_day 	= os_strstr(p_month, " ") + 1;		// ��ȡ"����" �ַ������׵�ַ
		p_clock = os_strstr(p_day,	" ") + 1;		// ��ȡ"ʱ��" �ַ������׵�ַ

		//�����·�
		cnt = p_day - p_month - 1;				// �·ݵ��ֽ���
		os_memcpy(tmp, p_month, cnt);
		tmp[cnt] = '\0';						// ĩβ�ӽ�����
		if(strcmp(tmp, "Jan") == 0)
		{
			str_date[index++] = '1';
		}
		else if (strcmp(tmp, "Feb") == 0)
		{
			str_date[index++] = '2';
		}
		else if (strcmp(tmp, "Mar") == 0)
		{
			str_date[index++] = '3';
		}
		else if (strcmp(tmp, "Apr") == 0)
		{
			str_date[index++] = '4';
		}
		else if (strcmp(tmp, "May") == 0)
		{
			str_date[index++] = '5';
		}
		else if (strcmp(tmp, "Jun") == 0)
		{
			str_date[index++] = '6';
		}
		else if (strcmp(tmp, "Jul") == 0)
		{
			str_date[index++] = '7';
		}
		else if (strcmp(tmp, "Aug") == 0)
		{
			str_date[index++] = '8';
		}
		else if (strcmp(tmp, "Sep") == 0)
		{
			str_date[index++] = '9';
		}
		else if (strcmp(tmp, "Oct") == 0)
		{
			str_date[index++] = '1';
			str_date[index++] = '0';
		}
		else if (strcmp(tmp, "Nov") == 0)
		{
			str_date[index++] = '1';
			str_date[index++] = '1';
		}
		else if (strcmp(tmp, "Dec") == 0)
		{
			str_date[index++] = '1';
			str_date[index++] = '2';
		}
		str_date[index++] = '/';

		//��������
		cnt = p_clock - p_day - 1;				// �������ֽ���
		os_memcpy(str_date+index, p_day, cnt);	// ����������str_date����
		if(str_date[index] == '0')				// ȥ��07ǰ��0
		{
			str_date[index] = str_date[index+1];
			index += (cnt-1);
		}
		else
			index += cnt;						// ָ��ո�
		str_date[index++] = ' ';				// ��ӿո�

		//��������
		cnt = p_month - p_week - 1;				// ���ڵ��ֽ���
		os_memcpy(str_date+index, p_week, cnt);	// ����������str_date����
		index += cnt;							// ָ��ո�
		str_date[index] = '\0';

		//����ʱ��
		os_memcpy(str_clock, p_clock, 8);		// ��ʱ�� �ַ�������str_clock����
		str_clock[8] = '\0';
		p_clock = str_clock;
		hour = atoi(p_clock);					//��ʱ���֡������ַ���תΪ����
		minute = atoi(p_clock + 3);
		second = atoi(p_clock + 6);

		if(flag == 1)									//��ʼ��ʱ����ʾ����
		{
			flag = 0;
			segColor = WHITE;							//������ɫǳ��ɫ
			LCD_Fill(0,0,LCD_W,LCD_H,SKYBLUE);			//����
			DIGIT_DrawDot();							//��ʾð��
			DIGIT_DrawStartTime(hour, minute, second);	//��ʾ��ʼʱ��
			strcpy(_str_date, str_date);
			switch(strlen(str_date))
			{
			case 7:
				LCD_ShowString(53, 2, str_date, WHITE, SKYBLUE, 24, 0);	//��������
				break;
			case 8:
				LCD_ShowString(47, 2, str_date, WHITE, SKYBLUE, 24, 0);	//��������
				break;
			case 9:
				LCD_ShowString(41, 2, str_date, WHITE, SKYBLUE, 24, 0);	//��������
				break;
			default:
				os_printf("\r\n---- strlen(str_date) error ----\r\n");
				break;
			}
			strcpy(_str_date, str_date);				//���汾������
			LCD_ShowPicture(7, 6, 20, 16, gImage_24);	//��ʾwifiͼ��
			_hour = hour;								//���ϴ�ʱ��ֵ��ֵ
			_minute = minute;
			_second = second;
		}

		if(flag == 0)						//ˢ�¶���
		{
			if(strcmp(_str_date, str_date))	//��������
			{
				LCD_Fill(40, 2, 148, 26, SKYBLUE);	// �����ʾ����
				switch(strlen(str_date))	//��ȡ���ȣ�΢��������ʾ�����ַ���
				{
				case 7:
					LCD_ShowString(52, 2, str_date, WHITE, SKYBLUE, 24, 0);
					break;
				case 8:
					LCD_ShowString(46, 2, str_date, WHITE, SKYBLUE, 24, 0);
					break;
				case 9:
					LCD_ShowString(40, 2, str_date, WHITE, SKYBLUE, 24, 0);
					break;
				default:
					os_printf("\r\n---- strlen(str_date) error ----\r\n");
					break;
				}
				strcpy(_str_date, str_date);			//���汾������
			}
			if(_second != second)
			{
				block_len = 2;
				locx_now = locx[6];
				locy_now = 44;
				DIGIT_Morph(second%10, _second%10);	 	//���λ�Ķ���
				if (second/10 !=  _second/10)
				{
					locx_now = locx[5];
					DIGIT_Morph(second/10, _second/10); //��ʮλ�Ķ���
				}
			}
			if(_minute != minute)
			{
				block_len = 3;
				locx_now = locx[4];						//��λ�����Ӹ�λλ��
				locy_now = 30;
				DIGIT_Morph(minute%10, _minute%10);		//���·��ӵĸ�λ
				if (minute/10 != _minute/10)
				{
					locx_now = locx[3];
					DIGIT_Morph(minute/10, _minute/10);
				}
			}
			if(_hour != hour)							//��ʱ
			{
				block_len = 3;
				locx_now = locx[2]; 		 			//��λ��Сʱ��λλ��
				locy_now = 30;
				DIGIT_Morph(hour%10, _hour%10);	 		//����Сʱ�ĸ�λ
				if (hour/10 != _hour/10)
				{
					locx_now = locx[1];		 			//��λ��Сʱʮλλ��
					DIGIT_Morph(hour/10, _hour/10);		//����Сʱ��ʮλ
				}
			}
			if(wifi_station_get_connect_status() != STATION_GOT_IP)	// �ж��Ƿ�ɹ�����WIFI��δ������ʾWIFI����
			{
				num++;
				if(num == 3)	//3����˸һ��
				{
					LCD_ShowPicture(7, 6, 20, 16, gImage_21);	//��ʾһ���ź�
					delay_ms(20);
					LCD_ShowPicture(7, 6, 20, 16, gImage_22);	//��ʾ�����ź�
					delay_ms(20);
					LCD_ShowPicture(7, 6, 20, 16, gImage_23);	//��ʾ�����ź�
					delay_ms(20);
					LCD_ShowPicture(7, 6, 20, 16, gImage_24);	//��ʾ�ĸ��ź�
					delay_ms(20);
					num = 0;
				}
			}
		}
		//���汾��ʱ��
		_hour = hour;
		_minute = minute;
		_second = second;
	}
}

void ICACHE_FLASH_ATTR OS_Timer_SNTP_Init(u32 time_ms, u8 time_repetitive)	// SNTP��ʱ��ʼ��
{
	os_timer_disarm(&OS_Timer_SNTP);
	os_timer_setfn(&OS_Timer_SNTP,(os_timer_func_t *)OS_Timer_SNTP_cb,NULL);
	os_timer_arm(&OS_Timer_SNTP, time_ms, time_repetitive);
}

void ICACHE_FLASH_ATTR ESP8266_SNTP_Init(void)	// ��ʼ��SNTP���̶��ĳ�����
{
	ip_addr_t * addr = (ip_addr_t *)os_zalloc(sizeof(ip_addr_t));

	sntp_setservername(0, "us.pool.ntp.org");	// ������_0����
	sntp_setservername(1, "ntp.sjtu.edu.cn");	// ������_1����

	ipaddr_aton("210.72.145.44", addr);			// ���ʮ���� => 32λ������
	sntp_setserver(2, addr);					// ������_2��IP��ַ
	os_free(addr);								// �ͷ�addr

	sntp_init();								// SNTP��ʼ��API
	OS_Timer_SNTP_Init(1000, 1);				// 1���ظ���ʱ(SNTP)
}

void ICACHE_FLASH_ATTR smartconfig_done_cb(sc_status status, void *pdata)	// �ڲ�ͬ״̬�£�pdata��������ǲ�ͬ��
{
	os_printf("\r\n---- smartconfig_cb ----\r\n");
    switch(status)
    {
		case SC_STATUS_WAIT:				// CmartConfig�ȴ�
		break;
		case SC_STATUS_FIND_CHANNEL:		// ����WIFI�źţ�8266������״̬�µȴ�������
			os_printf("\r\n---- Please Use WeChat to SmartConfig ----\r\n\r\n");
			LCD_Fill(0,0,LCD_W,LCD_H,SKYBLUE);
			LCD_ShowChinese(7,28,"����΢������",WHITE,SKYBLUE,24,0);
		break;
        case SC_STATUS_GETTING_SSID_PSWD:	// ���ڻ�ȡSSID��PSWD����״̬�� ����2==SmartConfig����ָ��
	    break;
        case SC_STATUS_LINK:	// �ɹ���ȡ��SSID��PSWD������STA������������WIFI
        	os_printf("\r\nSC_STATUS_LINK\r\n");
            struct station_config *sta_conf = pdata;	// ��ȡSTA����ָ�룬��״̬�� ����2==STA����ָ��
			spi_flash_erase_sector(Sector_STA_INFO);	// ��������
			spi_flash_write(Sector_STA_INFO*4096, (uint32 *)sta_conf, 96);	// ��wifi��Ϣ���浽�ⲿFlash������
	        wifi_station_set_config(sta_conf);			// ����STA����
	        wifi_station_disconnect();					// �Ͽ�STA����
	        wifi_station_connect();						// ESP8266����WIFI
	        LCD_Fill(0,0,LCD_W,LCD_H,SKYBLUE);
	    	LCD_ShowChinese(7, 28, "��������", WHITE, SKYBLUE, 24, 0);
	    	LCD_ShowString(107, 28, "WiFi", WHITE, SKYBLUE, 24, 0);
	    break;
        case SC_STATUS_LINK_OVER:		// ESP8266��ΪSTA���ɹ����ӵ�WIFI
            smartconfig_stop();			// ֹͣSmartConfig
			ESP8266_SNTP_Init();		// ��ʼ��SNTP
			os_printf("\r\n---- ESP8266 Connect to WIFI Successfully ----\r\n");
	    break;
    }
}

void ICACHE_FLASH_ATTR OS_Timer_IP_cb(void)
{
	struct ip_info ST_ESP8266_IP;	// ESP8266��IP��Ϣ
	u8 ESP8266_IP[4];				// ESP8266��IP��ַ
	u8 S_WIFI_STA_Connect = wifi_station_get_connect_status();

	if(S_WIFI_STA_Connect == STATION_GOT_IP)	// �ж��Ƿ�ɹ�����WIFI
	{
		wifi_get_ip_info(STATION_IF,&ST_ESP8266_IP);	// ��ȡSTA��IP��Ϣ
		ESP8266_IP[0] = ST_ESP8266_IP.ip.addr;			// IP��ַ�߰�λ == addr�Ͱ�λ
		ESP8266_IP[1] = ST_ESP8266_IP.ip.addr>>8;		// IP��ַ�θ߰�λ == addr�εͰ�λ
		ESP8266_IP[2] = ST_ESP8266_IP.ip.addr>>16;		// IP��ַ�εͰ�λ == addr�θ߰�λ
		ESP8266_IP[3] = ST_ESP8266_IP.ip.addr>>24;		// IP��ַ�Ͱ�λ == addr�߰�λ
		os_printf("\r\n---- ESP8266_IP = %d.%d.%d.%d ----\r\n",ESP8266_IP[0],ESP8266_IP[1],ESP8266_IP[2],ESP8266_IP[3]);// ��ʾESP8266��IP��ַ
		os_timer_disarm(&OS_Timer_IP);	// �رն�ʱ��
		ESP8266_SNTP_Init();			// ��ʼ��SNTP
	}
	else if(S_WIFI_STA_Connect==STATION_NO_AP_FOUND 	||		// δ�ҵ�ָ��WIFI
			S_WIFI_STA_Connect==STATION_WRONG_PASSWORD 	||		// WIFI�������
			S_WIFI_STA_Connect==STATION_CONNECT_FAIL		)	// ����WIFIʧ��
	{
		os_timer_disarm(&OS_Timer_IP);			// �رն�ʱ��
		os_printf("\r\n---- ESP8266 Can't Connect to WIFI ----\r\n");
		smartconfig_set_type(SC_TYPE_AIRKISS); 	// ΢�������������ã�������ʽ��AIRKISS
		smartconfig_start(smartconfig_done_cb);	// ������������ģʽ�������ûص�����
	}
}

void ICACHE_FLASH_ATTR OS_Timer_IP_Init(u32 time_ms, u8 time_repetitive)	//�����ʱ����ʼ��
{
	os_timer_disarm(&OS_Timer_IP);	// �رն�ʱ��
	os_timer_setfn(&OS_Timer_IP, (os_timer_func_t *)OS_Timer_IP_cb, NULL);	// ���ö�ʱ��
	os_timer_arm(&OS_Timer_IP, time_ms, time_repetitive);  // ʹ�ܶ�ʱ��
}

// user_init��entry of user application, init user function here
void ICACHE_FLASH_ATTR user_init(void)
{
	delay_ms(200);						// �ȴ�LCD�ϵ��ȶ�
	system_soft_wdt_feed();				// ι��
	delay_ms(300);						// �ȴ�LCD�ϵ��ȶ�
	system_soft_wdt_feed();				// ι��
	struct station_config STA_Config;	// STA�����ṹ��
	uart_init(115200, 115200);			// ��ʼ�����ڲ�����
	os_delay_us(10000);					// �ȴ������ȶ�
	os_printf("\r\n=================================================\r\n");
	os_printf("\t SDK version:\t%s", system_get_sdk_version());
	os_printf("\r\n=================================================\r\n");

	// ESP8266��ȡ���ⲿFlash���еġ�STA������(SSID/PASS)����ΪSTA������WIFI
	os_memset(&STA_Config, 0, sizeof(struct station_config));			// STA_INFO = 0
	spi_flash_read(Sector_STA_INFO * 4096, (uint32 *)&STA_Config, 96);	// ����STA����(SSID/PASS)
	STA_Config.ssid[31] = 0;			// SSID�����'\0'
	STA_Config.password[63] = 0;		// APSS�����'\0'

	wifi_set_opmode(0x01);						// ����ΪSTAģʽ�������浽Flash
	wifi_station_set_config(&STA_Config);		// ����STA����

	LCD_Init();									// LCD��ʼ��
	LCD_Fill(0,0,LCD_W,LCD_H,SKYBLUE);

	LCD_ShowChinese(7,28,"��������",WHITE,SKYBLUE,24,0);
	LCD_ShowString(107, 28, "WiFi", WHITE, SKYBLUE, 24, 0);

	OS_Timer_IP_Init(1000, 1);					//1�������ʱ���ظ���ʱ
}


/******************************************************************************
 * FunctionName : user_rf_cal_sector_set
 * Description  : SDK just reversed 4 sectors, used for rf init data and paramters.
 *                We add this function to force users to set rf cal sector, since
 *                we don't know which sector is free in user's application.
 *                sector map for last several sectors : ABCCC
 *                A : rf cal
 *                B : rf init data
 *                C : sdk parameters
 * Parameters   : none
 * Returns      : rf cal sector
*******************************************************************************/
uint32 ICACHE_FLASH_ATTR user_rf_cal_sector_set(void)
{
    enum flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 5;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;

        case FLASH_SIZE_64M_MAP_1024_1024:
            rf_cal_sec = 2048 - 5;
            break;
        case FLASH_SIZE_128M_MAP_1024_1024:
            rf_cal_sec = 4096 - 5;
            break;
        default:
            rf_cal_sec = 0;
            break;
    }
    return rf_cal_sec;
}

void ICACHE_FLASH_ATTR user_rf_pre_init(void){}
