/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/**
 * @file    hal_flash_device.c
 * @brief   Winbond W25Q serial flash driver code.
 *
 * @addtogroup WINBOND_W25Q
 * @{
 */

#include <string.h>

#include "hal.h"
#include "hal_serial_nor.h"

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

#define PAGE_SIZE                           256U
#define PAGE_MASK                           (PAGE_SIZE - 1U)

#if W25Q_USE_SUB_SECTORS == TRUE
#define SECTOR_SIZE                         0x00001000U
#define CMD_SECTOR_ERASE                    W25Q_CMD_SECTOR_ERASE_4KB
#else
#define SECTOR_SIZE                         0x00010000U
#define CMD_SECTOR_ERASE                    W25Q_CMD_BLOCK_ERASE_64KB
#endif

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   W25Q descriptor.
 */
flash_descriptor_t snor_descriptor = {
  .attributes       = FLASH_ATTR_ERASED_IS_ONE | FLASH_ATTR_REWRITABLE |
                      FLASH_ATTR_SUSPEND_ERASE_CAPABLE,
  .page_size        = 256U,
  .sectors_count    = 0U,           /* It is overwritten.*/
  .sectors          = NULL,
  .sectors_size     = SECTOR_SIZE,
  .address          = 0U,
  .size             = 0U            /* It is overwritten.*/
};

#if (SNOR_BUS_DRIVER == SNOR_BUS_DRIVER_WSPI) || defined(__DOXYGEN__)
#if (WSPI_SUPPORTS_MEMMAP == TRUE) || defined(__DOXYGEN__)
/* TODO: complete with wspi commands. */
#error "invalid SNOR_BUS_DRIVER mode"
/**
 * @brief   Fast read command for memory mapped mode.
 */
const wspi_command_t snor_memmap_read = {
  .addr             = 0U,
  .cmd              = 0U,
  .dummy            = 0U,
  .cfg              = 0U
};
#endif /* WSPI_SUPPORTS_MEMMAP == TRUE */
#endif /* SNOR_BUS_DRIVER == SNOR_BUS_DRIVER_WSPI */

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

#if SNOR_BUS_DRIVER == SNOR_BUS_DRIVER_WSPI
/* TODO: complete with wspi commands. */
#error "invalid SNOR_BUS_DRIVER mode"
/* Initial read ID command.*/
static const wspi_command_t w25q_cmd_read_id = {
  .cmd              = 0U,
  .cfg              = 0U,
  .dummy            = 0U,
  .addr             = 0U,
  .alt              = 0U
};
#endif /* SNOR_BUS_DRIVER == SNOR_BUS_DRIVER_WSPI */

static const uint8_t w25q_manufacturer_ids[] = W25Q_SUPPORTED_MANUFACTURE_IDS;
static const uint8_t w25q_memory_type_ids[] = W25Q_SUPPORTED_MEMORY_TYPE_IDS;
static const uint8_t w25q_memory_capacities[] = W25Q_SUPPORTED_MEMORY_CAPACITIES_IDS;

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

static bool w25q_find_id(const uint8_t *set, size_t size, uint8_t element) {
  size_t i;

  for (i = 0; i < size; i++) {
    if (set[i] == element) {
      return true;
    }
  }
  return false;
}

static flash_error_t w25q_poll_status(SNORDriver *devp) {
  uint8_t sts[2];

  do {
#if W25Q_NICE_WAITING == TRUE
    osalThreadSleepMilliseconds(1);
#endif
    /* Read status command.*/
#if W25Q_BUS_MODE == W25Q_BUS_MODE_SPI
    bus_cmd_receive(devp->config->busp, W25Q_CMD_READ_STATUS_REGISTER_1, 1U, sts);
#else
    /* TODO: complete with wspi commands. */
    #error "invalid SNOR_BUS_DRIVER mode"
#endif
  } while ((sts[0] & 1U) != 0U);

  return FLASH_NO_ERROR;
}

#if (SNOR_BUS_DRIVER == SNOR_BUS_DRIVER_WSPI) || defined(__DOXYGEN__)
/* TODO: complete with wspi commands. */
#error "invalid SNOR_BUS_DRIVER mode"
/**
 * @brief   Device software reset.
 * @note    It attempts to reset first in supposed final bus mode then tries
 *          in SPI mode.
 *
 * @param[in] devp      pointer to a @p SNORDriver instance
 */
static void w25q_reset(SNORDriver *devp) {
  /* TODO: complete with wspi commands. */
}

/**
 * @brief   Writes a Status register.
 *
 * @param[in] devp      pointer to a @p SNORDriver instance
 * @param[in] addr      address field
 * @param[in] value     value to be written
 */
static void w25q_write_sr(SNORDriver *devp, uint32_t addr, const uint8_t *value) {
  /* TODO: complete with wspi commands. */
}
#endif /* SNOR_BUS_DRIVER == SNOR_BUS_DRIVER_WSPI */

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Device initialization.
 *
 * @param[in] devp      pointer to a @p SNORDriver instance
 */
void snor_device_init(SNORDriver *devp) {

#if SNOR_BUS_DRIVER == SNOR_BUS_DRIVER_SPI
  /* Reading device ID.*/
  bus_cmd_receive(devp->config->busp, W25Q_CMD_READ_ID,
                  sizeof devp->device_id, devp->device_id);

#else /* SNOR_BUS_DRIVER == SNOR_BUS_DRIVER_WSPI */
  /* TODO: complete with wspi commands. */
#error "invalid SNOR_BUS_DRIVER mode"
#endif /* SNOR_BUS_DRIVER == SNOR_BUS_DRIVER_WSPI */

  /* Checking if the device is white listed.*/
  osalDbgAssert(w25q_find_id(w25q_manufacturer_ids,
                             sizeof w25q_manufacturer_ids,
                             devp->device_id[0]),
                             "invalid manufacturer id");
  osalDbgAssert(w25q_find_id(w25q_memory_type_ids,
                             sizeof w25q_memory_type_ids,
                             devp->device_id[1]),
                             "invalid memory type id");
  osalDbgAssert(w25q_find_id(w25q_memory_capacities,
                             sizeof w25q_memory_capacities,
                             devp->device_id[2]),
                             "invalid memory capacities");

#if (SNOR_BUS_DRIVER == SNOR_BUS_DRIVER_WSPI) && (W25Q_SWITCH_WIDTH == TRUE)
  /* TODO: complete with wspi commands. */
#error "invalid SNOR_BUS_DRIVER mode"
#endif

  /* Setting up the device size.*/
  switch (devp->device_id[2]) {
  case W25Q_SUPPORTED_MEMORY_8Mb:
    snor_descriptor.sectors_count = W25W_MEMORY_CAPACITY_8Mb_1MB / SECTOR_SIZE;
    break;
  case W25Q_SUPPORTED_MEMORY_16Mb:
    snor_descriptor.sectors_count = W25W_MEMORY_CAPACITY_16Mb_2MB / SECTOR_SIZE;
    break;
  case W25Q_SUPPORTED_MEMORY_32Mb:
    snor_descriptor.sectors_count = W25W_MEMORY_CAPACITY_32Mb_4MB / SECTOR_SIZE;
    break;
  case W25Q_SUPPORTED_MEMORY_128Mb:
    snor_descriptor.sectors_count = W25W_MEMORY_CAPACITY_128Mb_16M / SECTOR_SIZE;
    break;
  }

  snor_descriptor.size = (size_t)snor_descriptor.sectors_count * SECTOR_SIZE;
}

/**
 * @brief   Device read.
 *
 * @param[in] devp      pointer to a @p SNORDriver instance
 * @param[in] offset    flash offset
 * @param[in] n         number of bytes
 * @param[out] rp       pointer to the buffer
 */
flash_error_t snor_device_read(SNORDriver *devp, flash_offset_t offset,
                               size_t n, uint8_t *rp) {

#if SNOR_BUS_DRIVER == SNOR_BUS_DRIVER_WSPI
  /* TODO: complete with wspi commands. */
#error "invalid SNOR_BUS_DRIVER mode"
#else
  /* Read command in SPI mode.*/
  bus_cmd_addr_receive(devp->config->busp, W25Q_CMD_READ,
                       offset, n, rp);
#endif

  return FLASH_NO_ERROR;
}


/**
 * @brief   Device program.
 *
 * @param[in] devp      pointer to a @p SNORDriver instance
 * @param[in] offset    flash offset
 * @param[in] n         number of bytes
 * @param[in] pp        pointer to the buffer
 */
flash_error_t snor_device_program(SNORDriver *devp, flash_offset_t offset,
                                  size_t n, const uint8_t *pp) {

  /* Data is programmed page by page.*/
  while (n > 0U) {
    flash_error_t err;

    /* Data size that can be written in a single program page operation.*/
    size_t chunk = (size_t)(((offset | PAGE_MASK) + 1U) - offset);
    if (chunk > n) {
      chunk = n;
    }

#if W25Q_BUS_MODE == W25Q_BUS_MODE_SPI
    /* Enabling write operation.*/
    bus_cmd(devp->config->busp, W25Q_CMD_WRITE_ENABLE);

    /* Page program command.*/
    bus_cmd_addr_send(devp->config->busp, W25Q_CMD_PAGE_PROGRAM, offset,
                      chunk, pp);
#else
    /* TODO: complete with wspi commands. */
#error "invalid W25Q_BUS_MODE mode"
#endif

    /* Wait for status and check errors.*/
    err = w25q_poll_status(devp);
    if (err != FLASH_NO_ERROR) {

      return err;
    }

    /* Next page.*/
    offset += chunk;
    pp     += chunk;
    n      -= chunk;
  }

  return FLASH_NO_ERROR;
}

/**
 * @brief   Device global erase start.
 *
 * @param[in] devp      pointer to a @p SNORDriver instance
 */
flash_error_t snor_device_start_erase_all(SNORDriver *devp) {

#if W25Q_BUS_MODE == W25Q_BUS_MODE_SPI
  /* Enabling write operation.*/
  bus_cmd(devp->config->busp, W25Q_CMD_WRITE_ENABLE);

  /* Bulk erase command.*/
  bus_cmd(devp->config->busp, W25Q_CMD_CHIP_ERASE);
#else
  /* TODO: complete with wspi commands. */
#error "invalid W25Q_BUS_MODE mode"
#endif

  return FLASH_NO_ERROR;
}


/**
 * @brief   Device sector erase start.
 *
 * @param[in] devp      pointer to a @p SNORDriver instance
 * @param[in] sector    flash sector
 */
flash_error_t snor_device_start_erase_sector(SNORDriver *devp,
                                             flash_sector_t sector) {
  flash_offset_t offset = (flash_offset_t)(sector * SECTOR_SIZE);

#if W25Q_BUS_MODE == W25Q_BUS_MODE_SPI
  /* Enabling write operation.*/
  bus_cmd(devp->config->busp, W25Q_CMD_WRITE_ENABLE);

  /* Sector erase command.*/
  bus_cmd_addr(devp->config->busp, CMD_SECTOR_ERASE, offset);
#else
  /* TODO: complete with wspi commands. */
#error "invalid W25Q_BUS_MODE mode"
#endif

  return FLASH_NO_ERROR;
}

/**
 * @brief   Device erase verify.
 *
 * @param[in] devp      pointer to a @p SNORDriver instance
 * @param[in] sector    flash sector
 */
flash_error_t snor_device_verify_erase(SNORDriver *devp,
                                       flash_sector_t sector) {
  uint8_t cmpbuf[W25Q_COMPARE_BUFFER_SIZE];
  flash_offset_t offset;
  size_t n;

  /* Read command.*/
  offset = (flash_offset_t)(sector * SECTOR_SIZE);
  n = SECTOR_SIZE;
  while (n > 0U) {
    uint8_t *p;

#if SNOR_BUS_DRIVER == SNOR_BUS_DRIVER_WSPI
    /* TODO: complete with wspi commands. */
#error "invalid SNOR_BUS_DRIVER mode"
#else
   /* Normal read command in SPI mode.*/
   bus_cmd_addr_receive(devp->config->busp, W25Q_CMD_READ,
                        offset, sizeof cmpbuf, cmpbuf);
#endif

    /* Checking for erased state of current buffer.*/
    for (p = cmpbuf; p < &cmpbuf[W25Q_COMPARE_BUFFER_SIZE]; p++) {
      if (*p != 0xFFU) {
        /* Ready state again.*/
        devp->state = FLASH_READY;

        return FLASH_ERROR_VERIFY;
      }
    }

    offset += sizeof cmpbuf;
    n -= sizeof cmpbuf;
  }

  return FLASH_NO_ERROR;
}

/**
 * @brief   Queries if there is an erase in progress.
 *
 * @param[in] devp      pointer to a @p SNORDriver instance
 * @param[out] msec     suggested number of milliseconds before calling this
 *                      function again
 */
flash_error_t snor_device_query_erase(SNORDriver *devp, uint32_t *msec) {
  uint8_t sts[2];

  /* Read status register.*/
#if W25Q_BUS_MODE == W25Q_BUS_MODE_SPI
  bus_cmd_receive(devp->config->busp, W25Q_CMD_READ_STATUS_REGISTER_1, 1U, sts);
#else
  /* TODO: complete with wspi commands. */
#error "invalid W25Q_BUS_MODE mode"
#endif

  /* If the WIP bit is one (busy) or the flash in a suspended state then
     report that the operation is still in progress.*/
  if (((sts[0] & 1) != 0U)) {

    /* Recommended time before polling again, this is a simplified
       implementation.*/
    if (msec != NULL) {
      *msec = 1U;
    }

    return FLASH_BUSY_ERASING;
  }

  return FLASH_NO_ERROR;
}

flash_error_t snor_device_read_sfdp(SNORDriver *devp, flash_offset_t offset,
                                    size_t n, uint8_t *rp) {

  (void)devp;
  (void)rp;
  (void)offset;
  (void)n;

  return FLASH_NO_ERROR;
}

/** @} */
