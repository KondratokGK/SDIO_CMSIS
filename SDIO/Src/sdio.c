#include "stm32f4xx.h"
#include "system_stm32F4xx.h"
#include "stdlib.h"
#include "cmsis_delay.h"
#include "sdio.h"

#define INIT_FREQ 200000
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
	uint8_t data[512];
	uint32_t dir=0;
	
	//Init DMA
	  DMA2_Stream3->CR=0;
  	
  //Clear all the flags
  DMA2->LIFCR=DMA_LIFCR_CTCIF3 | DMA_LIFCR_CTEIF3 | DMA_LIFCR_CDMEIF3 | DMA_LIFCR_CFEIF3 | DMA_LIFCR_CHTIF3;
  
  //Set the DMA Addresses
  DMA2_Stream3->PAR=((uint32_t) 0x40012C80);  //SDIO FIFO Address (=SDIO Base+0x80)
  DMA2_Stream3->M0AR=(uint32_t) *data;    //Memory address
  
  //Set the number of data to transfer
  DMA2_Stream3->NDTR=0;   //Peripheral controls, therefore we don't need to indicate a size
  
  //Set the DMA CR
  tempreg=0;
  tempreg|=(0x04<<25) & DMA_SxCR_CHSEL;  //Select Channel 4
  tempreg|=(0x01<<23) & DMA_SxCR_MBURST;  //4 beat memory burst (memory is 32word. Therefore, each time dma access memory, it reads 4*32 bits) (FIFO size must be integer multiple of memory burst)(FIFO is 4byte. Therefore we can only use 4 beat in this case)
  //Note: Ref manual (p173 (the node at the end of 8.3.11) says that burst mode is not allowed when Pinc=0. However, it appears that this is not true at all. Furthermore. when I set pBurst=0, the SDIO's dma control does not work at all.)
  tempreg|=(0x01<<21) & DMA_SxCR_PBURST;  //4 beat memory burst Mode ([Burst Size*Psize] must be equal to [FIFO size] to prevent FIFO underrun and overrun errors) (burst also does not work in direct mode).
  tempreg|=(0x00<<18) & DMA_SxCR_DBM;   //Disable double buffer mode (when this is set, circluar mode is also automatically set. (the actual value is don't care)
  tempreg|=(0x03<<16) & DMA_SxCR_PL;     //Priority is very_high
  tempreg|=(0x00<<15) & DMA_SxCR_PINCOS;  //Peripheral increment offset (if this is 1 and Pinc=1, then Peripheral will be incremented by 4 regardless of Psize)
  tempreg|=(0x02<<13) & DMA_SxCR_MSIZE;  //Memory data size is 32bit (word)
  tempreg|=(0x02<<11) & DMA_SxCR_PSIZE;  //Peripheral data size is 32bit (word)
  tempreg|=(0x01<<10) & DMA_SxCR_MINC;  //Enable Memory Increment
  tempreg|=(0x00<<9) & DMA_SxCR_MINC;  //Disable Peripheral Increment
  tempreg|=(0x00<<8) & DMA_SxCR_CIRC;   //Disable Circular mode
  //tempreg|=(0x00<<6) & DMA_SxCR_DIR;  //Direction 0:P2M, 1:M2P
  tempreg|=(0x01<<5) & DMA_SxCR_PFCTRL; //Peripheral controls the flow control. (The DMA tranfer ends when the data issues end of transfer signal regardless of ndtr value)
  //Bit [4..1] is for interupt mask. I don't use interrupts here
  //Bit 0 is EN. I will set it after I set the FIFO CR. (FIFO CR cannot be modified when EN=1)
  DMA2_Stream3->CR=tempreg;
	
	//Set the FIFO CR
  tempreg=0x21; //Reset value
  tempreg|=(0<<7); //FEIE is disabled
  tempreg|=(1<<2); //Fifo is enabled (Direct mode is disabled);
  tempreg|=3;   //Full fifo (Fifo threshold selection)
  DMA2_Stream3->FCR=tempreg;
  
  //Set the Direction of transfer
  if (dir==0x2) {
    DMA2_Stream3->CR|=(0x01<<6) & DMA_SxCR_DIR;
  } else if (dir==0x0) {
    DMA2_Stream3->CR|=(0x00<<6) & DMA_SxCR_DIR;
  }
  
  //Enable the DMA (When it is enabled, it starts to respond dma requests)
  DMA2_Stream3->CR|=DMA_SxCR_EN;
  //END of PART I
	
	
	
	SDIO_Command(7,0x1,0,0); 
	SDIO_Command(7,0x1,SDIO_Get_RCA()<<16,0); //Select sd card
	
	SDIO->DLEN=512; //select data length
	
	SDIO->DTIMER=SDIO_DTIMER_DATATIME_Msk; //Set data timer
	
	tempreg=0;
	tempreg|=(uint32_t) 9 << 4; //block size 512 
	tempreg|=SDIO_DCTRL_DTDIR; //Data direction from card to controller
	tempreg|=SDIO_DCTRL_DTEN; //Data enable
	SDIO->DCTRL=tempreg;  
	
	SDIO_Command(17,0x1,0x000100000,0); //Get data

	Delay(10);
	

	return 0;
}

int SDIO_disk_write(const BYTE *buff, LBA_t sector, UINT count)
{
	SDIO_Command(7,0x1,0,0);
	SDIO_Command(7,0x1,SDIO_Get_RCA()<<16,0);
	return 0;
}
