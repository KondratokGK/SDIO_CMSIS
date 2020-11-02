#include "stm32f4xx.h"
#include "system_stm32F4xx.h"
#include "stdlib.h"
#include "cmsis_delay.h"
#include "sdio.h"

#define INIT_FREQ 400000
#define SDIO_INPUT_FREQ 48000000
#define BLOCK_SIZE 9 //2^9=512

uint16_t SDIO_RCA;

uint16_t SDIO_Get_RCA()
{
	return SDIO_RCA;
}

void SDIO_Init()
{
	uint32_t tempreg;
	uint8_t alternate_GPIOC[]={8,9,10,11,12};
	uint8_t alternate_GPIOD[]={2};
	RCC->APB2ENR |= RCC_APB2ENR_SDIOEN; 	// SDIO clock enable
	SDIO->POWER=SDIO_POWER_PWRCTRL_Msk; 	//SDIO Power up
	
	RCC->AHB1ENR|=RCC_AHB1ENR_DMA2EN;			//DMA clock enable
	SDIO->DCTRL|=SDIO_DCTRL_DMAEN;				//DMA SDIO enable
	
	tempreg=0;
	tempreg|=SDIO_CLKCR_CLKEN; 						//SDIO_CK clock enable
	tempreg|=(0x0<<SDIO_CLKCR_WIDBUS_Pos);//1 wire bus enable
	tempreg|=(((SDIO_INPUT_FREQ/INIT_FREQ)-2)<<SDIO_CLKCR_CLKDIV_Pos);//CLK devider 118
	SDIO->CLKCR=tempreg;
	
	for(int i = 0;i<5;i++)
	{
		GPIOC->MODER|=0x2<<(alternate_GPIOC[i]*2); //Alternate function
		GPIOC->PUPDR|=0x1<<(alternate_GPIOC[i]*2); //Pull-up
		GPIOC->OTYPER|=0x0<<alternate_GPIOC[i]; //Push-Pull
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
	
	
	
	
	for(int i = 0;i<1;i++)
	{
		GPIOD->MODER|=0x2<<alternate_GPIOD[i]*2; //Alternate function
		GPIOD->PUPDR|=0x1<<(alternate_GPIOD[i]*2); //Pull-up
		GPIOD->OTYPER|=0x0<<alternate_GPIOD[i];//open drain
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
	tempreg = 0;

	SDIO->DCTRL|=(BLOCK_SIZE<<SDIO_DCTRL_DBLOCKSIZE_Pos); //Block size 2^12=4096
	
}
uint8_t SDIO_Command(uint8_t cmd, uint8_t RespType, uint32_t argument, uint32_t response[4])
{
	if((cmd>>6))
	{
		return 0xFF;
	}
	else
	{
		uint32_t tempreg=0;
		SDIO->ICR=0xFFFFFFFF;
		Delay(1);
		SDIO->ARG=argument;
		Delay(1);
		tempreg|=RespType<<SDIO_CMD_WAITRESP_Pos;
		tempreg|=cmd<<SDIO_CMD_CMDINDEX_Pos;
		tempreg|=SDIO_CMD_CPSMEN;
		SDIO->CMD=tempreg;
		while((SDIO->STA&SDIO_STA_CMDACT));
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
				
			}
			else
			{
				
			}
		}
		return 0;
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
	
	uint32_t tempreg;
	
//	tempreg=SDIO->CLKCR;
//	tempreg&=~SDIO_CLKCR_CLKDIV_Msk;
//	tempreg|=46<<SDIO_CLKCR_CLKDIV_Pos;
//	SDIO->CLKCR=tempreg;
	
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
	uint32_t tempreg; //Create template register
	uint8_t data[512]={0};
	
	
	//DMA config
	DMA2_Stream3->CR&=~DMA_SxCR_EN;
	tempreg=
	4<<DMA_SxCR_CHSEL_Pos|		//Channel 4
	1<<DMA_SxCR_MBURST_Pos|		//4 burst memory mode
	1<<DMA_SxCR_PBURST_Pos|		//4 burst periphal mode
	0<<DMA_SxCR_DBM_Pos|			//Double buffer disable
	3<<DMA_SxCR_PL_Pos|				//Very high priotity
	0<<DMA_SxCR_PINCOS_Pos|		
	2<<DMA_SxCR_MSIZE_Pos|		//Memory size word
	2<<DMA_SxCR_PSIZE_Pos|		//Periphal size word
	1<<DMA_SxCR_MINC_Pos|			//Memory increment enable
	0<<DMA_SxCR_PINC_Pos|			//Periphal increment disable
	0<<DMA_SxCR_CIRC_Pos|			//Circular mode enable
	0<<DMA_SxCR_DIR_Pos|			//From periphal to memory
	1<<DMA_SxCR_PFCTRL_Pos|		//Periphal is flow controller
	0<<DMA_SxCR_TCIE_Pos|			//Interrupt disable
	0<<DMA_SxCR_HTIE_Pos|			//Interrupt disable
	0<<DMA_SxCR_TEIE_Pos|			//Interrupt disable
	0<<DMA_SxCR_DMEIE_Pos;		//Interrupt disable
	DMA2_Stream3->CR=tempreg;
	tempreg=0;
	
	tempreg=
	0<<DMA_SxFCR_FEIE_Pos|		//FIFO Interrupt disable
	1<<DMA_SxFCR_DMDIS_Pos|		//Use FIFO
	3<<DMA_SxFCR_FTH_Pos;			//Full FIFO
	DMA2_Stream3->FCR=tempreg;
	tempreg=0;
	
	DMA2_Stream3->NDTR=512/4;
	
	DMA2_Stream3->PAR=0x40012C80;
	
	DMA2_Stream3->M0AR=(uint32_t)data;
	
	DMA2_Stream3->CR|=DMA_SxCR_EN;
	
	
	SDIO_Command(7,0x1,0,0); 
	SDIO_Command(7,0x1,SDIO_Get_RCA()<<16,0); //Select sd card
	
	SDIO->DLEN=512; //select data length
	
	SDIO->DTIMER=SDIO_DTIMER_DATATIME_Msk; //Set data timer
	
	tempreg=0;
	tempreg|=(uint32_t) 9 << 4; //block size 512 
	tempreg|=SDIO_DCTRL_DTDIR; //Data direction from card to controller
	tempreg|=SDIO_DCTRL_DMAEN; //DMA enable
	tempreg|=SDIO_DCTRL_DTEN; //Data enable
	SDIO->DCTRL=tempreg;
	
	SDIO_Command(17,0x1,0x000000000,0); //Get data
	//TODO: DMA read from buffer

	//TODO: DMA STOP
	while((SDIO->STA&SDIO_STA_DATAEND)!=0){};
	return 0;
}

int SDIO_disk_write(const BYTE *buff, LBA_t sector, UINT count)
{
	SDIO_Command(7,0x1,0,0);
	SDIO_Command(7,0x1,SDIO_Get_RCA()<<16,0);
	return 0;
}
