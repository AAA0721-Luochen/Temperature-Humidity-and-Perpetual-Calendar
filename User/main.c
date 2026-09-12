#include "stm32f10x.h"                  // Device header
#include "Lcd_Driver.h"
#include "GUI.h"

int main(void)
{
	Lcd_Init();
	LCD_LED_SET;
	Lcd_Clear(WHITE);
	

	/* 四个图标左侧对齐，每个图标之间留 8 像素。 */
	Gui_ShowCalendarIcon(0, 0);
	Gui_ShowClockIcon(0, 40);
	Gui_ShowTemperatureIcon(0, 80);
	Gui_ShowHumidityIcon(0, 120);
	while(1)
	{
		
	}
}
