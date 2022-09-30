/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2007        */
/*-----------------------------------------------------------------------*/
/* This is a stub disk I/O module that acts as front end of the existing */
/* disk I/O modules and attach it to FatFs module with common interface. */
/*-----------------------------------------------------------------------*/

#include "hal.h"
#include "ffconf.h"
#include "ff.h"
#include "diskio.h"

/* TODO: add flash in this error check. */
//#if HAL_USE_MMC_SPI && HAL_USE_SDC
//#error "cannot specify both MMC_SPI and SDC drivers"
//#endif

#if !defined(FATFS_HAL_DEVICE)
#if HAL_USE_MMC_SPI
#define FATFS_HAL_DEVICE MMCD1
#elif HAL_USE_SDC
#define FATFS_HAL_DEVICE SDCD1
#elif HAL_USE_SPI
#include "hal_serial_nor.h"
#define FATFS_HAL_DEVICE snor1
#define SNOR_SECTOR_SIZE 4096u
#endif
#endif

#if HAL_USE_MMC_SPI
extern MMCDriver FATFS_HAL_DEVICE;
#elif HAL_USE_SDC
extern SDCDriver FATFS_HAL_DEVICE;
#elif HAL_USE_SPI
extern SNORDriver FATFS_HAL_DEVICE;
#else
#error "MMC_SPI, SDC or FLASH driver must be specified"
#endif

#if HAL_USE_RTC
extern RTCDriver RTCD1;
#endif

/*-----------------------------------------------------------------------*/
/* Correspondence between physical drive number and physical drive.      */

#define MMC     0
#define SDC     0
#define SNOR    0


/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */

DSTATUS disk_initialize (
    BYTE pdrv         /* Physical drive number (0..) */
)
{
  DSTATUS stat;

  switch (pdrv) {
#if HAL_USE_MMC_SPI
  case MMC:
    stat = 0;
    /* It is initialized externally, just reads the status.*/
    if (blkGetDriverState(&FATFS_HAL_DEVICE) != BLK_READY)
      stat |= STA_NOINIT;
    if (mmcIsWriteProtected(&FATFS_HAL_DEVICE))
      stat |=  STA_PROTECT;
    return stat;
#elif HAL_USE_SDC
  case SDC:
    stat = 0;
    /* It is initialized externally, just reads the status.*/
    if (blkGetDriverState(&FATFS_HAL_DEVICE) != BLK_READY)
      stat |= STA_NOINIT;
    if (sdcIsWriteProtected(&FATFS_HAL_DEVICE))
      stat |=  STA_PROTECT;
    return stat;
#elif HAL_USE_SPI
  case SNOR:
    stat = 0;
    /* It is initialized externally, just reads the status.*/
    if (FATFS_HAL_DEVICE.state != FLASH_READY)
      stat |= STA_NOINIT;
    return stat;
#endif
  }
  return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Return Disk Status                                                    */

DSTATUS disk_status (
    BYTE pdrv         /* Physical drive number (0..) */
)
{
  DSTATUS stat;

  switch (pdrv) {
#if HAL_USE_MMC_SPI
  case MMC:
    stat = 0;
    /* It is initialized externally, just reads the status.*/
    if (blkGetDriverState(&FATFS_HAL_DEVICE) != BLK_READY)
      stat |= STA_NOINIT;
    if (mmcIsWriteProtected(&FATFS_HAL_DEVICE))
      stat |= STA_PROTECT;
    return stat;
#elif HAL_USE_SDC
  case SDC:
    stat = 0;
    /* It is initialized externally, just reads the status.*/
    if (blkGetDriverState(&FATFS_HAL_DEVICE) != BLK_READY)
      stat |= STA_NOINIT;
    if (sdcIsWriteProtected(&FATFS_HAL_DEVICE))
      stat |= STA_PROTECT;
    return stat;
#elif HAL_USE_SPI
  case SNOR:
    stat = 0;
    if (FATFS_HAL_DEVICE.state != FLASH_READY)
      stat |= STA_NOINIT;
    return stat;
#endif
  }
  return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */

DRESULT disk_read (
    BYTE pdrv,        /* Physical drive number (0..) */
    BYTE *buff,       /* Data buffer to store read data */
    DWORD sector,     /* Sector address (LBA) */
    UINT count        /* Number of sectors to read (1..255) */
)
{
  switch (pdrv) {
#if HAL_USE_MMC_SPI
  case MMC:
    if (blkGetDriverState(&FATFS_HAL_DEVICE) != BLK_READY)
      return RES_NOTRDY;
    if (mmcStartSequentialRead(&FATFS_HAL_DEVICE, sector))
      return RES_ERROR;
    while (count > 0) {
      if (mmcSequentialRead(&FATFS_HAL_DEVICE, buff))
        return RES_ERROR;
      buff += MMCSD_BLOCK_SIZE;
      count--;
    }
    if (mmcStopSequentialRead(&FATFS_HAL_DEVICE))
        return RES_ERROR;
    return RES_OK;
#elif HAL_USE_SDC
  case SDC:
    if (blkGetDriverState(&FATFS_HAL_DEVICE) != BLK_READY)
      return RES_NOTRDY;
    if (sdcRead(&FATFS_HAL_DEVICE, sector, buff, count))
      return RES_ERROR;
    return RES_OK;
#elif HAL_USE_SPI
  case SNOR:
    if (FATFS_HAL_DEVICE.state != FLASH_READY) {
      return RES_NOTRDY;
    }

    //const flash_descriptor_t *flash_desc;
    //flash_desc = flashGetDescriptor((BaseFlash *)&FATFS_HAL_DEVICE);

    while (count > 0) {
      if (flashRead((BaseFlash *)&FATFS_HAL_DEVICE, sector * SNOR_SECTOR_SIZE, SNOR_SECTOR_SIZE, buff) != FLASH_NO_ERROR) {
        return RES_ERROR;
      }
      sector++;
      count--;
      buff += SNOR_SECTOR_SIZE;
    }
    return RES_OK;
#endif
  }
  return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */

#if !FF_FS_READONLY
DRESULT disk_write (
    BYTE pdrv,        /* Physical drive number (0..) */
    const BYTE *buff, /* Data to be written */
    DWORD sector,     /* Sector address (LBA) */
    UINT count        /* Number of sectors to write (1..255) */
)
{
  switch (pdrv) {
#if HAL_USE_MMC_SPI
  case MMC:
    if (blkGetDriverState(&FATFS_HAL_DEVICE) != BLK_READY)
        return RES_NOTRDY;
    if (mmcIsWriteProtected(&FATFS_HAL_DEVICE))
        return RES_WRPRT;
    if (mmcStartSequentialWrite(&FATFS_HAL_DEVICE, sector))
        return RES_ERROR;
    while (count > 0) {
        if (mmcSequentialWrite(&FATFS_HAL_DEVICE, buff))
            return RES_ERROR;
        buff += MMCSD_BLOCK_SIZE;
        count--;
    }
    if (mmcStopSequentialWrite(&FATFS_HAL_DEVICE))
        return RES_ERROR;
    return RES_OK;
#elif HAL_USE_SDC
  case SDC:
    if (blkGetDriverState(&FATFS_HAL_DEVICE) != BLK_READY)
      return RES_NOTRDY;
    if (sdcWrite(&FATFS_HAL_DEVICE, sector, buff, count))
      return RES_ERROR;
    return RES_OK;
#elif HAL_USE_SPI
  case SNOR:
    if (FATFS_HAL_DEVICE.state != FLASH_READY) {
      return RES_NOTRDY;
    }

    const flash_descriptor_t *flash_desc;
    flash_desc = flashGetDescriptor((BaseFlash *)&FATFS_HAL_DEVICE);

    while (count > 0) {
      flashStartEraseSector((BaseFlash *)&FATFS_HAL_DEVICE, sector);
      if (flashWaitErase((BaseFlash *)&FATFS_HAL_DEVICE) != FLASH_NO_ERROR) {
        return RES_ERROR;
      }
      if (flashVerifyErase((BaseFlash *)&FATFS_HAL_DEVICE, sector) != FLASH_NO_ERROR) {
        return RES_ERROR;
      }

      if (flashProgram((BaseFlash *)&FATFS_HAL_DEVICE, sector * flash_desc->sectors_size, flash_desc->sectors_size, buff) != FLASH_NO_ERROR) {
        return RES_ERROR;
      }
      sector++;
      count--;
      buff += flash_desc->sectors_size;
    }
    return RES_OK;
#endif
  }
  return RES_PARERR;
}
#endif /* _FS_READONLY */



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */

DRESULT disk_ioctl (
    BYTE pdrv,        /* Physical drive number (0..) */
    BYTE cmd,         /* Control code */
    void *buff        /* Buffer to send/receive control data */
)
{
  (void)buff;

#if HAL_USE_SPI
  const flash_descriptor_t *flash_desc;
  flash_desc = flashGetDescriptor((BaseFlash *)&FATFS_HAL_DEVICE);
#endif

  switch (pdrv) {
#if HAL_USE_MMC_SPI
  case MMC:
    switch (cmd) {
    case CTRL_SYNC:
        return RES_OK;
#if FF_MAX_SS > FF_MIN_SS
    case GET_SECTOR_SIZE:
        *((WORD *)buff) = MMCSD_BLOCK_SIZE;
        return RES_OK;
#endif
#if FF_USE_TRIM
    case CTRL_TRIM:
        mmcErase(&FATFS_HAL_DEVICE, *((DWORD *)buff), *((DWORD *)buff + 1));
        return RES_OK;
#endif
    default:
        return RES_PARERR;
    }
#elif HAL_USE_SDC
  case SDC:
    switch (cmd) {
    case CTRL_SYNC:
        return RES_OK;
    case GET_SECTOR_COUNT:
        *((DWORD *)buff) = mmcsdGetCardCapacity(&FATFS_HAL_DEVICE);
        return RES_OK;
#if FF_MAX_SS > FF_MIN_SS
    case GET_SECTOR_SIZE:
        *((WORD *)buff) = MMCSD_BLOCK_SIZE;
        return RES_OK;
#endif
    case GET_BLOCK_SIZE:
        *((DWORD *)buff) = 256; /* 512b blocks in one erase block */
        return RES_OK;
#if FF_USE_TRIM
    case CTRL_TRIM:
        sdcErase(&FATFS_HAL_DEVICE, *((DWORD *)buff), *((DWORD *)buff + 1));
        return RES_OK;
#endif

#elif HAL_USE_SPI
  case SNOR:

    switch (cmd) {

    case CTRL_SYNC:
      return RES_OK;

    case GET_SECTOR_COUNT:
//      const flash_descriptor_t *flash_desc;
//      flash_desc = flashGetDescriptor((BaseFlash *)&FATFS_HAL_DEVICE);
      /* TODO: test. */
//      *((DWORD *)buff) = flash_desc->sectors_count;
      *((DWORD *)buff) = flash_desc->size / SNOR_SECTOR_SIZE;
      return RES_OK;

#if FF_MAX_SS > FF_MIN_SS
    case GET_SECTOR_SIZE:
//      const flash_descriptor_t *flash_desc;
//      flash_desc = flashGetDescriptor((BaseFlash *)&FATFS_HAL_DEVICE);
      /* TODO: test. */
//      *((DWORD *)buff) = flash_desc->sectors_size;
      *((DWORD *)buff) = SNOR_SECTOR_SIZE;
      return RES_OK;
#endif

    case GET_BLOCK_SIZE:
//      const flash_descriptor_t *flash_desc;
//      flash_desc = flashGetDescriptor((BaseFlash *)&FATFS_HAL_DEVICE);
      /* TODO: test. */
//      *((DWORD *)buff) = flash_desc->sectors_size;
      *((DWORD *)buff) = 1;
      return RES_OK;

#if FF_USE_TRIM
    case CTRL_TRIM:
      start_block = *((DWORD *)buff);
      end_block = *((DWORD *)buff + 1);

      while (start_block < end_block) {
        if (flashStartEraseSector((BaseFlash *)&FATFS_HAL_DEVICE, start_block) != FLASH_NO_ERROR) {
          return RES_ERROR;
        }
        if (flashWaitErase((BaseFlash *)&FATFS_HAL_DEVICE) != FLASH_NO_ERROR) {
          return RES_ERROR;
        }
        start_block++;
      }
      return RES_OK;
#endif

    default:
      return RES_PARERR;
    }
#endif
  }
  return RES_PARERR;
}

DWORD get_fattime(void) {
#if HAL_USE_RTC
    RTCDateTime timespec;

    rtcGetTime(&RTCD1, &timespec);
    return rtcConvertDateTimeToFAT(&timespec);
#else
    return ((uint32_t)0 | (1 << 16)) | (1 << 21); /* wrong but valid time */
#endif
}
