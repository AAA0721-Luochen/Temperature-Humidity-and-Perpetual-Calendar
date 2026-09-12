#include "DHT11.h"
#include "DWT.h"

#define DHT11_DATA_READ() \
	GPIO_ReadInputDataBit(DHT11_DATA_GPIO_PORT, DHT11_DATA_GPIO_PIN)
#define DHT11_DATA_HIGH() \
	GPIO_SetBits(DHT11_DATA_GPIO_PORT, DHT11_DATA_GPIO_PIN)
#define DHT11_DATA_LOW() \
	GPIO_ResetBits(DHT11_DATA_GPIO_PORT, DHT11_DATA_GPIO_PIN)

static void DHT11_SetPinMode(GPIOMode_TypeDef mode)
{
	GPIO_InitTypeDef gpio;

	gpio.GPIO_Pin = DHT11_DATA_GPIO_PIN;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	gpio.GPIO_Mode = mode;
	GPIO_Init(DHT11_DATA_GPIO_PORT, &gpio);
}

/* 等待引脚离开指定电平，超时后返回 ERROR。 */
static ErrorStatus DHT11_WaitWhile(BitAction level, uint32_t timeout_us)
{
	uint32_t start = DWT_GetTick();
	uint32_t timeout_cycles = timeout_us * (SystemCoreClock / 1000000U);

	while (DHT11_DATA_READ() == level)
	{
		if ((uint32_t)(DWT_GetTick() - start) >= timeout_cycles)
		{
			return ERROR;
		}
	}

	return SUCCESS;
}

static ErrorStatus DHT11_ReadByte(uint8_t *value)
{
	uint8_t bit;
	uint8_t result = 0U;

	for (bit = 0U; bit < 8U; bit++)
	{
		/* 每一位以约 54us 的低电平开始。 */
		if (DHT11_WaitWhile(Bit_RESET, 80U) == ERROR)
		{
			return ERROR;
		}

		/* 约 40us 后仍为高电平，表示数据 1。 */
		DWT_DelayUs(40U);
		result <<= 1;
		if (DHT11_DATA_READ() == Bit_SET)
		{
			result |= 0x01U;
			if (DHT11_WaitWhile(Bit_SET, 100U) == ERROR)
			{
				return ERROR;
			}
		}
	}

	*value = result;
	return SUCCESS;
}

void DHT11_Init(void)
{
	RCC_APB2PeriphClockCmd(DHT11_DATA_GPIO_CLK, ENABLE);
	DHT11_SetPinMode(GPIO_Mode_IPU);
}

ErrorStatus DHT11_ReadData(DHT11_DataTypeDef *data)
{
	uint8_t raw[5];
	uint8_t index;
	uint8_t sum;

	if (data == 0)
	{
		return ERROR;
	}

	/* 主机发送 20ms 低电平起始信号。 */
	DHT11_SetPinMode(GPIO_Mode_Out_OD);
	DHT11_DATA_HIGH();
	DWT_DelayUs(10U);
	DHT11_DATA_LOW();
	DWT_DelayMs(20U);
	DHT11_DATA_HIGH();
	DHT11_SetPinMode(GPIO_Mode_IPU);
	DWT_DelayUs(10U);

	/* DHT11 应答：约 83us 低电平，再约 87us 高电平。 */
	if (DHT11_WaitWhile(Bit_SET, 60U) == ERROR ||
		DHT11_WaitWhile(Bit_RESET, 100U) == ERROR ||
		DHT11_WaitWhile(Bit_SET, 110U) == ERROR)
	{
		return ERROR;
	}

	for (index = 0U; index < 5U; index++)
	{
		if (DHT11_ReadByte(&raw[index]) == ERROR)
		{
			return ERROR;
		}
	}

	sum = (uint8_t)(raw[0] + raw[1] + raw[2] + raw[3]);
	if (sum != raw[4])
	{
		return ERROR;
	}

	data->humidity_integer = raw[0];
	data->humidity_decimal = raw[1];
	data->temperature_integer = raw[2];
	data->temperature_decimal = raw[3];
	data->checksum = raw[4];

	return SUCCESS;
}
