#include "stm32f10x.h"                  // Device header
#include "Lcd_Driver.h"
#include "GUI.h"
#include "Picture.h"

int main(void)
{
	u8 ch[] = "\xC4\xE3\xBA\xC3\xA3\xAC\xCA\xC0\xBD\xE7" /* GBK: 你好，世界 */;
	
	Lcd_Init();
	LCD_LED_SET;
	Lcd_Clear(WHITE);
	

	Gui_ShowUserImage(80, 20);	// 在右上区域显示温度图标
	Gui_ShowHumidityIcon(80, 60);	// 在温度图标下方显示湿度图标
	Gui_DrawFont_GBK16(1, 100, BLACK, WHITE, ch);
	while(1)
	{
		
	}
}
