#include "stm32f10x.h"                  // Device header
#include "Lcd_Driver.h"
#include "GUI.h"
#include "DWT.h"
#include "DHT11.h"

int main(void)
{
	DHT11_DataTypeDef dht11_data;
	uint32_t last_sample;
	uint32_t sample_interval_cycles;
	uint8_t temperature_negative;
	uint8_t temperature_decimal;
	ErrorStatus read_status;

	DWT_Init();
	DHT11_Init();
	Lcd_Init();
	LCD_LED_SET;
	Lcd_Clear(WHITE);
	

	/* 四个图标左侧对齐，每个图标之间留 8 像素。 */
	Gui_ShowCalendarIcon(0, 0);
	Gui_ShowClockIcon(0, 40);
	Gui_ShowTemperatureIcon(0, 80);
	Gui_ShowHumidityIcon(0, 120);
	Gui_ShowTemperatureValue(40, 84, 0, 0, 0, 0);
	Gui_ShowHumidityValue(40, 124, 0, 0, 0);

	/* DHT11 上电后至少等待 1 秒，采集间隔保持大于 2 秒。 */
	DWT_DelayMs(1000U);
	sample_interval_cycles = (SystemCoreClock / 1000U) * 2500U;
	last_sample = DWT_GetTick() - sample_interval_cycles;

	while(1)
	{
		if ((uint32_t)(DWT_GetTick() - last_sample) >= sample_interval_cycles)
		{
			last_sample = DWT_GetTick();
			read_status = DHT11_ReadData(&dht11_data);

			if (read_status == SUCCESS)
			{
				temperature_negative =
					(uint8_t)((dht11_data.temperature_decimal & 0x80U) != 0U);
				temperature_decimal =
					(uint8_t)(dht11_data.temperature_decimal & 0x7FU);

				Gui_ShowTemperatureValue(40, 84,
					dht11_data.temperature_integer, temperature_decimal,
					temperature_negative, 1U);
				Gui_ShowHumidityValue(40, 124,
					dht11_data.humidity_integer, dht11_data.humidity_decimal, 1U);
			}
			else
			{
				Gui_ShowTemperatureValue(40, 84, 0U, 0U, 0U, 0U);
				Gui_ShowHumidityValue(40, 124, 0U, 0U, 0U);
			}
		}
	}
}
