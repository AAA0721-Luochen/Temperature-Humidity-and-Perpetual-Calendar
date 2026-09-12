#include "stm32f10x.h"
#include "Lcd_Driver.h"
#include "GUI.h"
#include "Font.h"

//从ILI93xx读出的数据为GBR格式，而我们写入的时候为RGB格式。
//通过该函数转换
//c:GBR格式的颜色值
//返回值：RGB格式的颜色值
u16 LCD_BGR2RGB(u16 c)
{
  u16  r,g,b,rgb;   
  b=(c>>0)&0x1f;
  g=(c>>5)&0x3f;
  r=(c>>11)&0x1f;	 
  rgb=(b<<11)+(g<<5)+(r<<0);		 
  return(rgb);

}




void Gui_Circle(u16 X,u16 Y,u16 R,u16 fc) 
{//Bresenham算法 
    unsigned short  a,b; 
    int c; 
    a=0; 
    b=R; 
    c=3-2*R; 
    while (a<b) 
    { 
        Gui_DrawPoint(X+a,Y+b,fc);     //        7 
        Gui_DrawPoint(X-a,Y+b,fc);     //        6 
        Gui_DrawPoint(X+a,Y-b,fc);     //        2 
        Gui_DrawPoint(X-a,Y-b,fc);     //        3 
        Gui_DrawPoint(X+b,Y+a,fc);     //        8 
        Gui_DrawPoint(X-b,Y+a,fc);     //        5 
        Gui_DrawPoint(X+b,Y-a,fc);     //        1 
        Gui_DrawPoint(X-b,Y-a,fc);     //        4 

        if(c<0) c=c+4*a+6; 
        else 
        { 
            c=c+4*(a-b)+10; 
            b-=1; 
        } 
       a+=1; 
    } 
    if (a==b) 
    { 
        Gui_DrawPoint(X+a,Y+b,fc); 
        Gui_DrawPoint(X+a,Y+b,fc); 
        Gui_DrawPoint(X+a,Y-b,fc); 
        Gui_DrawPoint(X-a,Y-b,fc); 
        Gui_DrawPoint(X+b,Y+a,fc); 
        Gui_DrawPoint(X-b,Y+a,fc); 
        Gui_DrawPoint(X+b,Y-a,fc); 
        Gui_DrawPoint(X-b,Y-a,fc); 
    } 
	
} 
//画线函数，使用Bresenham 画线算法
void Gui_DrawLine(u16 x0, u16 y0,u16 x1, u16 y1,u16 Color)   
{
int dx,             // difference in x's
    dy,             // difference in y's
    dx2,            // dx,dy * 2
    dy2, 
    x_inc,          // amount in pixel space to move during drawing
    y_inc,          // amount in pixel space to move during drawing
    error,          // the discriminant i.e. error i.e. decision variable
    index;          // used for looping	


	Lcd_SetXY(x0,y0);
	dx = x1-x0;//计算x距离
	dy = y1-y0;//计算y距离

	if (dx>=0)
	{
		x_inc = 1;
	}
	else
	{
		x_inc = -1;
		dx    = -dx;  
	} 
	
	if (dy>=0)
	{
		y_inc = 1;
	} 
	else
	{
		y_inc = -1;
		dy    = -dy; 
	} 

	dx2 = dx << 1;
	dy2 = dy << 1;

	if (dx > dy)//x距离大于y距离，那么每个x轴上只有一个点，每个y轴上有若干个点
	{//且线的点数等于x距离，以x轴递增画点
		// initialize error term
		error = dy2 - dx; 

		// draw the line
		for (index=0; index <= dx; index++)//要画的点数不会超过x距离
		{
			//画点
			Gui_DrawPoint(x0,y0,Color);
			
			// test if error has overflowed
			if (error >= 0) //是否需要增加y坐标值
			{
				error-=dx2;

				// move to next line
				y0+=y_inc;//增加y坐标值
			} // end if error overflowed

			// adjust the error term
			error+=dy2;

			// move to the next pixel
			x0+=x_inc;//x坐标值每次画点后都递增1
		} // end for
	} // end if |slope| <= 1
	else//y轴大于x轴，则每个y轴上只有一个点，x轴若干个点
	{//以y轴为递增画点
		// initialize error term
		error = dx2 - dy; 

		// draw the line
		for (index=0; index <= dy; index++)
		{
			// set the pixel
			Gui_DrawPoint(x0,y0,Color);

			// test if error overflowed
			if (error >= 0)
			{
				error-=dy2;

				// move to next line
				x0+=x_inc;
			} // end if error overflowed

			// adjust the error term
			error+=dx2;

			// move to the next pixel
			y0+=y_inc;
		} // end for
	} // end else |slope| > 1
}



void Gui_box(u16 x, u16 y, u16 w, u16 h,u16 bc)
{
	Gui_DrawLine(x,y,x+w,y,0xEF7D);
	Gui_DrawLine(x+w-1,y+1,x+w-1,y+1+h,0x2965);
	Gui_DrawLine(x,y+h,x+w,y+h,0x2965);
	Gui_DrawLine(x,y,x,y+h,0xEF7D);
    Gui_DrawLine(x+1,y+1,x+1+w-2,y+1+h-2,bc);
}
void Gui_box2(u16 x,u16 y,u16 w,u16 h, u8 mode)
{
	if (mode==0)	{
		Gui_DrawLine(x,y,x+w,y,0xEF7D);
		Gui_DrawLine(x+w-1,y+1,x+w-1,y+1+h,0x2965);
		Gui_DrawLine(x,y+h,x+w,y+h,0x2965);
		Gui_DrawLine(x,y,x,y+h,0xEF7D);
		}
	if (mode==1)	{
		Gui_DrawLine(x,y,x+w,y,0x2965);
		Gui_DrawLine(x+w-1,y+1,x+w-1,y+1+h,0xEF7D);
		Gui_DrawLine(x,y+h,x+w,y+h,0xEF7D);
		Gui_DrawLine(x,y,x,y+h,0x2965);
	}
	if (mode==2)	{
		Gui_DrawLine(x,y,x+w,y,0xffff);
		Gui_DrawLine(x+w-1,y+1,x+w-1,y+1+h,0xffff);
		Gui_DrawLine(x,y+h,x+w,y+h,0xffff);
		Gui_DrawLine(x,y,x,y+h,0xffff);
	}
}


/**************************************************************************************
功能描述: 在屏幕显示一凸起的按钮框
输    入: u16 x1,y1,x2,y2 按钮框左上角和右下角坐标
输    出: 无
**************************************************************************************/
void DisplayButtonDown(u16 x1,u16 y1,u16 x2,u16 y2)
{
	Gui_DrawLine(x1,  y1,  x2,y1, GRAY2);  //H
	Gui_DrawLine(x1+1,y1+1,x2,y1+1, GRAY1);  //H
	Gui_DrawLine(x1,  y1,  x1,y2, GRAY2);  //V
	Gui_DrawLine(x1+1,y1+1,x1+1,y2, GRAY1);  //V
	Gui_DrawLine(x1,  y2,  x2,y2, WHITE);  //H
	Gui_DrawLine(x2,  y1,  x2,y2, WHITE);  //V
}

/**************************************************************************************
功能描述: 在屏幕显示一凹下的按钮框
输    入: u16 x1,y1,x2,y2 按钮框左上角和右下角坐标
输    出: 无
**************************************************************************************/
void DisplayButtonUp(u16 x1,u16 y1,u16 x2,u16 y2)
{
	Gui_DrawLine(x1,  y1,  x2,y1, WHITE); //H
	Gui_DrawLine(x1,  y1,  x1,y2, WHITE); //V
	
	Gui_DrawLine(x1+1,y2-1,x2,y2-1, GRAY1);  //H
	Gui_DrawLine(x1,  y2,  x2,y2, GRAY2);  //H
	Gui_DrawLine(x2-1,y1+1,x2-1,y2, GRAY1);  //V
    Gui_DrawLine(x2  ,y1  ,x2,y2, GRAY2); //V
}

/**************************************************************************************
功能描述: 数码管数字
输    入: x、y为起点坐标，fc为文字颜色，bc为背景，s为数字
输    出: 无
**************************************************************************************/

void Gui_DrawFont_Num32(u16 x, u16 y, u16 fc, u16 bc, u16 num)
{
	unsigned char i,j,k,c;
	//lcd_text_any(x+94+i*42,y+34,32,32,0x7E8,0x0,sz32,knum[i]);
//	w=w/8;

    for(i=0;i<32;i++)
	{
		for(j=0;j<4;j++) 
		{
			c=*(sz32+num*32*4+i*4+j);
			for (k=0;k<8;k++)	
			{
	
		    	if(c&(0x80>>k))	Gui_DrawPoint(x+j*8+k,y+i,fc);
				else {
					if (fc!=bc) Gui_DrawPoint(x+j*8+k,y+i,bc);
				}
			}
		}
	}
}

#define GUI_SMALL_NUM_WIDTH       12U
#define GUI_SMALL_NUM_HEIGHT      24U
#define GUI_FONT24_SOURCE_SIZE    24U
#define GUI_NUM32_SOURCE_SIZE     32U
#define GUI_NUM32_GLYPH_BYTES     128U
#define GUI_NUM_GLYPH_DOT         10U
#define GUI_NUM_GLYPH_PERCENT     12U
#define GUI_NUM_GLYPH_CELSIUS     13U
#define GUI_NUM_GLYPH_MINUS       14U
#define GUI_NUM_GLYPH_BLANK       0xFFU
#define GUI_FONT24_DOT_INDEX      11U
#define GUI_FONT24_CELSIUS_INDEX  12U

/* 把显示用的数字编号转换为 Font_Data 中的字形编号。 */
static uint8_t Gui_Font24Index(uint8_t glyph)
{
	if (glyph <= 9U)
	{
		return glyph == 0U ? 9U : (uint8_t)(glyph - 1U);
	}
	if (glyph == GUI_NUM_GLYPH_DOT)
	{
		return GUI_FONT24_DOT_INDEX;
	}
	if (glyph == GUI_NUM_GLYPH_CELSIUS)
	{
		return GUI_FONT24_CELSIUS_INDEX;
	}
	return GUI_NUM_GLYPH_BLANK;
}

/* 读取新加入的 24×24 字模中的一个像素。 */
static uint8_t Gui_Font24Pixel(uint8_t glyph, uint8_t x, uint8_t y)
{
	uint8_t font_index = Gui_Font24Index(glyph);
	uint8_t value;

	if (font_index == GUI_NUM_GLYPH_BLANK)
	{
		return 0U;
	}

	value = Font_Data[font_index].dat[(uint16_t)y * 3U + x / 8U];
	return (uint8_t)((value & (uint8_t)(0x80U >> (x & 0x07U))) != 0U);
}

/* 百分号和负号未包含在新字模中，继续从原字库读取。 */
static uint8_t Gui_LegacySymbolPixel(uint8_t glyph, uint8_t x, uint8_t y)
{
	uint32_t offset;
	uint8_t value;

	offset = (uint32_t)glyph * GUI_NUM32_GLYPH_BYTES;
	offset += (uint32_t)y * 4U + x / 8U;
	value = sz32[offset];
	return (uint8_t)((value & (uint8_t)(0x80U >> (x & 0x07U))) != 0U);
}

/*
 * 新数字字模由 24×24 缩放为 12×24。每个目标像素覆盖的源像素只要
 * 有一个点亮就保留，可避免缩小后笔画断裂。
 */
static void Gui_DrawSmallNumberGlyph(u16 x, u16 y, uint8_t glyph, u16 fc, u16 bc)
{
	uint8_t dst_x;
	uint8_t dst_y;
	uint8_t src_x;
	uint8_t src_y;
	uint8_t src_x_begin;
	uint8_t src_x_end;
	uint8_t src_y_begin;
	uint8_t src_y_end;
	uint8_t pixel_on;
	uint8_t source_size;
	uint8_t use_font24;

	use_font24 = (uint8_t)(Gui_Font24Index(glyph) != GUI_NUM_GLYPH_BLANK);
	source_size = use_font24 != 0U ? GUI_FONT24_SOURCE_SIZE : GUI_NUM32_SOURCE_SIZE;

	Lcd_SetRegion(x + 2U, y, x + GUI_SMALL_NUM_WIDTH - 1U,
		y + GUI_SMALL_NUM_HEIGHT - 1U);

	for (dst_y = 0U; dst_y < GUI_SMALL_NUM_HEIGHT; dst_y++)
	{
		src_y_begin = (uint8_t)((uint16_t)dst_y * source_size /
			GUI_SMALL_NUM_HEIGHT);
		src_y_end = (uint8_t)((uint16_t)(dst_y + 1U) * source_size /
			GUI_SMALL_NUM_HEIGHT);

		for (dst_x = 0U; dst_x < GUI_SMALL_NUM_WIDTH; dst_x++)
		{
			pixel_on = 0U;
			if (glyph != GUI_NUM_GLYPH_BLANK)
			{
				src_x_begin = (uint8_t)((uint16_t)dst_x * source_size /
					GUI_SMALL_NUM_WIDTH);
				src_x_end = (uint8_t)((uint16_t)(dst_x + 1U) * source_size /
					GUI_SMALL_NUM_WIDTH);

				for (src_y = src_y_begin; src_y < src_y_end && pixel_on == 0U; src_y++)
				{
					for (src_x = src_x_begin; src_x < src_x_end; src_x++)
					{
						if ((use_font24 != 0U &&
							Gui_Font24Pixel(glyph, src_x, src_y) != 0U) ||
							(use_font24 == 0U &&
							Gui_LegacySymbolPixel(glyph, src_x, src_y) != 0U))
						{
							pixel_on = 1U;
							break;
						}
					}
				}
			}

			LCD_WriteData_16Bit(pixel_on != 0U ? fc : bc);
		}
	}
}

static void Gui_DrawSixGlyphs(u16 x, u16 y, const uint8_t glyphs[6])
{
	uint8_t index;

	for (index = 0U; index < 6U; index++)
	{
		Gui_DrawSmallNumberGlyph(x + (uint16_t)index * GUI_SMALL_NUM_WIDTH,
			y, glyphs[index], BLACK, WHITE);
	}
}

void Gui_ShowTemperatureValue(u16 x, u16 y, uint8_t integer,
	uint8_t decimal, uint8_t negative, uint8_t valid)
{
	uint8_t glyphs[6];

	if (valid == 0U)
	{
		glyphs[0] = GUI_NUM_GLYPH_BLANK;
		glyphs[1] = GUI_NUM_GLYPH_MINUS;
		glyphs[2] = GUI_NUM_GLYPH_MINUS;
		glyphs[3] = GUI_NUM_GLYPH_DOT;
		glyphs[4] = GUI_NUM_GLYPH_MINUS;
		glyphs[5] = GUI_NUM_GLYPH_CELSIUS;
	}
	else
	{
		glyphs[0] = negative != 0U ? GUI_NUM_GLYPH_MINUS : GUI_NUM_GLYPH_BLANK;
		glyphs[1] = integer >= 10U ? (uint8_t)((integer / 10U) % 10U) : GUI_NUM_GLYPH_BLANK;
		glyphs[2] = (uint8_t)(integer % 10U);
		glyphs[3] = GUI_NUM_GLYPH_DOT;
		glyphs[4] = (uint8_t)(decimal % 10U);
		glyphs[5] = GUI_NUM_GLYPH_CELSIUS;
	}

	Gui_DrawSixGlyphs(x, y, glyphs);
}

void Gui_ShowHumidityValue(u16 x, u16 y, uint8_t integer,
	uint8_t decimal, uint8_t valid)
{
	uint8_t glyphs[6];

	if (valid == 0U)
	{
		glyphs[0] = GUI_NUM_GLYPH_BLANK;
		glyphs[1] = GUI_NUM_GLYPH_MINUS;
		glyphs[2] = GUI_NUM_GLYPH_MINUS;
		glyphs[3] = GUI_NUM_GLYPH_DOT;
		glyphs[4] = GUI_NUM_GLYPH_MINUS;
		glyphs[5] = GUI_NUM_GLYPH_PERCENT;
	}
	else
	{
		glyphs[0] = integer >= 100U ? (uint8_t)((integer / 100U) % 10U) : GUI_NUM_GLYPH_BLANK;
		glyphs[1] = integer >= 10U ? (uint8_t)((integer / 10U) % 10U) : GUI_NUM_GLYPH_BLANK;
		glyphs[2] = (uint8_t)(integer % 10U);
		glyphs[3] = GUI_NUM_GLYPH_DOT;
		glyphs[4] = (uint8_t)(decimal % 10U);
		glyphs[5] = GUI_NUM_GLYPH_PERCENT;
	}

	Gui_DrawSixGlyphs(x, y, glyphs);
}


/**************************************************************************************
功能描述: 任意位置显示任意大小图片
输    入: x - 起点x ；y - 起点y ；c - 图片长度 ；k - 图片宽度；p - 图片数组;
输    出: 无
**************************************************************************************/

void Gui_showimage(const unsigned char *p, uint8_t c, uint8_t k, uint8_t x, uint8_t y)
{
	uint32_t i, picL, picH;
	Lcd_SetRegion(x+2,y,x+(c - 1) ,y + (k - 1));		//坐标设置
    for(i=0;i<c*k;i++)
	 {	
	 	picL=*(p+i*2);	//数据低位在前
		picH=*(p+i*2+1);				
		LCD_WriteData_16Bit(picH<<8|picL);  						
	 }		
}



/* Font.h 只在本文件中包含，避免主程序重复定义字库和图片数组。 */
void Gui_ShowTemperatureIcon(uint8_t x, uint8_t y)
{
	Gui_showimage(TEMPERATURE_ICON_DATA, TEMPERATURE_ICON_WIDTH, TEMPERATURE_ICON_HEIGHT, x, y);
}

/* 显示湿度图标。 */
void Gui_ShowHumidityIcon(uint8_t x, uint8_t y)
{
	Gui_showimage(HUMIDITY_ICON_DATA, HUMIDITY_ICON_WIDTH, HUMIDITY_ICON_HEIGHT, x, y);
}

/* 显示日历图标。 */
void Gui_ShowCalendarIcon(uint8_t x, uint8_t y)
{
	Gui_showimage(CALENDAR_ICON_DATA, CALENDAR_ICON_WIDTH, CALENDAR_ICON_HEIGHT, x, y);
}

/* 显示时钟图标。 */
void Gui_ShowClockIcon(uint8_t x, uint8_t y)
{
	Gui_showimage(CLOCK_ICON_DATA, CLOCK_ICON_WIDTH, CLOCK_ICON_HEIGHT, x, y);
}
