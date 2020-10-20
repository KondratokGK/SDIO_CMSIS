#pragma once

#define FF_INTDEF 2
typedef unsigned int	UINT;	/* int must be 16-bit or 32-bit */
typedef unsigned char	BYTE;	/* char must be 8-bit */
typedef uint16_t		WORD;	/* 16-bit unsigned integer */
typedef uint32_t		DWORD;	/* 32-bit unsigned integer */
typedef uint64_t		QWORD;	/* 64-bit unsigned integer */
typedef WORD			WCHAR;	/* UTF-16 character type */

#define FF_LBA64		0
#define FF_FS_EXFAT		1

#if FF_FS_EXFAT
#if FF_INTDEF != 2
#error exFAT feature wants C99 or later
#endif
typedef QWORD FSIZE_t;
#if FF_LBA64
typedef QWORD LBA_t;
#else
typedef DWORD LBA_t;
#endif
#else
#if FF_LBA64
#error exFAT needs to be enabled when enable 64-bit LBA
#endif
typedef DWORD FSIZE_t;
typedef DWORD LBA_t;
#endif


uint16_t SDIO_Get_RCA();

void SDIO_Init(void);

uint8_t SDIO_Command(uint8_t cmd, uint8_t RespType, uint32_t argument, uint32_t response[4]);

void SDIO_Connect();

int SDIO_disk_initialize();

int SDIO_disk_status();

int SDIO_disk_read(BYTE *buff, LBA_t sector, UINT count);

int SDIO_disk_write(const BYTE *buff, LBA_t sector, UINT count);