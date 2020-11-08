#include "stm32f4xx.h"
#include "system_stm32F4xx.h"
#include "stdlib.h"
#include "ff.h"
#include "sdio.h"
#include "cmsis_delay.h"
#define HSE_VALUE    ((uint32_t)8000000)


uint32_t RccInit()
{
	PWR->CR |= PWR_CR_VOS; //Performance power mode enabled
	RCC->CR |= RCC_CR_HSEON; // HSE enable
	//RCC->CR |= RCC_CR_CSSON; // CSS enable
	RCC->PLLCFGR = 0; //PLL register set 0
	RCC->PLLCFGR |= 7 << RCC_PLLCFGR_PLLQ_Pos; // /Q 48 MHz clock devider 
	RCC->PLLCFGR |= 4 << RCC_PLLCFGR_PLLM_Pos; // /M Main devider
	RCC->PLLCFGR |= 168 << RCC_PLLCFGR_PLLN_Pos; // *N Multiplicator
	RCC->PLLCFGR |= 0 << RCC_PLLCFGR_PLLP_Pos;// /P
	RCC->PLLCFGR |= RCC_PLLCFGR_PLLSRC_HSE; // HSE as source for PLL
	RCC->CR |= RCC_CR_PLLON; //PLL enable
	FLASH->ACR |= FLASH_ACR_LATENCY_7WS; // latency set
	GPIOA->BSRR |= 0x20000; // PA1 reset
	RCC->CFGR |= RCC_CFGR_PPRE2_DIV2; //APB2 prescaler set
	RCC->CFGR |= RCC_CFGR_PPRE1_DIV4; //APB1 prescaler set
	while((RCC->CR & RCC_CR_HSERDY_Msk )!= RCC_CR_HSERDY){}; //Wait for HSE
	while(((RCC->CR & RCC_CR_PLLRDY_Msk )!= RCC_CR_PLLRDY)){}; //Wait for PLL
	RCC->CFGR |= RCC_CFGR_SW_PLL; //Sslect PLL as system clock source
	SystemCoreClockUpdate(); //calculate current clock
	if(((RCC->CFGR & RCC_CFGR_SWS_PLL) != RCC_CFGR_SWS_PLL)) return 10;
	else return 0;
	
}

void GpioInit()
{
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN; // GPIOA clock enable
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN; // GPIOC clock enable
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN; // GPIOD clock enable
	GPIOA->MODER |= 0x4; // PA1 set to output
}

void UsartInit()
{
	RCC->APB2ENR|=RCC_APB2ENR_USART1EN;
	GPIOA->MODER|=0xA<<18; //Set PA9, PA10 to alternate function
	GPIOA->AFR[1]|=7<<4; //PA9 set alternate function Usart1
	GPIOA->AFR[1]|=7<<8; //PA9 set alternate function Usart1
	uint32_t baud = 38400; //baudrate
	uint32_t mantissa = ((SystemCoreClock/2) /(16 * baud));
	uint64_t fraction = 168000000;
	fraction *= 5000;
	fraction /= 16;
	fraction /= baud;
	fraction %= mantissa*10000;
	for(uint8_t i = 4; i>0;i--)
	{
		fraction = fraction << 1;
		USART1->BRR|=(fraction/10000)<<(i-1);
		fraction %= 10000;
	}
	USART1->BRR|=mantissa<<4;
	USART1->CR1|=USART_CR1_UE;
	USART1->CR1|=USART_CR1_TE;
	USART1->CR1|=USART_CR1_RE;
}

void USARTPrint(uint8_t text[],uint32_t size)
{
	for(int i = 0;i<size;i++)
		{
			while(!(USART1->SR & USART_SR_TC)){}
			USART1->DR=text[i];
		}
}


void RccChange()
{
	//TODO
}
void Fault_Handler()
{
	while(1){}
}
int main()
{
	GpioInit();
	if(RccInit()==10) Fault_Handler();
	SDIO_Init();
	UsartInit();
	SDIO_Connect();
	SDIO_Command(7,0x1,SDIO_Get_RCA()<<16,0);
	SDIO_disk_read(0,0,0);
	GPIOA->BSRR|=GPIO_BSRR_BS1;
	Delay(100);
	GPIOA->BSRR|=GPIO_BSRR_BR1;
	Delay(100);
	while(1)
	{
			
		GPIOA->BSRR|=GPIO_BSRR_BS1;
		Delay(1000);
		GPIOA->BSRR|=GPIO_BSRR_BR1;
		Delay(1000);
	}
}
