/*
 * STM32F103C8T6 桌面环境显示程序
 *
 * DS1302 提供日期和时间，DHT11 提供温湿度，1.8 英寸 TFT 负责显示。
 * 界面自上而下依次为日期、时间、温度和湿度。
 */
#include "stm32f10x.h"                  // Device header
#include "Lcd_Driver.h"
#include "GUI.h"
#include "DWT.h"
#include "DHT11.h"
#include "DS1302.h"

/* 日期占满 128 像素宽度并由 GUI 居中；时间宽 88 像素，起点 20 可居中。 */
#define RTC_DATE_VALUE_X 0U
#define RTC_TIME_VALUE_X 20U

/*
 * 使用 DS1302 最后两个 RAM 字节记录校准版本。
 * 修改默认校准时间时同步修改 VERSION，可使新固件只校准一次，
 * 后续复位仍继续使用纽扣电池保存的时间。
 */
#define RTC_CALIBRATION_SIGNATURE_INDEX 29U
#define RTC_CALIBRATION_VERSION_INDEX   30U
#define RTC_CALIBRATION_SIGNATURE       0xD2U
#define RTC_CALIBRATION_VERSION         0x51U

int main(void)
{
	const DS1302_TimeTypeDef initial_rtc_time =
	{
		2026U, 9U, 14U, 1U,  /* 年、月、日、星期一 */
		16U, 51U, 0U          /* 时、分、秒 */
	};

	/* 传感器数据及最近一次成功显示的 RTC 数据。 */
	DHT11_DataTypeDef dht11_data;
	DS1302_TimeTypeDef rtc_time;
	DS1302_TimeTypeDef displayed_rtc_time;

	/* DWT 周期计数器用于实现互不阻塞的周期任务。 */
	uint32_t last_sample;
	uint32_t sample_interval_cycles;
	uint32_t last_rtc_sample;
	uint32_t rtc_sample_interval_cycles;
	uint8_t temperature_negative;
	uint8_t temperature_decimal;
	uint8_t rtc_display_valid;
	uint8_t calibration_signature;
	uint8_t calibration_version;
	ErrorStatus read_status;

	/* DS1302 驱动使用 DWT 微秒延时，因此 DWT 必须最先初始化。 */
	DWT_Init();
	DS1302_Init();
	/* 新校准版本只写入一次，后续复位继续使用电池保存的走时。 */
	if (DS1302_ReadRamByte(RTC_CALIBRATION_SIGNATURE_INDEX,
			&calibration_signature) == ERROR ||
		DS1302_ReadRamByte(RTC_CALIBRATION_VERSION_INDEX,
			&calibration_version) == ERROR ||
		calibration_signature != RTC_CALIBRATION_SIGNATURE ||
		calibration_version != RTC_CALIBRATION_VERSION ||
		DS1302_ReadTime(&rtc_time) == ERROR)
	{
		if (DS1302_SetTime(&initial_rtc_time) == SUCCESS)
		{
			DS1302_WriteRamByte(RTC_CALIBRATION_SIGNATURE_INDEX,
				RTC_CALIBRATION_SIGNATURE);
			DS1302_WriteRamByte(RTC_CALIBRATION_VERSION_INDEX,
				RTC_CALIBRATION_VERSION);
		}
	}
	DHT11_Init();
	Lcd_Init();
	LCD_LED_SET;
	Lcd_Clear(WHITE);
	

	/* 日期、时间独占前两行；温湿度图标保持左侧对齐。 */
	Gui_ShowTemperatureIcon(0, 80);
	Gui_ShowHumidityIcon(0, 120);
	Gui_ShowCalendarValue(RTC_DATE_VALUE_X, 4, 0U, 0U, 0U, 0U);
	Gui_ShowClockValue(RTC_TIME_VALUE_X, 44, 0U, 0U, 0U, 0U);
	Gui_ShowTemperatureValue(40, 84, 0, 0, 0, 0);
	Gui_ShowHumidityValue(40, 124, 0, 0, 0);

	/* DHT11 上电后等待 1 秒，此后每 1 秒采集一次。 */
	DWT_DelayMs(1000U);
	sample_interval_cycles = (SystemCoreClock / 1000U) * 1000U;
	last_sample = DWT_GetTick() - sample_interval_cycles;
	rtc_sample_interval_cycles = (SystemCoreClock / 1000U) * 200U;
	last_rtc_sample = DWT_GetTick() - rtc_sample_interval_cycles;
	rtc_display_valid = 0U;

	while(1)
	{
		/* 每 200ms 读取一次一致的 RTC 快照，数值变化后才刷新屏幕。 */
		if ((uint32_t)(DWT_GetTick() - last_rtc_sample) >= rtc_sample_interval_cycles)
		{
			last_rtc_sample = DWT_GetTick();
			if (DS1302_ReadTime(&rtc_time) == SUCCESS)
			{
				if (rtc_display_valid == 0U ||
					rtc_time.year != displayed_rtc_time.year ||
					rtc_time.month != displayed_rtc_time.month ||
					rtc_time.day != displayed_rtc_time.day)
				{
					Gui_ShowCalendarValue(RTC_DATE_VALUE_X, 4, rtc_time.year,
						rtc_time.month, rtc_time.day, 1U);
				}
				if (rtc_display_valid == 0U ||
					rtc_time.hour != displayed_rtc_time.hour ||
					rtc_time.minute != displayed_rtc_time.minute ||
					rtc_time.second != displayed_rtc_time.second)
				{
					Gui_ShowClockValue(RTC_TIME_VALUE_X, 44, rtc_time.hour,
						rtc_time.minute, rtc_time.second, 1U);
				}
				displayed_rtc_time = rtc_time;
				rtc_display_valid = 1U;
			}
			else if (rtc_display_valid != 0U)
			{
				Gui_ShowCalendarValue(RTC_DATE_VALUE_X, 4, 0U, 0U, 0U, 0U);
				Gui_ShowClockValue(RTC_TIME_VALUE_X, 44, 0U, 0U, 0U, 0U);
				rtc_display_valid = 0U;
			}
		}

		/* DHT11 采集与显示任务。读取失败时显示占位符。 */
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
