#ifndef __DHT11_H
#define __DHT11_H

#include "stm32f10x.h"

/* DHT11 单总线连接到 PB14。DATA 应上拉到 3.3V。 */
#define DHT11_DATA_GPIO_PORT      GPIOB
#define DHT11_DATA_GPIO_CLK       RCC_APB2Periph_GPIOB
#define DHT11_DATA_GPIO_PIN       GPIO_Pin_14

typedef struct
{
	uint8_t humidity_integer;
	uint8_t humidity_decimal;
	uint8_t temperature_integer;
	uint8_t temperature_decimal;
	uint8_t checksum;
} DHT11_DataTypeDef;

void DHT11_Init(void);
ErrorStatus DHT11_ReadData(DHT11_DataTypeDef *data);

#endif
