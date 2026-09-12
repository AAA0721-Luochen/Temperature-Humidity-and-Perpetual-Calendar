#include "DWT.h"

#define DWT_DEMCR_REG       (*(volatile uint32_t *)0xE000EDFCUL)
#define DWT_CTRL_REG        (*(volatile uint32_t *)0xE0001000UL)
#define DWT_CYCCNT_REG      (*(volatile uint32_t *)0xE0001004UL)
#define DWT_TRCENA_MASK     (1UL << 24)
#define DWT_CYCCNTENA_MASK  (1UL << 0)

void DWT_Init(void)
{
	DWT_DEMCR_REG |= DWT_TRCENA_MASK;
	DWT_CYCCNT_REG = 0U;
	DWT_CTRL_REG |= DWT_CYCCNTENA_MASK;
}

uint32_t DWT_GetTick(void)
{
	return DWT_CYCCNT_REG;
}

void DWT_DelayUs(uint32_t time)
{
	uint32_t start = DWT_GetTick();
	uint32_t cycles = time * (SystemCoreClock / 1000000U);

	while ((uint32_t)(DWT_GetTick() - start) < cycles)
	{
	}
}

void DWT_DelayMs(uint32_t time)
{
	while (time-- > 0U)
	{
		DWT_DelayUs(1000U);
	}
}
