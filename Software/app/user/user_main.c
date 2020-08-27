#include "stdlib.h"
#include "user_config.h"		// 用户配置
#include "c_types.h"			// 变量类型
#include "eagle_soc.h"			// GPIO函数、宏定义
#include "ip_addr.h"			// 被"espconn.h"使用
#include "espconn.h"			// TCP/UDP接口
#include "ets_sys.h"			// 回调函数
#include "mem.h"				// 内存申请等函数
#include "os_type.h"			// os_XXX
#include "osapi.h"  			// os_XXX、软件定时器
#include "sntp.h"				// SNTP
#include "user_interface.h" 	// 系统接口、system_param_xxx接口、WIFI、RateContro
#include "smartconfig.h"

#include "driver/lcd_init.h"
#include "driver/lcd.h"
#include "driver/pic.h"

extern u8 locx_now, locy_now;  			//取实际像素点的值
extern u16 segColor;
extern u8 locx[];
extern u8 block_len;

u8 hour = 19, minute = 59, second = 55; //当前的时间值
u8 _hour, _minute, _second;  			//上次的时间值
u8 flag = 1;							//0:刷新动画; 1:初始化界面;
u8 _str_date[10] = {0};					//上次更新的日期数组
u8 num = 0;

#define		Sector_STA_INFO		0x90	//STA参数保存扇区

os_timer_t 	OS_Timer_IP;				//软件定时器
os_timer_t 	OS_Timer_SNTP;				//定时器_SNTP

void ICACHE_FLASH_ATTR OS_Timer_SNTP_cb(void * arg)	// SNTP定时回调函数
{
	u8 cnt = 0;					// 字符串字节计数
	u8 index = 0;				// 对应str_date下标
	uint32 TimeStamp;			// 时间戳
	char tmp[5] = {0};			// 缓存数组
	char str_clock[10] = {0};	// 时间数组
	char str_date[10] = {0};	// 日期数组
	char * p_week;
	char * p_month;
	char * p_day;
	char * p_clock;
	char * Str_RealTime;		// 实际时间的字符串

	TimeStamp = sntp_get_current_timestamp();	// 查询当前距离基准时间(1970.01.01 00:00:00 GMT+8)的时间戳(单位:秒)

	if(TimeStamp)		// 判断是否获取到偏移时间
	{
		Str_RealTime = sntp_get_real_time(TimeStamp);// 查询实际时间(GMT+8):东八区北京时间 "周 月 日 时:分:秒 年"
//		os_printf("SNTP_InternetTime = %s\n", Str_RealTime);	// 时间格式： Wed Jun 17 18:16:00 2020

		p_week 	= Str_RealTime;						// 获取"星期" 字符串的首地址
		p_month = os_strstr(p_week,	" ") + 1;		// 获取"月份" 字符串的首地址
		p_day 	= os_strstr(p_month, " ") + 1;		// 获取"日数" 字符串的首地址
		p_clock = os_strstr(p_day,	" ") + 1;		// 获取"时钟" 字符串的首地址

		//处理月份
		cnt = p_day - p_month - 1;				// 月份的字节数
		os_memcpy(tmp, p_month, cnt);
		tmp[cnt] = '\0';						// 末尾加结束符
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
		else if (strcmp(tmp, "Sept") == 0)
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

		//处理日数
		cnt = p_clock - p_day - 1;				// 日数的字节数
		os_memcpy(str_date+index, p_day, cnt);	// 将日数填入str_date数组
		if(str_date[index] == '0')				// 去掉07前的0
		{
			str_date[index] = str_date[index+1];
			index += (cnt-1);
		}
		else
			index += cnt;						// 指向空格
		str_date[index++] = ' ';				// 添加空格

		//处理星期
		cnt = p_month - p_week - 1;				// 星期的字节数
		os_memcpy(str_date+index, p_week, cnt);	// 将星期填入str_date数组
		index += cnt;							// 指向空格
		str_date[index] = '\0';

		//处理时钟
		os_memcpy(str_clock, p_clock, 8);		// 将时钟 字符串填入str_clock数组
		str_clock[8] = '\0';
		p_clock = str_clock;
		hour = atoi(p_clock);					//将时、分、秒由字符串转为整数
		minute = atoi(p_clock + 3);
		second = atoi(p_clock + 6);

		if(flag == 1)									//初始化时间显示界面
		{
			flag = 0;
			segColor = WHITE;							//画笔颜色浅蓝色
			LCD_Fill(0,0,LCD_W,LCD_H,SKYBLUE);			//清屏
			DIGIT_DrawDot();							//显示冒号
			DIGIT_DrawStartTime(hour, minute, second);	//显示初始时间
			strcpy(_str_date, str_date);
			switch(strlen(str_date))
			{
			case 7:
				LCD_ShowString(53, 2, str_date, WHITE, SKYBLUE, 24, 0);	//更新日期
				break;
			case 8:
				LCD_ShowString(47, 2, str_date, WHITE, SKYBLUE, 24, 0);	//更新日期
				break;
			case 9:
				LCD_ShowString(41, 2, str_date, WHITE, SKYBLUE, 24, 0);	//更新日期
				break;
			default:
				os_printf("\r\n---- strlen(str_date) error ----\r\n");
				break;
			}
			strcpy(_str_date, str_date);				//储存本次日期
			LCD_ShowPicture(7, 6, 20, 16, gImage_24);	//显示wifi图标
			_hour = hour;								//给上次时间值赋值
			_minute = minute;
			_second = second;
		}

		if(flag == 0)						//刷新动画
		{
			if(strcmp(_str_date, str_date))	//更新日期
			{
				switch(strlen(str_date))	//获取长度，微调居中显示日期字符串
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
				strcpy(_str_date, str_date);			//储存本次日期
			}
			if(_second != second)
			{
				block_len = 2;
				locx_now = locx[6];
				locy_now = 44;
				DIGIT_Morph(second%10, _second%10);	 	//秒个位的动画
				if (second/10 !=  _second/10)
				{
					locx_now = locx[5];
					DIGIT_Morph(second/10, _second/10); //秒十位的动画
				}
			}
			if(_minute != minute)
			{
				block_len = 3;
				locx_now = locx[4];						//定位到分钟个位位置
				locy_now = 30;
				DIGIT_Morph(minute%10, _minute%10);		//更新分钟的个位
				if (minute/10 != _minute/10)
				{
					locx_now = locx[3];
					DIGIT_Morph(minute/10, _minute/10);
				}
			}
			if(_hour != hour)							//画时
			{
				block_len = 3;
				locx_now = locx[2]; 		 			//定位到小时个位位置
				locy_now = 30;
				DIGIT_Morph(hour%10, _hour%10);	 		//更新小时的个位
				if (hour/10 != _hour/10)
				{
					locx_now = locx[1];		 			//定位到小时十位位置
					DIGIT_Morph(hour/10, _hour/10);		//更新小时的十位
				}
			}
			if(wifi_station_get_connect_status() != STATION_GOT_IP)	// 判断是否成功接入WIFI，未连接显示WIFI动画
			{
				num++;
				if(num == 3)	//3秒闪烁一次
				{
					LCD_ShowPicture(7, 6, 20, 16, gImage_21);	//显示一格信号
					delay_ms(20);
					LCD_ShowPicture(7, 6, 20, 16, gImage_22);	//显示两格信号
					delay_ms(20);
					LCD_ShowPicture(7, 6, 20, 16, gImage_23);	//显示三格信号
					delay_ms(20);
					LCD_ShowPicture(7, 6, 20, 16, gImage_24);	//显示四格信号
					delay_ms(20);
					num = 0;
				}
			}
		}
		//储存本次时间
		_hour = hour;
		_minute = minute;
		_second = second;
	}
}

void ICACHE_FLASH_ATTR OS_Timer_SNTP_Init(u32 time_ms, u8 time_repetitive)	// SNTP定时初始化
{
	os_timer_disarm(&OS_Timer_SNTP);
	os_timer_setfn(&OS_Timer_SNTP,(os_timer_func_t *)OS_Timer_SNTP_cb,NULL);
	os_timer_arm(&OS_Timer_SNTP, time_ms, time_repetitive);
}

void ICACHE_FLASH_ATTR ESP8266_SNTP_Init(void)	// 初始化SNTP，固定的程序步骤
{
	ip_addr_t * addr = (ip_addr_t *)os_zalloc(sizeof(ip_addr_t));

	sntp_setservername(0, "us.pool.ntp.org");	// 服务器_0域名
	sntp_setservername(1, "ntp.sjtu.edu.cn");	// 服务器_1域名

	ipaddr_aton("210.72.145.44", addr);			// 点分十进制 => 32位二进制
	sntp_setserver(2, addr);					// 服务器_2的IP地址
	os_free(addr);								// 释放addr

	sntp_init();								// SNTP初始化API
	OS_Timer_SNTP_Init(1000, 1);				// 1秒重复定时(SNTP)
}

void ICACHE_FLASH_ATTR smartconfig_done_cb(sc_status status, void *pdata)	// 在不同状态下，pdata传入参数是不同的
{
	os_printf("\r\n---- smartconfig_cb ----\r\n");
    switch(status)
    {
		case SC_STATUS_WAIT:				// CmartConfig等待
		break;
		case SC_STATUS_FIND_CHANNEL:		// 发现WIFI信号（8266在这种状态下等待配网）
			os_printf("\r\n---- Please Use WeChat to SmartConfig ----\r\n\r\n");
			LCD_Fill(0,0,LCD_W,LCD_H,SKYBLUE);
			LCD_ShowChinese(7,28,"请用微信配置",WHITE,SKYBLUE,24,0);
		break;
        case SC_STATUS_GETTING_SSID_PSWD:	// 正在获取SSID和PSWD，此状态下 参数2==SmartConfig类型指针
	    break;
        case SC_STATUS_LINK:	// 成功获取到SSID和PSWD，保存STA参数，并连接WIFI
        	os_printf("\r\nSC_STATUS_LINK\r\n");
            struct station_config *sta_conf = pdata;	// 获取STA参数指针，此状态下 参数2==STA参数指针
			spi_flash_erase_sector(Sector_STA_INFO);	// 擦除扇区
			spi_flash_write(Sector_STA_INFO*4096, (uint32 *)sta_conf, 96);	// 将wifi信息保存到外部Flash扇区中
	        wifi_station_set_config(sta_conf);			// 设置STA参数
	        wifi_station_disconnect();					// 断开STA连接
	        wifi_station_connect();						// ESP8266连接WIFI
	        LCD_Fill(0,0,LCD_W,LCD_H,SKYBLUE);
	    	LCD_ShowChinese(7, 28, "正在连接", WHITE, SKYBLUE, 24, 0);
	    	LCD_ShowString(107, 28, "WiFi", WHITE, SKYBLUE, 24, 0);
	    break;
        case SC_STATUS_LINK_OVER:		// ESP8266作为STA，成功连接到WIFI
            smartconfig_stop();			// 停止SmartConfig
			ESP8266_SNTP_Init();		// 初始化SNTP
			os_printf("\r\n---- ESP8266 Connect to WIFI Successfully ----\r\n");
	    break;
    }
}

void ICACHE_FLASH_ATTR OS_Timer_IP_cb(void)
{
	struct ip_info ST_ESP8266_IP;	// ESP8266的IP信息
	u8 ESP8266_IP[4];				// ESP8266的IP地址
	u8 S_WIFI_STA_Connect = wifi_station_get_connect_status();

	if(S_WIFI_STA_Connect == STATION_GOT_IP)	// 判断是否成功接入WIFI
	{
		wifi_get_ip_info(STATION_IF,&ST_ESP8266_IP);	// 获取STA的IP信息
		ESP8266_IP[0] = ST_ESP8266_IP.ip.addr;			// IP地址高八位 == addr低八位
		ESP8266_IP[1] = ST_ESP8266_IP.ip.addr>>8;		// IP地址次高八位 == addr次低八位
		ESP8266_IP[2] = ST_ESP8266_IP.ip.addr>>16;		// IP地址次低八位 == addr次高八位
		ESP8266_IP[3] = ST_ESP8266_IP.ip.addr>>24;		// IP地址低八位 == addr高八位
		os_printf("\r\n---- ESP8266_IP = %d.%d.%d.%d ----\r\n",ESP8266_IP[0],ESP8266_IP[1],ESP8266_IP[2],ESP8266_IP[3]);// 显示ESP8266的IP地址
		os_timer_disarm(&OS_Timer_IP);	// 关闭定时器
		ESP8266_SNTP_Init();			// 初始化SNTP
	}
	else if(S_WIFI_STA_Connect==STATION_NO_AP_FOUND 	||		// 未找到指定WIFI
			S_WIFI_STA_Connect==STATION_WRONG_PASSWORD 	||		// WIFI密码错误
			S_WIFI_STA_Connect==STATION_CONNECT_FAIL		)	// 连接WIFI失败
	{
		os_timer_disarm(&OS_Timer_IP);			// 关闭定时器
		os_printf("\r\n---- ESP8266 Can't Connect to WIFI ----\r\n");
		smartconfig_set_type(SC_TYPE_AIRKISS); 	// 微信智能配网设置，配网方式是AIRKISS
		smartconfig_start(smartconfig_done_cb);	// 进入智能配网模式，并设置回调函数
	}
}

void ICACHE_FLASH_ATTR OS_Timer_IP_Init(u32 time_ms, u8 time_repetitive)	//软件定时器初始化
{
	os_timer_disarm(&OS_Timer_IP);	// 关闭定时器
	os_timer_setfn(&OS_Timer_IP, (os_timer_func_t *)OS_Timer_IP_cb, NULL);	// 设置定时器
	os_timer_arm(&OS_Timer_IP, time_ms, time_repetitive);  // 使能定时器
}

// user_init：entry of user application, init user function here
void ICACHE_FLASH_ATTR user_init(void)
{
	delay_ms(200);						// 等待LCD上电稳定
	system_soft_wdt_feed();				// 喂狗
	delay_ms(300);						// 等待LCD上电稳定
	system_soft_wdt_feed();				// 喂狗
	struct station_config STA_Config;	// STA参数结构体
	uart_init(115200, 115200);			// 初始化串口波特率
	os_delay_us(10000);					// 等待串口稳定
	os_printf("\r\n=================================================\r\n");
	os_printf("\t SDK version:\t%s", system_get_sdk_version());
	os_printf("\r\n=================================================\r\n");

	// ESP8266读取【外部Flash】中的【STA参数】(SSID/PASS)，作为STA，连接WIFI
	os_memset(&STA_Config, 0, sizeof(struct station_config));			// STA_INFO = 0
	spi_flash_read(Sector_STA_INFO * 4096, (uint32 *)&STA_Config, 96);	// 读出STA参数(SSID/PASS)
	STA_Config.ssid[31] = 0;			// SSID最后添'\0'
	STA_Config.password[63] = 0;		// APSS最后添'\0'

	wifi_set_opmode(0x01);						// 设置为STA模式，并保存到Flash
	wifi_station_set_config(&STA_Config);		// 设置STA参数

	LCD_Init();									// LCD初始化
	LCD_Fill(0,0,LCD_W,LCD_H,SKYBLUE);

	LCD_ShowChinese(7,28,"正在连接",WHITE,SKYBLUE,24,0);
	LCD_ShowString(107, 28, "WiFi", WHITE, SKYBLUE, 24, 0);

	OS_Timer_IP_Init(1000, 1);					//1秒软件定时，重复定时
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
