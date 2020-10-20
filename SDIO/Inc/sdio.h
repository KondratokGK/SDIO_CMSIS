#pragma once

uint16_t SDIO_Get_RCA();

void SDIO_Init(void);

uint8_t SDIO_Command(uint8_t cmd, uint8_t RespType, uint32_t argument, uint32_t response[4]);

void SDIO_Connect();

