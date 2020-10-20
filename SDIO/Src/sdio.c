#include "stm32f4xx.h"
#include "system_stm32F4xx.h"
#include "stdlib.h"
#include "cmsis_delay.h"
#include "sdio.h"


uint16_t SDIO_RCA;

uint16_t SDIO_Get_RCA()
{
	return SDIO_RCA;
}

void SDIO_Init()
{
	
	RCC->APB2ENR |= RCC_APB2ENR_SDIOEN; // SDIO clock enable
	SDIO->POWER=SDIO_POWER_PWRCTRL_Msk; //SDIO Power up
	Delay(1);
	SDIO->CLKCR|=SDIO_CLKCR_CLKEN; //clock enable
	Delay(1);
	SDIO->CLKCR|=(0x1<<SDIO_CLKCR_WIDBUS_Pos);//4 wire bus enable
	Delay(1);
	SDIO->CLKCR|=(238<<SDIO_CLKCR_CLKDIV_Pos);//CLK devider 118
	Delay(1);
	uint8_t alternate_GPIOC[]={8,9,10,11,12};
	for(int i = 0;i<5;i++)
	{
		GPIOC->MODER|=0x2<<(alternate_GPIOC[i]*2); //Alternate function
		GPIOC->PUPDR|=0x1<<(alternate_GPIOC[i]*2); //Pull-up
		GPIOC->OTYPER|=0x1<<alternate_GPIOC[i];
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
		GPIOD->PUPDR|=0x1<<(alternate_GPIOD[i]*2); //Pull-up
		GPIOD->OTYPER|=0x1<<alternate_GPIOD[i];//open drain
		GPIOD->OSPEEDR|=0x2<<(alternate_GPIOD[i]*2); //high speed
		if(alternate_GPIOD[i]>7)
		{
			GPIOD->AFR[1]|=12<<((alternate_GPIOC[i]-8)*4); //alternate function AF12
		}
		else
		{
			GPIOD->AFR[0]|=12<<(alternate_GPIOD[i]*4); //alternate function AF12
		}
		Delay(1);
		
	}
	
	SDIO->DCTRL|=(9<<SDIO_DCTRL_DBLOCKSIZE_Pos); //Block size 2^12=4096
	Delay(1);

	//SDIO->CMD|=SDIO_CMD_CPSMEN; //CMD send enable
	
	
	
}
uint8_t SDIO_Command(uint8_t cmd, uint8_t RespType, uint32_t argument, uint32_t response[4])
{
	if((cmd>>6))
	{
		return 0xFF;
	}
	else
	{
		SDIO->ICR=0xFFFFFFFF;
		Delay(1);
		SDIO->CMD=0;
		Delay(1);
		SDIO->ARG=argument;
		Delay(1);
		SDIO->CMD|=RespType<<SDIO_CMD_WAITRESP_Pos;
		Delay(1);
		SDIO->CMD|=cmd<<SDIO_CMD_CMDINDEX_Pos;
		Delay(1);
		SDIO->CMD|=SDIO_CMD_CPSMEN;
		Delay(1);
		while((SDIO->STA&SDIO_STA_CMDACT));
		Delay(1);
		SDIO->CMD&=~SDIO_CMD_CPSMEN;
		if(RespType==0x0|RespType==0x2) return 0;
		else
		{
			if(RespType==0x1|RespType==0x3)
			{
				response[0]=SDIO->RESP1;
				response[1]=SDIO->RESP2;
				response[2]=SDIO->RESP3;
				response[3]=SDIO->RESP4;
				return 0;
			}
			else
			{
				return 0xFF;
			}
		}
	}
}

void SDIO_Connect()
{
	uint32_t resp[4];
	SDIO_Command(0,0,0,0);
	SDIO_Command(8,1,0x000001AA,resp);
	SDIO_Command(55,1,0x0,0);
	if(resp[0]==0x000001AA)
	{
		SDIO_Command(41,1,0x00000000|1<<30|0xFF80<<8,resp);
		while(!(resp[0]>>31))
		{
			SDIO_Command(55,1,0x0,0);
			Delay(20);
			SDIO_Command(41,1,0x00000000|1<<30|0xFF80<<8,resp);
			Delay(10);
		}
	}
	else
	{
		SDIO_Command(41,1,0x00000000|0xFF80<<8,resp);
		while(!(resp[0]>>31))
		{
			SDIO_Command(55,1,0x0,0);
			Delay(20);
			SDIO_Command(41,1,0x00000000|0xFF80<<8,resp);
			Delay(10);
		}
	}
	//SdioCommand(11,3,0,0);
	SDIO_Command(2,1,0,0);
	SDIO_Command(3,1,0,resp);
	SDIO_RCA=resp[0]>>16;
	if(SDIO->STA&SDIO_STA_CMDREND)
	{
		GPIOA->BSRR|=GPIO_BSRR_BS1;
	}
	SDIO->CLKCR&=~SDIO_CLKCR_CLKDIV_Msk;
	Delay(1);
	SDIO->CLKCR|=(118<<SDIO_CLKCR_CLKDIV_Pos);
	
}

int SDIO_disk_initialize()
{
	return 0;
}

int SDIO_disk_status()
{
	return 0;
}


int SDIO_disk_read(BYTE *buff, LBA_t sector, UINT count)
{
	return 0;
}

int SDIO_disk_write(const BYTE *buff, LBA_t sector, UINT count)
{
	return 0;
}
