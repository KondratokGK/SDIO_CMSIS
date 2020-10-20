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
	//uint32_t temp[4];
	GpioInit();
	if(RccInit()==10) Fault_Handler();
	SDIO_Init();
	UsartInit();
	uint8_t textt[]="All done\r\n";
//USARTPrint(textt,sizeof(textt)/sizeof(*textt)-1);
//	delay(100);
//	SdioCommand(0,0,0,0);
//	USARTPrint((uint8_t *)"Reset\r\n",7);
//	delay(100);
//	USARTPrint((uint8_t *)"Wait \r\n",7);
//	SdioCommand(8,3,0x000001AA,temp);
//	delay(1);
//	SdioCommand(41,3,0x40000000,temp);
//	delay(1);
//	for(int i=0;i<24;i++)
//	{
//		//SdioCommand(0,0,0,0);
//		delay(1);
//		//SdioCommand(8,3,0x000001AA,temp);
//		delay(1);
//		SDIO->ICR=0xFFFFFFFF;
//		delay(1);
//		SdioCommand(41,3,0x40000000|1<<i,temp);
//		delay(1);
//		if(!(SDIO->STA&SDIO_STA_CTIMEOUT)) i=25;
//	}
	//SdioCommand(41,3,0x00000000|1<<17,temp);
	//USARTPrint((uint8_t *)"Get  \r\n",7);
	//delay(100);
	//USARTPrint((uint8_t *)"Wait \r\n",7);
	//uint8_t text[] = "SDIO   status:";
	//USARTPrint(text,sizeof(text)/sizeof(*text));
	//uint32_t response = SDIO->STA;
	//uint8_t RespText[]="0x--------\r\n";
//	for(int i = 0;i<32;i+=4)
//	{
//		uint8_t temp = (response%(1<<(i+4))/(1<<i));
//		if(temp>9)
//		{
//			RespText[sizeof(RespText)/sizeof(*RespText)-2-2-(i/4)]=temp+55;
//		}
//		else
//		{
//			RespText[sizeof(RespText)/sizeof(*RespText)-2-2-(i/4)]=temp+48;//ASCII convert
//		}
//		
//	}
//	USARTPrint(RespText,sizeof(RespText));
//	uint8_t ttext[] = "SDIO response:";
//	USARTPrint(ttext,sizeof(ttext)/sizeof(*ttext));
//	response = SDIO->RESP1;
//	for(int i = 0;i<32;i+=4)
//	{
//		uint8_t temp = (response%(1<<(i+4))/(1<<i));
//		if(temp>9)
//		{
//			RespText[sizeof(RespText)/sizeof(*RespText)-2-2-(i/4)]=temp+55;
//		}
//		else
//		{
//			RespText[sizeof(RespText)/sizeof(*RespText)-2-2-(i/4)]=temp+48;//ASCII convert
//		}
//		
//	}
//	USARTPrint(RespText,sizeof(RespText));
//	SdioCommand(41,3,0x40001000,temp);
	SDIO_Connect();
	//SdioCommand(9,0x3,SdioRca<<16,0);
	SDIO_Command(7,0x1,SDIO_Get_RCA()<<16,0);
	SDIO->DCTRL|=1<<8;
	SDIO_Command(17,0x1,1,0);
	uint32_t data = SDIO->FIFO;
	data = SDIO->FIFO;
	while(1)
	{
			USARTPrint(textt,sizeof(textt)/sizeof(*textt)-1);
			Delay(1000);
//		SDIO->CMD|=SDIO_CMD_CPSMEN;
//		delay(10);
//		SDIO->CMD&=~SDIO_CMD_CPSMEN;
//		delay(10);
//		if(!(SDIO->STA&SDIO_STA_CTIMEOUT))
//		{
//			GPIOA->BSRR|=GPIO_BSRR_BS1;
//		}
//		SDIO->ICR=0xFFFFFFFF;
//		delay(1);
		
	}
}
