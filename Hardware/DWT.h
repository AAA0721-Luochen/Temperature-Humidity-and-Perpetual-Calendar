#ifndef __DWT_H
#define __DWT_H

#include "stm32f10x.h"

/* 初始化 Cortex-M3 的 DWT 周期计数器。 */
void DWT_Init(void);

/* 获取当前周期计数值。 */
uint32_t DWT_GetTick(void);

/* 基于内核周期计数器的阻塞延时。 */
void DWT_DelayUs(uint32_t time);
void DWT_DelayMs(uint32_t time);

#endif
