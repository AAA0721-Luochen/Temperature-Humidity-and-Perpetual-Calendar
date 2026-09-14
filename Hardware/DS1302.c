#include "DS1302.h"
#include "DWT.h"

/* DS1302 寄存器命令：偶数地址写，奇数地址读。 */
#define DS1302_WRITE_PROTECT_WRITE 0x8EU
#define DS1302_TRICKLE_WRITE       0x90U
#define DS1302_CLOCK_BURST_WRITE   0xBEU
#define DS1302_CLOCK_BURST_READ    0xBFU
#define DS1302_RAM_WRITE_BASE      0xC0U
#define DS1302_RAM_SIZE            31U

/* 直接操作 BSRR/BRR 的 GPIO 宏用于产生三线通信时序。 */
#define DS1302_RST_HIGH()  GPIO_SetBits(DS1302_GPIO_PORT, DS1302_RST_PIN)
#define DS1302_RST_LOW()   GPIO_ResetBits(DS1302_GPIO_PORT, DS1302_RST_PIN)
#define DS1302_CLK_HIGH()  GPIO_SetBits(DS1302_GPIO_PORT, DS1302_CLK_PIN)
#define DS1302_CLK_LOW()   GPIO_ResetBits(DS1302_GPIO_PORT, DS1302_CLK_PIN)
#define DS1302_DATA_HIGH() GPIO_SetBits(DS1302_GPIO_PORT, DS1302_DATA_PIN)
#define DS1302_DATA_LOW()  GPIO_ResetBits(DS1302_GPIO_PORT, DS1302_DATA_PIN)
#define DS1302_DATA_READ() GPIO_ReadInputDataBit(DS1302_GPIO_PORT, DS1302_DATA_PIN)

#define DS1302_IO_DELAY_US 1U

static void DS1302_SetDataOutput(void)
{
	GPIO_InitTypeDef gpio;

	gpio.GPIO_Pin = DS1302_DATA_PIN;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	gpio.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(DS1302_GPIO_PORT, &gpio);
}

static void DS1302_SetDataInput(void)
{
	GPIO_InitTypeDef gpio;

	gpio.GPIO_Pin = DS1302_DATA_PIN;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(DS1302_GPIO_PORT, &gpio);
}

static void DS1302_Begin(void)
{
	/* CE 上升沿开始一次命令；开始前保证 CLK 为低电平。 */
	DS1302_SetDataOutput();
	DS1302_DATA_LOW();
	DS1302_CLK_LOW();
	DS1302_RST_LOW();
	DWT_DelayUs(DS1302_IO_DELAY_US);
	DS1302_RST_HIGH();
	DWT_DelayUs(4U);
}

static void DS1302_End(void)
{
	/* CE 拉低结束命令，并将总线恢复为低电平输出。 */
	DS1302_RST_LOW();
	DS1302_CLK_LOW();
	DS1302_SetDataOutput();
	DS1302_DATA_LOW();
	DWT_DelayUs(DS1302_IO_DELAY_US);
}

/* DS1302 的命令和数据均最低位先发送。 */
static void DS1302_WriteByte(uint8_t value)
{
	uint8_t bit;

	for (bit = 0U; bit < 8U; bit++)
	{
		if ((value & 0x01U) != 0U)
		{
			DS1302_DATA_HIGH();
		}
		else
		{
			DS1302_DATA_LOW();
		}

		DWT_DelayUs(DS1302_IO_DELAY_US);
		DS1302_CLK_HIGH();
		DWT_DelayUs(DS1302_IO_DELAY_US);
		DS1302_CLK_LOW();
		DWT_DelayUs(DS1302_IO_DELAY_US);
		value >>= 1;
	}
}

/* 读命令最后一个上升沿后先释放 DATA，再产生下降沿，避免总线冲突。 */
static void DS1302_WriteReadCommand(uint8_t command)
{
	uint8_t bit;

	for (bit = 0U; bit < 8U; bit++)
	{
		if ((command & 0x01U) != 0U)
		{
			DS1302_DATA_HIGH();
		}
		else
		{
			DS1302_DATA_LOW();
		}

		DWT_DelayUs(DS1302_IO_DELAY_US);
		DS1302_CLK_HIGH();
		DWT_DelayUs(DS1302_IO_DELAY_US);
		if (bit == 7U)
		{
			DS1302_SetDataInput();
		}
		DS1302_CLK_LOW();
		DWT_DelayUs(DS1302_IO_DELAY_US);
		command >>= 1;
	}
}

static uint8_t DS1302_ReadByte(void)
{
	uint8_t bit;
	uint8_t value = 0U;

	/* DS1302 同样按最低位在前的顺序输出数据。 */
	for (bit = 0U; bit < 8U; bit++)
	{
		if (DS1302_DATA_READ() == Bit_SET)
		{
			value |= (uint8_t)(1U << bit);
		}

		DS1302_CLK_HIGH();
		DWT_DelayUs(DS1302_IO_DELAY_US);
		DS1302_CLK_LOW();
		DWT_DelayUs(DS1302_IO_DELAY_US);
	}

	return value;
}

static void DS1302_WriteRegister(uint8_t address, uint8_t value)
{
	DS1302_Begin();
	DS1302_WriteByte(address);
	DS1302_WriteByte(value);
	DS1302_End();
}

static uint8_t DS1302_ReadRegister(uint8_t address)
{
	uint8_t value;

	DS1302_Begin();
	DS1302_WriteReadCommand(address);
	value = DS1302_ReadByte();
	DS1302_End();
	return value;
}

static void DS1302_ReadClockBurst(uint8_t raw[8])
{
	uint8_t index;

	/* 突发读取会获得同一时刻的完整日期时间，避免跨秒不一致。 */
	DS1302_Begin();
	DS1302_WriteReadCommand(DS1302_CLOCK_BURST_READ);
	for (index = 0U; index < 8U; index++)
	{
		raw[index] = DS1302_ReadByte();
	}
	DS1302_End();
}

static void DS1302_WriteClockBurst(const uint8_t raw[8])
{
	uint8_t index;

	DS1302_Begin();
	DS1302_WriteByte(DS1302_CLOCK_BURST_WRITE);
	for (index = 0U; index < 8U; index++)
	{
		DS1302_WriteByte(raw[index]);
	}
	DS1302_End();
}

static uint8_t DS1302_DecimalToBcd(uint8_t value)
{
	return (uint8_t)(((value / 10U) << 4) | (value % 10U));
}

static uint8_t DS1302_BcdToDecimal(uint8_t value)
{
	return (uint8_t)(((value >> 4) * 10U) + (value & 0x0FU));
}

static uint8_t DS1302_IsValidBcd(uint8_t value)
{
	return (uint8_t)(((value & 0x0FU) <= 9U) && ((value >> 4) <= 9U));
}

static uint8_t DS1302_DaysInMonth(uint16_t year, uint8_t month)
{
	/* DS1302 年份只保存两位，本工程限定在 2000~2099 年。 */
	static const uint8_t days[12] =
	{
		31U, 28U, 31U, 30U, 31U, 30U,
		31U, 31U, 30U, 31U, 30U, 31U
	};
	uint8_t result;

	if (month < 1U || month > 12U)
	{
		return 0U;
	}

	result = days[month - 1U];
	if (month == 2U && (year % 4U) == 0U)
	{
		result = 29U;
	}
	return result;
}

static ErrorStatus DS1302_ValidateTime(const DS1302_TimeTypeDef *time)
{
	if (time == 0 || time->year < 2000U || time->year > 2099U ||
		time->month < 1U || time->month > 12U ||
		time->day < 1U || time->day > DS1302_DaysInMonth(time->year, time->month) ||
		time->week < 1U || time->week > 7U ||
		time->hour > 23U || time->minute > 59U || time->second > 59U)
	{
		return ERROR;
	}

	return SUCCESS;
}

void DS1302_Init(void)
{
	GPIO_InitTypeDef gpio;

	RCC_APB2PeriphClockCmd(DS1302_GPIO_CLK, ENABLE);

	gpio.GPIO_Pin = DS1302_RST_PIN | DS1302_CLK_PIN;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	gpio.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(DS1302_GPIO_PORT, &gpio);
	DS1302_SetDataOutput();

	DS1302_RST_LOW();
	DS1302_CLK_LOW();
	DS1302_DATA_LOW();

	/* CR2032 不可充电：解除保护、关闭涓流充电，再恢复写保护。 */
	DS1302_WriteRegister(DS1302_WRITE_PROTECT_WRITE, 0x00U);
	DS1302_WriteRegister(DS1302_TRICKLE_WRITE, 0x00U);
	DS1302_WriteRegister(DS1302_WRITE_PROTECT_WRITE, 0x80U);
}

ErrorStatus DS1302_ReadTime(DS1302_TimeTypeDef *time)
{
	uint8_t raw[8];
	uint8_t hour12;
	uint8_t raw_hour;

	if (time == 0)
	{
		return ERROR;
	}

	DS1302_ReadClockBurst(raw);
	/* CH=1 表示晶振停止；同时拒绝保留位或 BCD 编码非法的数据。 */
	if ((raw[0] & 0x80U) != 0U || (raw[1] & 0x80U) != 0U ||
		(raw[2] & 0x40U) != 0U || (raw[3] & 0xC0U) != 0U ||
		(raw[4] & 0xE0U) != 0U || (raw[5] & 0xF8U) != 0U ||
		DS1302_IsValidBcd(raw[0] & 0x7FU) == 0U ||
		DS1302_IsValidBcd(raw[1] & 0x7FU) == 0U ||
		DS1302_IsValidBcd(raw[3] & 0x3FU) == 0U ||
		DS1302_IsValidBcd(raw[4] & 0x1FU) == 0U ||
		DS1302_IsValidBcd(raw[6]) == 0U)
	{
		return ERROR;
	}

	time->second = DS1302_BcdToDecimal(raw[0] & 0x7FU);
	time->minute = DS1302_BcdToDecimal(raw[1] & 0x7FU);

	/* 兼容模块中已有的 12 小时制数据，对外统一转换为 24 小时制。 */
	if ((raw[2] & 0x80U) != 0U)
	{
		raw_hour = (uint8_t)(raw[2] & 0x1FU);
		if (DS1302_IsValidBcd(raw_hour) == 0U)
		{
			return ERROR;
		}
		hour12 = DS1302_BcdToDecimal(raw_hour);
		if (hour12 < 1U || hour12 > 12U)
		{
			return ERROR;
		}
		time->hour = (uint8_t)(hour12 % 12U);
		if ((raw[2] & 0x20U) != 0U)
		{
			time->hour = (uint8_t)(time->hour + 12U);
		}
	}
	else
	{
		raw_hour = (uint8_t)(raw[2] & 0x3FU);
		if (DS1302_IsValidBcd(raw_hour) == 0U)
		{
			return ERROR;
		}
		time->hour = DS1302_BcdToDecimal(raw_hour);
	}

	time->day = DS1302_BcdToDecimal(raw[3] & 0x3FU);
	time->month = DS1302_BcdToDecimal(raw[4] & 0x1FU);
	time->week = (uint8_t)(raw[5] & 0x07U);
	time->year = (uint16_t)(2000U + DS1302_BcdToDecimal(raw[6]));

	return DS1302_ValidateTime(time);
}

ErrorStatus DS1302_SetTime(const DS1302_TimeTypeDef *time)
{
	uint8_t raw[8];

	if (DS1302_ValidateTime(time) == ERROR)
	{
		return ERROR;
	}

	raw[0] = DS1302_DecimalToBcd(time->second); /* CH=0，启动振荡器。 */
	raw[1] = DS1302_DecimalToBcd(time->minute);
	raw[2] = DS1302_DecimalToBcd(time->hour);   /* 24 小时制。 */
	raw[3] = DS1302_DecimalToBcd(time->day);
	raw[4] = DS1302_DecimalToBcd(time->month);
	raw[5] = time->week;
	raw[6] = DS1302_DecimalToBcd((uint8_t)(time->year - 2000U));
	raw[7] = 0x00U;

	/* 写时间前解除保护，突发写完后立即恢复写保护。 */
	DS1302_WriteRegister(DS1302_WRITE_PROTECT_WRITE, 0x00U);
	DS1302_WriteClockBurst(raw);
	DS1302_WriteRegister(DS1302_WRITE_PROTECT_WRITE, 0x80U);

	return SUCCESS;
}

ErrorStatus DS1302_ReadRamByte(uint8_t index, uint8_t *value)
{
	uint8_t address;

	if (index >= DS1302_RAM_SIZE || value == 0)
	{
		return ERROR;
	}

	address = (uint8_t)(DS1302_RAM_WRITE_BASE + index * 2U + 1U);
	*value = DS1302_ReadRegister(address);
	return SUCCESS;
}

ErrorStatus DS1302_WriteRamByte(uint8_t index, uint8_t value)
{
	uint8_t address;

	if (index >= DS1302_RAM_SIZE)
	{
		return ERROR;
	}

	/* RAM 与时钟寄存器共用 WP 写保护。 */
	address = (uint8_t)(DS1302_RAM_WRITE_BASE + index * 2U);
	DS1302_WriteRegister(DS1302_WRITE_PROTECT_WRITE, 0x00U);
	DS1302_WriteRegister(address, value);
	DS1302_WriteRegister(DS1302_WRITE_PROTECT_WRITE, 0x80U);
	return SUCCESS;
}
