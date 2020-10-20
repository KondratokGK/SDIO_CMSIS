#include "stm32f4xx.h"
#include "system_stm32F4xx.h"
#include "stdlib.h"
#include "cmsis_delay.h"

//_Bool InitDelayFlag = 0;
volatile uint64_t DelayCounter = 0;
uint32_t OldCoreClock;
uint32_t devider=1000;

void SysTick_Handler(void) {
  DelayCounter++;
}

int Delay_Init(uint32_t dev)
{
	
	devider=dev;
	SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk;
	SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
	SysTick_Config(SystemCoreClock/devider);
	OldCoreClock=SystemCoreClock;
	//InitDelayFlag = 1;
	return 0;
	
}

void Delay(const uint32_t milliseconds)
{
    SystemCoreClockUpdate();
		if(OldCoreClock!=SystemCoreClock) Delay_Init(1000);
		//if(InitDelayFlag == 0) InitDelay(1000);
		uint32_t start = DelayCounter;
    while((DelayCounter - start) < milliseconds);
}
