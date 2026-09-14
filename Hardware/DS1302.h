#ifndef __DS1302_H
#define __DS1302_H

#include "stm32f10x.h"

/*
 * DS1302 实时时钟驱动
 * 三线接口使用 GPIO 模拟：RST/CE=PB7，DATA=PB8，CLK=PB9。
 * DATA 是双向数据线，驱动会在发送命令和读取数据之间切换 GPIO 方向。
 */
#define DS1302_GPIO_PORT       GPIOB
#define DS1302_GPIO_CLK        RCC_APB2Periph_GPIOB
#define DS1302_RST_PIN         GPIO_Pin_7
#define DS1302_DATA_PIN        GPIO_Pin_8
#define DS1302_CLK_PIN         GPIO_Pin_9

typedef struct
{
	uint16_t year;  /* 2000~2099 */
	uint8_t month;  /* 1~12 */
	uint8_t day;    /* 1~31，受月份和闰年限制 */
	uint8_t week;   /* 1~7，本工程约定 1 为星期一 */
	uint8_t hour;   /* 0~23 */
	uint8_t minute; /* 0~59 */
	uint8_t second; /* 0~59 */
} DS1302_TimeTypeDef;

/* 初始化 GPIO，并关闭针对不可充电 CR2032 的涓流充电。不会重写时间。 */
void DS1302_Init(void);

/* 一次突发读取完整日期和时间；数据无效或时钟停振时返回 ERROR。 */
ErrorStatus DS1302_ReadTime(DS1302_TimeTypeDef *time);

/* 写入 2000~2099 年的日期和 24 小时时间。week 使用 1~7。 */
ErrorStatus DS1302_SetTime(const DS1302_TimeTypeDef *time);

/* 读写 31 字节掉电保持 RAM，index 范围为 0~30。 */
ErrorStatus DS1302_ReadRamByte(uint8_t index, uint8_t *value);
ErrorStatus DS1302_WriteRamByte(uint8_t index, uint8_t value);

#endif
