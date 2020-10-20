#include "stm32f4xx.h"
#include "system_stm32F4xx.h"
#include "stdlib.h"
#define HSE_VALUE    ((uint32_t)8000000)

_Bool InitDelayFlag = 0;
volatile uint64_t DelayCounter = 0;
uint32_t OldCoreClock;

void SysTick_Handler(void) {
  DelayCounter++;
}


int InitDelay()
{
	
	SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk;
	SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
	SysTick_Config(SystemCoreClock/1000);
	OldCoreClock=SystemCoreClock;
	InitDelayFlag = 1;
	return 0;
	
}

void delay(const uint32_t milliseconds)
{
    SystemCoreClockUpdate();
		if(OldCoreClock!=SystemCoreClock) SysTick_Config(SystemCoreClock/1000);
		if(InitDelayFlag == 0) InitDelay();
		uint32_t start = DelayCounter;
    while((DelayCounter - start) < milliseconds);
}

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
	uint32_t baud = 9600; //baudrate
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

void SdioInit()
{
	
	RCC->APB2ENR |= RCC_APB2ENR_SDIOEN; // SDIO clock enable
	delay(1); //Wait
	SDIO->CLKCR|=(0x0<<SDIO_CLKCR_WIDBUS_Pos);//4 wire bus enable
	delay(1);
	SDIO->CLKCR|=(118<<SDIO_CLKCR_CLKDIV_Pos);//CLK devider 118
	SDIO->POWER=SDIO_POWER_PWRCTRL_Msk; //SDIO Power up
	delay(1);
	SDIO->CLKCR|=SDIO_CLKCR_CLKEN; //clock enable
	uint8_t alternate_GPIOC[]={8,9,10,11,12};
	for(int i = 0;i<5;i++)
	{
		GPIOC->MODER|=0x2<<(alternate_GPIOC[i]*2); //Alternate function
		GPIOC->PUPDR|=0x1<<(alternate_GPIOC[i]*2); //Pull-up
		GPIOC->OTYPER|=0x1<<alternate_GPIOC[i];
		delay(1);
		GPIOC->OSPEEDR|=0x2<<(alternate_GPIOC[i]*2); //high speed
		if(alternate_GPIOC[i]>7)
		{
			GPIOC->AFR[1]|=12<<((alternate_GPIOC[i]-8)*4); //alternate function AF12
		}
		else
		{
			GPIOC->AFR[0]|=12<<(alternate_GPIOC[i]*4); //alternate function AF12
		}
		
	}
	uint8_t alternate_GPIOD[]={2};
	for(int i = 0;i<1;i++)
	{
		GPIOD->MODER|=0x2<<alternate_GPIOD[i]*2; //Alternate function
		delay(1);
		GPIOD->PUPDR|=0x1<<(alternate_GPIOD[i]*2); //Pull-up
		delay(1);
		GPIOD->OTYPER|=0x1<<alternate_GPIOD[i];//open drain
		delay(1);
		GPIOD->OSPEEDR|=0x2<<(alternate_GPIOD[i]*2); //high speed
		if(alternate_GPIOD[i]>7)
		{
			GPIOD->AFR[1]|=12<<((alternate_GPIOC[i]-8)*4); //alternate function AF12
		}
		else
		{
			GPIOD->AFR[0]|=12<<(alternate_GPIOD[i]*4); //alternate function AF12
		}
		delay(1);
		
	}
	
	//SDIO->DCTRL|=(8<<SDIO_DCTRL_DBLOCKSIZE_Pos); //Block size 2^12=4096
	delay(1);

	//SDIO->CMD|=SDIO_CMD_CPSMEN; //CMD send enable
	
	
	
}
uint8_t SdioCommand(uint8_t cmd, uint8_t RespType, uint32_t argument, uint32_t response[4])
{
	if((cmd>>6))
	{
		return 0xFF;
	}
	else
	{
		SDIO->ICR=0xFFFFFFFF;
		delay(1); 
		SDIO->CMD&=~SDIO_CMD_CPSMEN;
		delay(1); 
		SDIO->ARG=argument;
		SDIO->CMD|=(RespType<<SDIO_CMD_WAITRESP_Pos);
		delay(1); 
		SDIO->CMD|=(cmd<<SDIO_CMD_CMDINDEX_Pos);
		delay(1); 
		SDIO->CMD|=SDIO_CMD_CPSMEN;
		delay(1);
		SDIO->CMD&=~SDIO_CMD_CPSMEN;
		while((SDIO->STA&SDIO_STA_CMDACT));
		if(RespType==0x0|RespType==0x2) return 0;
		else
		{
			if(RespType==0x1|RespType==0x3)
			{
				response[0]=SDIO->RESP1;
				response[1]=SDIO->RESP2;
				response[2]=SDIO->RESP3;
				response[3]=SDIO->RESP4;
			}
			else
			{
				return 0xFF;
			}
		}
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
	uint32_t temp[4];
	GpioInit();
	if(RccInit()==10) Fault_Handler();
	SdioInit();
	UsartInit();
	delay(100);
	SdioCommand(0,0,0,0);
	USARTPrint((uint8_t *)"Reset\r\n",7);
	delay(100);
	USARTPrint((uint8_t *)"Wait \r\n",7);
	SdioCommand(8,3,0x000001AA,temp);
	USARTPrint((uint8_t *)"Get  \r\n",7);
	delay(100);
	USARTPrint((uint8_t *)"Wait \r\n",7);
	uint8_t text[] = "SDIO   status:";
	USARTPrint(text,sizeof(text)/sizeof(*text));
	uint32_t response = SDIO->STA;
	uint8_t RespText[]="0x--------\r\n";
	for(int i = 0;i<32;i+=4)
	{
		uint8_t temp = (response%(1<<(i+4))/(1<<i));
		if(temp>9)
		{
			RespText[sizeof(RespText)/sizeof(*RespText)-2-2-(i/4)]=temp+55;
		}
		else
		{
			RespText[sizeof(RespText)/sizeof(*RespText)-2-2-(i/4)]=temp+48;//ASCII convert
		}
		
	}
	USARTPrint(RespText,sizeof(RespText));
	uint8_t ttext[] = "SDIO response:";
	USARTPrint(ttext,sizeof(ttext)/sizeof(*ttext));
	response = SDIO->RESP1;
	for(int i = 0;i<32;i+=4)
	{
		uint8_t temp = (response%(1<<(i+4))/(1<<i));
		if(temp>9)
		{
			RespText[sizeof(RespText)/sizeof(*RespText)-2-2-(i/4)]=temp+55;
		}
		else
		{
			RespText[sizeof(RespText)/sizeof(*RespText)-2-2-(i/4)]=temp+48;//ASCII convert
		}
		
	}
	USARTPrint(RespText,sizeof(RespText));
	while(1)
	{
		
		
	}
}
