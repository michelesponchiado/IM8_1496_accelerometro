/*
 * flash.c
 *
 *  Created on: Apr 10, 2017
 *      Author: michele
 */

#include "board.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum eIAP_COMMANDS
{
  IAP_PREPARE = 50,       // Prepare sector(s) for write operation
  IAP_COPY_RAM2FLASH,     // Copy RAM to Flash
  IAP_ERASE,              // Erase sector(s)
  IAP_BLANK_CHECK,        // Blank check sector(s)
  IAP_READ_PART_ID,       // Read chip part ID
  IAP_READ_BOOT_VER,      // Read chip boot code version
  IAP_COMPARE,            // Compare memory areas
  IAP_REINVOKE_ISP,       // Reinvoke ISP
  IAP_READ_UID,           // Read unique ID
};

struct sIAP
{
  uint32_t cmd;           // Command
  uint32_t par[4];        // Parameters
  uint32_t stat;          // Status
  uint32_t res[4];        // Result
};


/* IAP Call */
typedef void (*IAP_Entry) (uint32_t *cmd, uint32_t *stat);
#define IAP_Call ((IAP_Entry) 0x1FFF1FF1)

#define FLASH_32        (32)
#define FLASH_24        (24)
#define FLASH_16        (16)
#define FLASH_8         (8)

// Specify Flash type here
#define FLASH_TYPE    FLASH_32

#if FLASH_TYPE == FLASH_32
#define END_SECTOR      (7)
#define MIRROR_OFFSET   (0x00008000UL)
#elif FLASH_TYPE == FLASH_24
#define END_SECTOR      (5)
#define MIRROR_OFFSET   (0x00008000UL)
#elif FLASH_TYPE == FLASH_16
#define END_SECTOR      (3)
#define MIRROR_OFFSET   (0x00008000UL)
#elif FLASH_TYPE == FLASH_8
#define END_SECTOR      (1)
#define MIRROR_OFFSET   (0x00008000UL)
#else
#error Undefined Flash type
#endif

#define VERS            (1)     // Interface Version 1.01

#define UNKNOWN         (0)     // Unknown
#define ONCHIP          (1)     // On-chip Flash Memory
#define EXT8BIT         (2)     // External Flash Device on 8-bit  Bus
#define EXT16BIT        (3)     // External Flash Device on 16-bit Bus
#define EXT32BIT        (4)     // External Flash Device on 32-bit Bus
#define EXTSPI          (5)     // External Flash Device on SPI

#define SECTOR_NUM      (512)   // Max Number of Sector Items
#define PAGE_MAX        (65536) // Max Page Size for Programming

struct FlashSectors  {
  uint32_t        szSector;     // Sector Size in Bytes
  uint32_t      AddrSector;     // Address of Sector
};

#define SECTOR_END 0xFFFFFFFF, 0xFFFFFFFF

struct FlashDevice  {
   uint16_t           Vers;     // Version Number and Architecture
   char       DevName[128];     // Device Name and Description
   uint16_t        DevType;     // Device Type: ONCHIP, EXT8BIT, EXT16BIT, ...
   uint32_t         DevAdr;     // Default Device Start Address
   uint32_t          szDev;     // Total Size of Device
   uint32_t         szPage;     // Programming Page Size
   uint32_t            Res;     // Reserved for future Extension
   uint8_t        valEmpty;     // Content of Erased Memory

   uint32_t         toProg;     // Time Out of Program Page Function
   uint32_t        toErase;     // Time Out of Erase Sector Function

   struct FlashSectors sectors[SECTOR_NUM];
};

#define FLASH_DRV_VERS (0x0100+VERS)   // Driver Version, do not modify!

int EraseChip(void);
int EraseSector(uint32_t adr);
int BlankCheckChip(void);
int BlankCheckSector(uint32_t adr);
int Program(uint32_t adr, uint32_t sz, uint8_t *buf);





#if FLASH_TYPE == FLASH_32
struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "LPC11xx/13xx IAP 32kB Flash", // Device Name
   ONCHIP,                     // Device Type
   0x00000000,                 // Device Start Address
   0x00008000,                 // Device Size (32kB)
   1024,                       // Programming Page Size
   0,                          // Reserved, must be 0
   0xFF,                       // Initial Content of Erased Memory
   100,                        // Program Page Timeout 100 mSec
   3000,                       // Erase Sector Timeout 3000 mSec

// Specify Size and Address of Sectors
   0x001000, 0x000000,         // Sector Size  4kB (8 Sectors)
   SECTOR_END
};

#elif FLASH_TYPE == FLASH_24
struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "LPC11xx/13xx IAP 24kB Flash", // Device Name
   ONCHIP,                     // Device Type
   0x00000000,                 // Device Start Address
   0x00006000,                 // Device Size (24kB)
   1024,                       // Programming Page Size
   0,                          // Reserved, must be 0
   0xFF,                       // Initial Content of Erased Memory
   100,                        // Program Page Timeout 100 mSec
   3000,                       // Erase Sector Timeout 3000 mSec

// Specify Size and Address of Sectors
   0x001000, 0x000000,         // Sector Size  4kB (6 Sectors)
   SECTOR_END
};

#elif FLASH_TYPE == FLASH_16
struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "LPC11xx/13xx IAP 16kB Flash", // Device Name
   ONCHIP,                     // Device Type
   0x00000000,                 // Device Start Address
   0x00004000,                 // Device Size (16kB)
   1024,                       // Programming Page Size
   0,                          // Reserved, must be 0
   0xFF,                       // Initial Content of Erased Memory
   100,                        // Program Page Timeout 100 mSec
   3000,                       // Erase Sector Timeout 3000 mSec

// Specify Size and Address of Sectors
   0x001000, 0x000000,         // Sector Size  4kB (4 Sectors)
   SECTOR_END
};

#elif FLASH_TYPE == FLASH_24
struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "LPC11xx/13xx IAP 8kB Flash",  // Device Name
   ONCHIP,                     // Device Type
   0x00000000,                 // Device Start Address
   0x00002000,                 // Device Size (8kB)
   1024,                       // Programming Page Size
   0,                          // Reserved, must be 0
   0xFF,                       // Initial Content of Erased Memory
   100,                        // Program Page Timeout 100 mSec
   3000,                       // Erase Sector Timeout 3000 mSec

// Specify Size and Address of Sectors
   0x001000, 0x000000,         // Sector Size  4kB (2 Sectors)
   SECTOR_END
};
#endif


#define SET_VALID_CODE		0           // Set Valid User Code Signature

static struct sIAP IAP;

/*
 * Get Sector Number
 *    Parameter:      adr:  Sector Address
 *    Return Value:   Sector Number
 */

uint32_t GetSecNum (uint32_t adr)
{
  uint32_t n;

  n = adr >> 12;                               //  4kB Sector
  if (n >= 0x10) {
    n = 0x0E + (n >> 3);                       // 32kB Sector
  }

  return (n);                                  // Sector Number
}


/*
 *  Erase complete Flash Memory
 *    Return Value:   0 - OK,  otherwise - Failed
 */

int EraseChip (void)
{

  IAP.cmd    = IAP_PREPARE;                    // Prepare Sector for Erase
  IAP.par[0] = 0;                              // Start Sector
  IAP.par[1] = END_SECTOR;                     // End Sector
  IAP_Call (&IAP.cmd, &IAP.stat);              // Call IAP Command
  if (IAP.stat) return (IAP.stat);             // Command Failed

  IAP.cmd    = IAP_ERASE;                      // Erase Sector
  IAP.par[0] = 0;                              // Start Sector
  IAP.par[1] = END_SECTOR;                     // End Sector
  IAP.par[2] = SystemCoreClock / 1000;         // CCLK in kHz
  IAP_Call (&IAP.cmd, &IAP.stat);              // Call IAP Command
  if (IAP.stat) return (IAP.stat);             // Command Failed

  return (0);                                  // Finished without Errors
}


/*
 *  Erase Sector in Flash Memory
 *    Parameter:      adr:  Sector Address
 *    Return Value:   0 - OK,  1 - Failed
 */

int EraseSector (uint32_t adr)
{
  uint32_t n;

  n = GetSecNum(adr);                          // Get Sector Number

  IAP.cmd    = IAP_PREPARE;                    // Prepare Sector for Erase
  IAP.par[0] = n;                              // Start Sector
  IAP.par[1] = n;                              // End Sector
  IAP_Call (&IAP.cmd, &IAP.stat);              // Call IAP Command
  if (IAP.stat) return (IAP.stat);             // Command Failed

  IAP.cmd    = IAP_ERASE;                      // Erase Sector
  IAP.par[0] = n;                              // Start Sector
  IAP.par[1] = n;                              // End Sector
  IAP.par[2] = SystemCoreClock / 1000;         // CCLK in kHz
  IAP_Call (&IAP.cmd, &IAP.stat);              // Call IAP Command
  if (IAP.stat) return (IAP.stat);             // Command Failed

  return (0);                                  // Finished without Errors
}


/*
 *  Blank Check Chip Flash Memory
 *    Return Value:   0 - OK,  1 - Failed
 */

int BlankCheckChip (void)
{
  IAP.cmd    = IAP_BLANK_CHECK;                // Erase Sector
  IAP.par[0] = 0;                              // Start Sector
  IAP.par[1] = END_SECTOR;                     // End Sector
  IAP_Call (&IAP.cmd, &IAP.stat);              // Call IAP Command
  if (IAP.stat) return (IAP.stat);             // Command Failed

  return (0);                                  // Finished without Errors
}


/*
 *  Blank Check Sector in Flash Memory
 *    Parameter:      adr:  Sector Address
 *    Return Value:   0 - OK,  1 - Failed
 */

int BlankCheckSector (uint32_t adr)
{
  uint32_t n;

  n = GetSecNum(adr);                          // Get Sector Number

  IAP.cmd    = IAP_BLANK_CHECK;                // Erase Sector
  IAP.par[0] = n;                              // Start Sector
  IAP.par[1] = n;                              // End Sector
  IAP_Call (&IAP.cmd, &IAP.stat);              // Call IAP Command
  if (IAP.stat) return (IAP.stat);             // Command Failed

  return (0);                                  // Finished without Errors
}


/*
 *  Program Data in Flash Memory
 *    Parameter:      adr:  Destination Start Address
 *                    sz:   Data Size
 *                    buf:  Source Data
 *    Return Value:   0 - OK,  1 - Failed
 */

int Program (uint32_t adr, uint32_t sz, uint8_t *buf)
{
  uint32_t n;

#if SET_VALID_CODE != 0                        // Set valid User Code Signature
  if (adr == 0) {                              // Check for Vector Table
    n = *((uint32_t *)(buf + 0x00)) +
        *((uint32_t *)(buf + 0x04)) +
        *((uint32_t *)(buf + 0x08)) +
        *((uint32_t *)(buf + 0x0C)) +
        *((uint32_t *)(buf + 0x10)) +
        *((uint32_t *)(buf + 0x14)) +
        *((uint32_t *)(buf + 0x18));
    *((uint32_t *)(buf + 0x1C)) = 0 - n;  // Signature at Reserved Vector
  }
#endif

  n = GetSecNum(adr);                          // Get Sector Number

  IAP.cmd    = IAP_PREPARE;                    // Prepare Sector for Write
  IAP.par[0] = n;                              // Start Sector
  IAP.par[1] = n;                              // End Sector
  IAP_Call (&IAP.cmd, &IAP.stat);              // Call IAP Command
  if (IAP.stat) return (IAP.stat);             // Command Failed

  IAP.cmd    = IAP_COPY_RAM2FLASH;             // Copy RAM to Flash
  IAP.par[0] = adr;                            // Destination Flash Address
  IAP.par[1] = (uint32_t)buf;                  // Source RAM Address
  IAP.par[2] = sz;                             // Number of bytes
  IAP.par[3] = SystemCoreClock / 1000;         // CCLK in kHz
  IAP_Call (&IAP.cmd, &IAP.stat);              // Call IAP Command
  if (IAP.stat) return (IAP.stat);             // Command Failed

  return (0);                                  // Finished without Errors
}

uint32_t ReadIdentification(void)
{
  IAP.cmd = IAP_READ_PART_ID;
  IAP_Call (&IAP.cmd, &IAP.stat);
  if (IAP.stat) return (0);

  return (IAP.res[0]);
}

uint32_t ReadBootcodeVersion(void)
{
  IAP.cmd = IAP_READ_BOOT_VER;
  IAP_Call (&IAP.cmd, &IAP.stat);
  if (IAP.stat) return (0);

  return (IAP.res[0]);
}

int ReadUID(uint32_t * uid)
{
  if (uid == NULL)
    return 1;

  IAP.cmd = IAP_READ_UID;
  IAP_Call (&IAP.cmd, &IAP.stat);
  if (IAP.stat) return (1);

  uid[0] = IAP.res[0];
  uid[1] = IAP.res[1];
  uid[2] = IAP.res[2];
  uid[3] = IAP.res[3];
  return (0);
}

#if 0
static int mcmp(const void * src1, const void * src2, unsigned long size)
{
  char * s1 = (char *)src1;
  char * s2 = (char *)src2;

  while (size--)
    if (*(s1++) != *(s2++))
      return 1; /* mismatch found */
  return 0; /* src1 matches src2 */
}


void test_flash(void)
{

	volatile int DevID;
	static struct sIAP IAP;
	  uint32_t uid[4];
	  volatile uint32_t i = 0;


	  IAP.cmd = 0;
	  IAP.stat = 0;

	  i = ReadIdentification();
	  if (i != 0x2c40102b) while (1);
	  i = ReadBootcodeVersion();
	  if (i != 0x00000501) while (1);
	  DevID = ReadUID(uid);

	  if (EraseSector(0x7000)) while (1);
	  if (BlankCheckSector(0x7000)) while (1);

	  uint32_t test_write_sector[256]= // must be an integer multiple of 256
	  {
			  0x12345678,
			  0xabbacdde,
			  0x98765432,
	  };
	  if (Program(0x7000, sizeof(test_write_sector), (uint8_t*)test_write_sector)) while (1);
	  if (mcmp((void *)(0x00007000 + MIRROR_OFFSET), test_write_sector, sizeof(test_write_sector))) while (1);

}
#endif


uint8_t * get_ptr_flash_data(uint32_t offset)
{
	if (offset >= 4 * 1024)
	{
		return NULL;
	}
	return (uint8_t *)(0x00007000 + MIRROR_OFFSET + offset);
}
