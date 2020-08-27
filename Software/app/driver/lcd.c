#include "driver/lcdfont.h"
#include "driver/lcd_init.h"
#include "driver/lcd.h"

#define SA 		 0
#define SB 		 1
#define SC 		 2
#define SD 		 3
#define SE 		 4
#define SF 		 5
#define SG 		 6

#define SEG_WIDTH     		6       //段宽
#define SEG_HEIGHT    		6       //段高
#define ANIM_SPEED			17

u8 locx_now = 1;  					//取实际像素点的值
u8 locy_now = 5;					//取实际像素点的值
u16 segColor = WHITE;				//存储段的颜色
u8 block_len = 3;					//块的长度

u8 locx[7] = {0, 5, 32, 63, 90, 119, 138};

const u8 digitBits[] = {
  0xFC, //B11111100, 0,  ABCDEF--
  0x60, //B01100000, 1,  -BC-----
  0xDA, //B11011010, 2,  AB-DE-G-
  0xF2, //B11110010, 3,  ABCD--G-
  0x66, //B01100110, 4,  -BC--FG-
  0xB6, //B10110110, 5,  A-CD-FG-
  0xBE, //B10111110, 6,  A-CDEFG-
  0xE0, //B11100000, 7,  ABC-----
  0xFE, //B11111110, 8,  ABCDEFG-
  0xF6, //B11110110, 9,  ABCD_FG-
};

void ICACHE_FLASH_ATTR DIGIT_DrawDot(void)
{
	LCD_Fill(58, 41, 61, 44, segColor);
	LCD_Fill(58, 59, 61, 62, segColor);
}

//画一个配置好的像素块，y轴显示下移2行！
void ICACHE_FLASH_ATTR DIGIT_DrawPixelBlock(u16 x, u16 y, u16 color)	//x和y 抽象点
{
	LCD_Fill(locx_now+x, y+locy_now, locx_now+x+block_len, y+block_len+locy_now, color);		//实际点
}

//根据设置好的像素块画线
void ICACHE_FLASH_ATTR DIGIT_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2, u16 color)	//x和y是 抽象点
{
	int i;
	if (x1 == x2)
	{
		/*
		 * 画竖线:当y1的坐标比y2小时,填充矩形函数起始位置用(x1,y1);
		 * 当y1的坐标比y2大时,填充矩形函数起始位置用(x1,y2).
		 */
		if (y1 <= y2)
			for (i  = 0; i <=  y2 - y1; i++)
				DIGIT_DrawPixelBlock(x1*block_len, (y1+i)*block_len, color);	//抽象起始点, 一点变九点
		else if (y1 > y2)
			for (i  = 0; i <=  y1 - y2; i++)
				DIGIT_DrawPixelBlock(x1*block_len, (y2+i)*block_len, color);
	} else if (y1 == y2) {
		/*
		 * 画横线:当x1的坐标比x2小时,填充矩形函数起始位置用(x1,y1);
		 * 当x1的坐标比x2大时,填充矩形函数起始位置用(x2,y1).
		 */
		if (x1 <= x2)
			for (i  = 0; i <=  x2 - x1; i++)
				DIGIT_DrawPixelBlock((x1+i)*block_len, y1*block_len, color);
		else if (x1 > x2)
			for (i  = 0; i <=  x1 - x2; i++)
				DIGIT_DrawPixelBlock((x2+i)*block_len, y1*block_len, color);
	}
}

//动画1变到2
void ICACHE_FLASH_ATTR DIGIT_Morph2(void)
{
	int i;
	for (i = 0; i <= SEG_WIDTH; i++)
	{
		if (i < SEG_WIDTH)
		{
			DIGIT_DrawPixelBlock((SEG_WIDTH - i)*block_len, 0, segColor);  //画A
			DIGIT_DrawPixelBlock((SEG_WIDTH - i)*block_len, (SEG_HEIGHT + 1)*block_len, segColor);   //画G
			DIGIT_DrawPixelBlock((SEG_WIDTH - i)*block_len, (2*SEG_HEIGHT + 2)*block_len, segColor); //画D
		}
		//左平移E
		DIGIT_DrawLine(SEG_WIDTH - i + 1, SEG_HEIGHT + 2, SEG_WIDTH - i + 1, 2*SEG_HEIGHT + 1, SKYBLUE);
		DIGIT_DrawLine(SEG_WIDTH - i, SEG_HEIGHT + 2, SEG_WIDTH - i, 2*SEG_HEIGHT + 1, segColor);
		delay_ms(ANIM_SPEED);
	}
}

//动画2变到3
void ICACHE_FLASH_ATTR DIGIT_Morph3(void)
{
	int i;
	for (i = 0; i <= SEG_WIDTH; i++)
	{
		//右平移E
		DIGIT_DrawLine(i, SEG_HEIGHT + 2, i, 2*SEG_HEIGHT + 1, SKYBLUE);
		DIGIT_DrawLine(i + 1, SEG_HEIGHT + 2, i + 1, 2*SEG_HEIGHT + 1, segColor);
		delay_ms(ANIM_SPEED);
	}
}

void ICACHE_FLASH_ATTR DIGIT_Morph4(void)
{
	int i;
	for (i = 0; i < SEG_WIDTH; i++)
	{
		DIGIT_DrawPixelBlock((SEG_WIDTH - i)*block_len, 0, SKYBLUE);  					//擦A
		DIGIT_DrawPixelBlock(0, (1 + i)*block_len, segColor);							//画F
		DIGIT_DrawPixelBlock((1 + i)*block_len, (2*SEG_HEIGHT + 2)*block_len, SKYBLUE); 	//擦D
		delay_ms(ANIM_SPEED);
	}
}

void ICACHE_FLASH_ATTR DIGIT_Morph5(void)
{
	int i;
	for (i = 0; i < SEG_WIDTH; i++)
	{
		DIGIT_DrawPixelBlock((SEG_WIDTH + 1)*block_len, (SEG_HEIGHT - i)*block_len, SKYBLUE);  	//擦B
		DIGIT_DrawPixelBlock((SEG_WIDTH - i)*block_len, 0, segColor); 				 			//画A
		DIGIT_DrawPixelBlock((SEG_WIDTH - i)*block_len, (2*SEG_HEIGHT + 2)*block_len, segColor);//画D
		delay_ms(ANIM_SPEED);
	}
}

void ICACHE_FLASH_ATTR DIGIT_Morph6(void)
{
	int i;
	for (i = 0; i <= SEG_WIDTH; i++)
	{
		//左平移C
		if (i > 0)
			DIGIT_DrawLine(SEG_WIDTH - i + 1, SEG_HEIGHT + 2, SEG_WIDTH - i + 1, 2*SEG_HEIGHT + 1, SKYBLUE);
		DIGIT_DrawLine(SEG_WIDTH - i, SEG_HEIGHT + 2, SEG_WIDTH - i, 2*SEG_HEIGHT + 1, segColor);
		delay_ms(ANIM_SPEED);
	}
}

void ICACHE_FLASH_ATTR DIGIT_Morph7(void)
{
	int i;
	for (i = 0; i <= SEG_WIDTH; i++)
	{
		//右平移B
		DIGIT_DrawLine(i, 1, i, SEG_HEIGHT, SKYBLUE);
		DIGIT_DrawLine(i + 1, 1, i + 1, SEG_HEIGHT, segColor);

		//右平移E
		DIGIT_DrawLine(i, SEG_HEIGHT + 2, i, 2*SEG_HEIGHT + 1, SKYBLUE);
		DIGIT_DrawLine(i + 1, SEG_HEIGHT + 2, i + 1, 2*SEG_HEIGHT + 1, segColor);

		DIGIT_DrawPixelBlock((1 + i)*block_len, (SEG_HEIGHT + 1)*block_len, SKYBLUE);  	//擦G
		DIGIT_DrawPixelBlock((1 + i)*block_len, (2*SEG_HEIGHT + 2)*block_len, SKYBLUE);   //擦D

		delay_ms(ANIM_SPEED);
	}
}

void ICACHE_FLASH_ATTR DIGIT_Morph8(void)
{
	int i;
	for (i = 0; i <= SEG_WIDTH; i++)
	{
		//左平移B
		if (i > 0)
			DIGIT_DrawLine(SEG_WIDTH - i + 1, 1, SEG_WIDTH - i + 1, SEG_HEIGHT, SKYBLUE);
		DIGIT_DrawLine(SEG_WIDTH - i, 1, SEG_WIDTH - i, SEG_HEIGHT, segColor);

		//左平移C
		if (i > 0)
			DIGIT_DrawLine(SEG_WIDTH - i + 1, SEG_HEIGHT + 2, SEG_WIDTH - i + 1, 2*SEG_HEIGHT + 1, SKYBLUE);
		DIGIT_DrawLine(SEG_WIDTH - i, SEG_HEIGHT + 2, SEG_WIDTH - i, 2*SEG_HEIGHT + 1, segColor);

		if (i < SEG_WIDTH)
		{
			DIGIT_DrawPixelBlock((SEG_WIDTH - i)*block_len, (SEG_HEIGHT + 1)*block_len, segColor); 	 //画G
			DIGIT_DrawPixelBlock((SEG_WIDTH - i)*block_len, (2*SEG_HEIGHT + 2)*block_len, segColor); //画D
		}
		delay_ms(ANIM_SPEED);
	}
}

void ICACHE_FLASH_ATTR DIGIT_Morph9(void)
{
	int i;
	for (i = 0; i <= SEG_WIDTH; i++)
	{
		//右平移E
		DIGIT_DrawLine(i, SEG_HEIGHT + 2, i, 2*SEG_HEIGHT + 1, SKYBLUE);
		DIGIT_DrawLine(i + 1, SEG_HEIGHT + 2, i + 1, 2*SEG_HEIGHT + 1, segColor);
		delay_ms(ANIM_SPEED);
	}
}

//_value为上一次的数字值，根据上次的数字值来决定此次的动画效果
void ICACHE_FLASH_ATTR DIGIT_Morph0(u8 _value)
{
	int i;
	for (i = 0; i <= SEG_WIDTH; i++)
	{
		if (_value == 1)
		{
			//左平移B
			if (i > 0)
				DIGIT_DrawLine(SEG_WIDTH - i + 1, 1, SEG_WIDTH - i + 1, SEG_HEIGHT, SKYBLUE);
			DIGIT_DrawLine(SEG_WIDTH - i, 1, SEG_WIDTH - i, SEG_HEIGHT, segColor);

			//左平移C
			if (i > 0)
				DIGIT_DrawLine(SEG_WIDTH - i + 1, SEG_HEIGHT + 2, SEG_WIDTH - i + 1, 2*SEG_HEIGHT + 1, SKYBLUE);
			DIGIT_DrawLine(SEG_WIDTH - i, SEG_HEIGHT + 2, SEG_WIDTH - i, 2*SEG_HEIGHT + 1, segColor);

			if (i < SEG_WIDTH)
			{
				DIGIT_DrawPixelBlock((SEG_WIDTH - i)*block_len, 0, segColor); 							//画A
				DIGIT_DrawPixelBlock((SEG_WIDTH - i)*block_len, (2*SEG_HEIGHT + 2)*block_len, segColor);//画D
			}
		}

		if (_value == 2)
		{
			//左平移B
			if (i > 0)
				DIGIT_DrawLine(SEG_WIDTH - i + 1, 1, SEG_WIDTH - i + 1, SEG_HEIGHT, SKYBLUE);
			DIGIT_DrawLine(SEG_WIDTH - i, 1, SEG_WIDTH - i, SEG_HEIGHT, segColor);

			//右平移E
			if (i > 0)
				DIGIT_DrawLine(i, SEG_HEIGHT + 2, i, 2*SEG_HEIGHT + 1, SKYBLUE);
			DIGIT_DrawLine(i + 1, SEG_HEIGHT + 2, i + 1, 2*SEG_HEIGHT + 1, segColor);

			if (i < SEG_WIDTH)
			{
				DIGIT_DrawPixelBlock((i + 1)*block_len, (SEG_HEIGHT + 1)*block_len, SKYBLUE); //擦G
			}
		}

		if (_value == 3)
		{
			//左平移B
			if (i > 0)
				DIGIT_DrawLine(SEG_WIDTH - i + 1, 1, SEG_WIDTH - i + 1, SEG_HEIGHT, SKYBLUE);
			DIGIT_DrawLine(SEG_WIDTH - i, 1, SEG_WIDTH - i, SEG_HEIGHT, segColor);

			//左平移C
			if (i > 0)
				DIGIT_DrawLine(SEG_WIDTH - i + 1, SEG_HEIGHT + 2, SEG_WIDTH - i + 1, 2*SEG_HEIGHT + 1, SKYBLUE);
			DIGIT_DrawLine(SEG_WIDTH - i, SEG_HEIGHT + 2, SEG_WIDTH - i, 2*SEG_HEIGHT + 1, segColor);

			if (i < SEG_WIDTH)
			{
				DIGIT_DrawPixelBlock((SEG_WIDTH - i)*block_len, (SEG_HEIGHT + 1)*block_len, SKYBLUE);  //擦G
			}
		}

		if (_value == 5)
		{
			if (i <= SEG_WIDTH)
			{
				//右平移F
				if (i > 0)
					DIGIT_DrawLine(i, 1, i, SEG_HEIGHT, SKYBLUE);
				DIGIT_DrawLine(i + 1, 1, i + 1, SEG_HEIGHT, segColor);
			}
		}

		if (_value == 5 || _value == 9)
		{
			if (i < SEG_WIDTH)
			{
				DIGIT_DrawPixelBlock((SEG_WIDTH - i)*block_len, (SEG_HEIGHT + 1)*block_len, SKYBLUE); //擦G
				DIGIT_DrawPixelBlock(0, (SEG_HEIGHT + 2 + i)*block_len, segColor); 		//画E
			}
		}
		delay_ms(ANIM_SPEED);
	}
}

void ICACHE_FLASH_ATTR DIGIT_Morph1(void)
{
	int i;
	for (i = 0; i <= SEG_WIDTH; i++)
	{
		//右平移F
		DIGIT_DrawLine(i, 1, i, SEG_HEIGHT, SKYBLUE);
		DIGIT_DrawLine(i + 1, 1, i + 1, SEG_HEIGHT, segColor);

		//右平移E
		DIGIT_DrawLine(i, SEG_HEIGHT + 2, i, 2*SEG_HEIGHT + 1, SKYBLUE);
		DIGIT_DrawLine(i + 1, SEG_HEIGHT + 2, i + 1, 2*SEG_HEIGHT + 1, segColor);

		if (i < SEG_WIDTH)
		{
			DIGIT_DrawPixelBlock((i + 1)*block_len, 0, SKYBLUE);  			   				//擦A
			DIGIT_DrawPixelBlock((i + 1)*block_len, (SEG_HEIGHT + 1)*block_len, SKYBLUE);   //擦G
			DIGIT_DrawPixelBlock((i + 1)*block_len, (2*SEG_HEIGHT + 2)*block_len, SKYBLUE); //擦D
		}
		delay_ms(ANIM_SPEED);
	}
}

//画数字的某一段，数字分为7段，seg为段号
void ICACHE_FLASH_ATTR DIGIT_DrawSeg(u8 seg)
{
	switch (seg)
	{
		case SA: DIGIT_DrawLine(1, 0, SEG_WIDTH, 0, segColor); break;
		case SB: DIGIT_DrawLine(SEG_WIDTH+1, 1, SEG_WIDTH+1, SEG_HEIGHT, segColor); break;
		case SC: DIGIT_DrawLine(SEG_WIDTH+1, SEG_HEIGHT+2, SEG_WIDTH+1, 2*SEG_HEIGHT+1, segColor); break;
		case SD: DIGIT_DrawLine(1, 2*SEG_HEIGHT+2, SEG_WIDTH, 2*SEG_HEIGHT+2, segColor); break;
		case SE: DIGIT_DrawLine(0, SEG_HEIGHT+2, 0, 2*SEG_HEIGHT+1, segColor); break;
		case SF: DIGIT_DrawLine(0, 1, 0, SEG_HEIGHT, segColor); break;
		case SG: DIGIT_DrawLine(1, SEG_HEIGHT+1, SEG_WIDTH, SEG_HEIGHT+1, segColor); break;
		default: break;
	}
}

//画数字
void ICACHE_FLASH_ATTR DIGIT_DrawDigit(u8 num)  //locx是起始位置
{
	u8 value;
	if (num > 9)  //不在允许范围之内则退出
		return;
	value = digitBits[num];
	if (value & 0x80)	DIGIT_DrawSeg(SA);
	if (value & 0x40)	DIGIT_DrawSeg(SB);
	if (value & 0x20)	DIGIT_DrawSeg(SC);
	if (value & 0x10)	DIGIT_DrawSeg(SD);
	if (value & 0x08)	DIGIT_DrawSeg(SE);
	if (value & 0x04)	DIGIT_DrawSeg(SF);
	if (value & 0x02)	DIGIT_DrawSeg(SG);
}

/* 作用: 数字变换
 * 参数: value->当前的数字值
 *		 _value->上次的数字值
 * 返回值: none
 */
void ICACHE_FLASH_ATTR DIGIT_Morph(u8 value, u8 _value)
{
	switch (value)
	{
		case 0: DIGIT_Morph0(_value); break;
		case 1: DIGIT_Morph1(); break;
		case 2: DIGIT_Morph2(); break;
		case 3: DIGIT_Morph3(); break;
		case 4: DIGIT_Morph4(); break;
		case 5: DIGIT_Morph5(); break;
		case 6: DIGIT_Morph6(); break;
		case 7: DIGIT_Morph7(); break;
		case 8: DIGIT_Morph8(); break;
		case 9: DIGIT_Morph9(); break;
	}
}

void ICACHE_FLASH_ATTR DIGIT_DrawStartTime(u8 hour, u8 minute, u8 second)
{
	block_len = 2;
	locx_now = locx[6];			 //定位
	locy_now = 44;
	DIGIT_DrawDigit(second%10);  //画秒的个位
	locx_now = locx[5];			 //定位
	DIGIT_DrawDigit(second/10);  //画秒的十位

	block_len = 3;
	locx_now = locx[4];			 //定位
	locy_now = 30;
	DIGIT_DrawDigit(minute%10);  //画分的个位
	locx_now = locx[3];		 	 //定位
	DIGIT_DrawDigit(minute/10);  //画分的十位
	locx_now = locx[2];			 //定位
	DIGIT_DrawDigit(hour%10);    //画时的个位
	locx_now = locx[1];			 //定位
	DIGIT_DrawDigit(hour/10);    //画时的十位
}

/******************************************************************************
      函数说明：在指定区域填充颜色
      入口数据：xsta,ysta   起始坐标
                xend,yend   终止坐标
								color       要填充的颜色
      返回值：  无
******************************************************************************/
void ICACHE_FLASH_ATTR LCD_Fill(u16 xsta,u16 ysta,u16 xend,u16 yend,u16 color)
{
	u16 i, j;
	LCD_Address_Set(xsta, ysta, xend-1, yend-1);//设置显示范围，不包括(xend,yend)在内显示！！！
	for(i = ysta; i < yend; i++)
	{
		for(j = xsta; j < xend; j++)
		{
			LCD_WR_DATA(color);
		}
	}
}

/******************************************************************************
      函数说明：在指定位置画点
      入口数据：x,y 画点坐标
                color 点的颜色
      返回值：  无
******************************************************************************/
void ICACHE_FLASH_ATTR LCD_DrawPoint(u16 x,u16 y,u16 color)
{
	LCD_Address_Set(x,y,x,y);//设置光标位置
	LCD_WR_DATA(color);
}


/******************************************************************************
      函数说明：画线
      入口数据：x1,y1   起始坐标
                x2,y2   终止坐标
                color   线的颜色
      返回值：  无
******************************************************************************/
void ICACHE_FLASH_ATTR LCD_DrawLine(u16 x1,u16 y1,u16 x2,u16 y2,u16 color)
{
	u16 t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance;
	int incx,incy,uRow,uCol;
	delta_x=x2-x1; //计算坐标增量 
	delta_y=y2-y1;
	uRow=x1;//画线起点坐标
	uCol=y1;
	if(delta_x>0)incx=1; //设置单步方向 
	else if (delta_x==0)incx=0;//垂直线 
	else {incx=-1;delta_x=-delta_x;}
	if(delta_y>0)incy=1;
	else if (delta_y==0)incy=0;//水平线 
	else {incy=-1;delta_y=-delta_x;}
	if(delta_x>delta_y)distance=delta_x; //选取基本增量坐标轴 
	else distance=delta_y;
	for(t=0;t<distance+1;t++)
	{
		LCD_DrawPoint(uRow,uCol,color);//画点
		xerr+=delta_x;
		yerr+=delta_y;
		if(xerr>distance)
		{
			xerr-=distance;
			uRow+=incx;
		}
		if(yerr>distance)
		{
			yerr-=distance;
			uCol+=incy;
		}
	}
}


/******************************************************************************
      函数说明：画矩形
      入口数据：x1,y1   起始坐标
                x2,y2   终止坐标
                color   矩形的颜色
      返回值：  无
******************************************************************************/
void ICACHE_FLASH_ATTR LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2,u16 color)
{
	LCD_DrawLine(x1,y1,x2,y1,color);
	LCD_DrawLine(x1,y1,x1,y2,color);
	LCD_DrawLine(x1,y2,x2,y2,color);
	LCD_DrawLine(x2,y1,x2,y2,color);
}


/******************************************************************************
      函数说明：画圆
      入口数据：x0,y0   圆心坐标
                r       半径
                color   圆的颜色
      返回值：  无
******************************************************************************/
void ICACHE_FLASH_ATTR Draw_Circle(u16 x0,u16 y0,u8 r,u16 color)
{
	int a,b;
	a=0;b=r;	  
	while(a<=b)
	{
		LCD_DrawPoint(x0-b,y0-a,color);             //3           
		LCD_DrawPoint(x0+b,y0-a,color);             //0           
		LCD_DrawPoint(x0-a,y0+b,color);             //1                
		LCD_DrawPoint(x0-a,y0-b,color);             //2             
		LCD_DrawPoint(x0+b,y0+a,color);             //4               
		LCD_DrawPoint(x0+a,y0-b,color);             //5
		LCD_DrawPoint(x0+a,y0+b,color);             //6 
		LCD_DrawPoint(x0-b,y0+a,color);             //7
		a++;
		if((a*a+b*b)>(r*r))//判断要画的点是否过远
		{
			b--;
		}
	}
}

/******************************************************************************
      函数说明：显示汉字串
      入口数据：x,y显示坐标
                *s 要显示的汉字串
                fc 字的颜色
                bc 字的背景色
                sizey 字号 可选 16 24 32
                mode:  0非叠加模式  1叠加模式
      返回值：  无
******************************************************************************/
void ICACHE_FLASH_ATTR LCD_ShowChinese(u16 x,u16 y,u8 *s,u16 fc,u16 bc,u8 sizey,u8 mode)
{
	while(*s!=0)
	{
		if(sizey==16) LCD_ShowChinese16x16(x,y,s,fc,bc,sizey,mode);
		else if(sizey==24) LCD_ShowChinese24x24(x,y,s,fc,bc,sizey,mode);
		else if(sizey==32) LCD_ShowChinese32x32(x,y,s,fc,bc,sizey,mode);
		else return;
		s+=2;
		x+=sizey;
	}
}

/******************************************************************************
      函数说明：显示单个16x16汉字
      入口数据：x,y显示坐标
                *s 要显示的汉字
                fc 字的颜色
                bc 字的背景色
                sizey 字号
                mode:  0非叠加模式  1叠加模式
      返回值：  无
******************************************************************************/
void ICACHE_FLASH_ATTR LCD_ShowChinese16x16(u16 x,u16 y,u8 *s,u16 fc,u16 bc,u8 sizey,u8 mode)
{
	u8 i,j;
	u16 k;
	u16 HZnum;//汉字数目
	u16 TypefaceNum;//一个字符所占字节大小
	u16 x0=x;
	TypefaceNum=sizey/8*sizey;//此算法只适用于字宽等于字高，且字高是8的倍数的字，
	                          //也建议用户使用这样大小的字,否则显示容易出问题！
	HZnum=sizeof(tfont16)/sizeof(typFNT_GB16);	//统计汉字数目
	for(k=0;k<HZnum;k++) 
	{
		if ((tfont16[k].Index[0]==*(s))&&(tfont16[k].Index[1]==*(s+1)))
		{ 	
			LCD_Address_Set(x,y,x+sizey-1,y+sizey-1);
			for(i=0;i<TypefaceNum;i++)
			{
				for(j=0;j<8;j++)
				{	
					if(!mode)//非叠加方式
					{
						if(tfont16[k].Msk[i]&(0x01<<j))LCD_WR_DATA(fc);
						else LCD_WR_DATA(bc);
					}
					else//叠加方式
					{
						if(tfont16[k].Msk[i]&(0x01<<j))	LCD_DrawPoint(x,y,fc);//画一个点
						x++;
						if((x-x0)==sizey)
						{
							x=x0;
							y++;
							break;
						}
					}
				}
			}
		}				  	
		continue;  //查找到对应点阵字库立即退出，防止多个汉字重复取模带来影响
	}
} 


/******************************************************************************
      函数说明：显示单个24x24汉字
      入口数据：x,y显示坐标
                *s 要显示的汉字
                fc 字的颜色
                bc 字的背景色
                sizey 字号
                mode:  0非叠加模式  1叠加模式
      返回值：  无
******************************************************************************/
void ICACHE_FLASH_ATTR LCD_ShowChinese24x24(u16 x,u16 y,u8 *s,u16 fc,u16 bc,u8 sizey,u8 mode)
{
	u8 i,j;
	u16 k;
	u16 HZnum;//汉字数目
	u16 TypefaceNum;//一个字符所占字节大小
	u16 x0=x;
	TypefaceNum=sizey/8*sizey;//此算法只适用于字宽等于字高，且字高是8的倍数的字，
	                          //也建议用户使用这样大小的字,否则显示容易出问题！
	HZnum=sizeof(tfont24)/sizeof(typFNT_GB24);	//统计汉字数目
	for(k=0;k<HZnum;k++) 
	{
		if ((tfont24[k].Index[0]==*(s))&&(tfont24[k].Index[1]==*(s+1)))
		{ 	
			LCD_Address_Set(x,y,x+sizey-1,y+sizey-1);
			for(i=0;i<TypefaceNum;i++)
			{
				for(j=0;j<8;j++)
				{	
					if(!mode)//非叠加方式
					{
						if(tfont24[k].Msk[i]&(0x01<<j))LCD_WR_DATA(fc);
						else LCD_WR_DATA(bc);
					}
					else//叠加方式
					{
						if(tfont24[k].Msk[i]&(0x01<<j))	LCD_DrawPoint(x,y,fc);//画一个点
						x++;
						if((x-x0)==sizey)
						{
							x=x0;
							y++;
							break;
						}
					}
				}
			}
		}				  	
		continue;  //查找到对应点阵字库立即退出，防止多个汉字重复取模带来影响
	}
} 

/******************************************************************************
      函数说明：显示单个32x32汉字
      入口数据：x,y显示坐标
                *s 要显示的汉字
                fc 字的颜色
                bc 字的背景色
                sizey 字号
                mode:  0非叠加模式  1叠加模式
      返回值：  无
******************************************************************************/
void ICACHE_FLASH_ATTR LCD_ShowChinese32x32(u16 x,u16 y,u8 *s,u16 fc,u16 bc,u8 sizey,u8 mode)
{
	u8 i,j;
	u16 k;
	u16 HZnum;//汉字数目
	u16 TypefaceNum;//一个字符所占字节大小
	u16 x0=x;
	TypefaceNum=sizey/8*sizey;//此算法只适用于字宽等于字高，且字高是8的倍数的字，
	                          //也建议用户使用这样大小的字,否则显示容易出问题！
	HZnum=sizeof(tfont32)/sizeof(typFNT_GB32);	//统计汉字数目
	for(k=0;k<HZnum;k++) 
	{
		if ((tfont32[k].Index[0]==*(s))&&(tfont32[k].Index[1]==*(s+1)))
		{ 	
			LCD_Address_Set(x,y,x+sizey-1,y+sizey-1);
			for(i=0;i<TypefaceNum;i++)
			{
				for(j=0;j<8;j++)
				{	
					if(!mode)//非叠加方式
					{
						if(tfont32[k].Msk[i]&(0x01<<j))LCD_WR_DATA(fc);
						else LCD_WR_DATA(bc);
					}
					else//叠加方式
					{
						if(tfont32[k].Msk[i]&(0x01<<j))	LCD_DrawPoint(x,y,fc);//画一个点
						x++;
						if((x-x0)==sizey)
						{
							x=x0;
							y++;
							break;
						}
					}
				}
			}
		}				  	
		continue;  //查找到对应点阵字库立即退出，防止多个汉字重复取模带来影响
	}
}


/******************************************************************************
      函数说明：显示单个字符
      入口数据：x,y显示坐标
                num 要显示的字符
                fc 字的颜色
                bc 字的背景色
                sizey 字号
                mode:  0非叠加模式  1叠加模式
      返回值：  无
******************************************************************************/
void ICACHE_FLASH_ATTR LCD_ShowChar(u16 x,u16 y,u8 num,u16 fc,u16 bc,u8 sizey,u8 mode)
{
	u8 temp,sizex,t;
	u16 i, TypefaceNum;//一个字符所占字节大小
	u16 x0 = x;
	sizex=sizey/2;
	TypefaceNum=sizex/8*sizey;
	num=num-' ';    //得到偏移后的值
	LCD_Address_Set(x,y,x+sizex-1,y+sizey-1);  //设置光标位置 
	for(i=0;i<TypefaceNum;i++)
	{ 
		if(sizey==16)
			temp = ascii_1608[num][i];		       //调用8x16字体
		else if(sizey==32)
			temp = ascii_3216[num][i];		 //调用16x32字体
		else return;
		for(t=0;t<8;t++)
		{
			if(!mode)//非叠加模式
			{
				if(temp&(0x01<<t))LCD_WR_DATA(fc);
				else LCD_WR_DATA(bc);
			}
			else//叠加模式
			{
				if(temp&(0x01<<t))LCD_DrawPoint(x,y,fc);//画一个点
				x++;
				if((x-x0)==sizex)
				{
					x=x0;
					y++;
					break;
				}
			}
		}
	}   	 	  
}

//只用于显示24x12字体
void ICACHE_FLASH_ATTR MY_LCD_ShowChar(u16 x,u16 y,u8 num,u16 fc,u16 bc,u8 sizey,u8 mode)
{
	u8 temp, sizex, t, i;
	sizex = sizey/2 + 4;	//x长度设为12+4=16
	num=num-' ';    		//得到偏移后的值
	LCD_Address_Set(x, y, x+sizex-1, y+sizey-1);  //设置光标位置
	for(i = 0; i < 48; i++)			//字模中48个字节
	{
		temp = ascii_2412[num][i];	//调用24x12楷体
		for(t = 0; t < 8; t++)
		{
			if(temp&(0x01<<t))
				LCD_WR_DATA(fc);
			else
				LCD_WR_DATA(bc);
		}
	}
}

/******************************************************************************
      函数说明：显示字符串
      入口数据：x,y显示坐标
                *p 要显示的字符串
                fc 字的颜色
                bc 字的背景色
                sizey 字号
                mode:  0非叠加模式  1叠加模式
      返回值：  无
******************************************************************************/
void ICACHE_FLASH_ATTR LCD_ShowString(u16 x,u16 y,const u8 *p,u16 fc,u16 bc,u8 sizey,u8 mode)
{         
	while(*p != '\0')
	{       
//		LCD_ShowChar(x,y,*p,fc,bc,sizey,mode);
		MY_LCD_ShowChar(x,y,*p,fc,bc,sizey,mode);
		x += sizey / 2;
		p++;
	}  
}


/******************************************************************************
      函数说明：显示数字
      入口数据：m底数，n指数
      返回值：  无
******************************************************************************/
u32 ICACHE_FLASH_ATTR mypow(u8 m,u8 n)
{
	u32 result=1;	 
	while(n--)result*=m;
	return result;
}


/******************************************************************************
      函数说明：显示整数变量
      入口数据：x,y显示坐标
                num 要显示整数变量
                len 要显示的位数
                fc 字的颜色
                bc 字的背景色
                sizey 字号
      返回值：  无
******************************************************************************/
void ICACHE_FLASH_ATTR LCD_ShowIntNum(u16 x,u16 y,u16 num,u8 len,u16 fc,u16 bc,u8 sizey)
{         	
	u8 t,temp;
	u8 enshow=0;
	u8 sizex=sizey/2;
	for(t=0;t<len;t++)
	{
		temp=(num/mypow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				LCD_ShowChar(x+t*sizex,y,' ',fc,bc,sizey,0);
				continue;
			}else enshow=1; 
		 	 
		}
	 	LCD_ShowChar(x+t*sizex,y,temp+48,fc,bc,sizey,0);
	}
} 


/******************************************************************************
      函数说明：显示两位小数变量
      入口数据：x,y显示坐标
                num 要显示小数变量
                len 要显示的位数
                fc 字的颜色
                bc 字的背景色
                sizey 字号
      返回值：  无
******************************************************************************/
void ICACHE_FLASH_ATTR LCD_ShowFloatNum1(u16 x,u16 y,float num,u8 len,u16 fc,u16 bc,u8 sizey)
{         	
	u8 t,temp,sizex;
	u16 num1;
	sizex=sizey/2;
	num1=num*100;
	for(t=0;t<len;t++)
	{
		temp=(num1/mypow(10,len-t-1))%10;
		if(t==(len-2))
		{
			LCD_ShowChar(x+(len-2)*sizex,y,'.',fc,bc,sizey,0);
			t++;
			len+=1;
		}
	 	LCD_ShowChar(x+t*sizex,y,temp+48,fc,bc,sizey,0);
	}
}


/******************************************************************************
      函数说明：显示图片
      入口数据：x,y起点坐标
                length 图片长度
                width  图片宽度
                pic[]  图片数组    
      返回值：  无
******************************************************************************/
void ICACHE_FLASH_ATTR LCD_ShowPicture(u16 x,u16 y,u16 length,u16 width,const u8 pic[])
{
	u16 i,j,k=0;
	LCD_Address_Set(x,y,x+length-1,y+width-1);
	for(i=0;i<length;i++)
	{
		for(j=0;j<width;j++)
		{
			LCD_WR_DATA8(pic[k*2]);
			LCD_WR_DATA8(pic[k*2+1]);
			k++;
		}
	}			
}


