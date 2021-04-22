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

#include "ch.h"
#include "hal.h"
#include <string.h>

#define MT25Q_CMD_RESET_ENABLE                     0x66
#define MT25Q_CMD_RESET_MEMORY                     0x99
#define MT25Q_CMD_READ_ID                          0x9F
#define MT25Q_CMD_MULTIPLE_IO_READ_ID              0xAF
#define MT25Q_CMD_READ_DISCOVERY_PARAMETER         0x5A
#define MT25Q_CMD_READ                             0x03
#define MT25Q_CMD_FAST_READ                        0x0B
#define MT25Q_CMD_WRITE_ENABLE                     0x06
#define MT25Q_CMD_WRITE_DISABLE                    0x04
#define MT25Q_CMD_READ_STATUS_REGISTER             0x05
#define MT25Q_CMD_WRITE_STATUS_REGISTER            0x01
#define MT25Q_CMD_READ_LOCK_REGISTER               0xE8
#define MT25Q_CMD_WRITE_LOCK_REGISTER              0xE5
#define MT25Q_CMD_READ_FLAG_STATUS_REGISTER        0x70
#define MT25Q_CMD_CLEAR_FLAG_STATUS_REGISTER       0x50
#define MT25Q_CMD_READ_NV_CONFIGURATION_REGISTER   0xB5
#define MT25Q_CMD_WRITE_NV_CONFIGURATION_REGISTER  0xB1
#define MT25Q_CMD_READ_V_CONF_REGISTER             0x85
#define MT25Q_CMD_WRITE_V_CONF_REGISTER            0x81
#define MT25Q_CMD_READ_ENHANCED_V_CONF_REGISTER    0x65
#define MT25Q_CMD_WRITE_ENHANCED_V_CONF_REGISTER   0x61
#define MT25Q_CMD_PAGE_PROGRAM                     0x02
#define MT25Q_CMD_SUBSECTOR_ERASE                  0x20
#define MT25Q_CMD_SECTOR_ERASE                     0xD8
#define MT25Q_CMD_BULK_ERASE                       0xC7
#define MT25Q_CMD_PROGRAM_ERASE_RESUME             0x7A
#define MT25Q_CMD_PROGRAM_ERASE_SUSPEND            0x75
#define MT25Q_CMD_READ_OTP_ARRAY                   0x4B
#define MT25Q_CMD_PROGRAM_OTP_ARRAY                0x42

static uint8_t receive[128] = {0};
static uint8_t pattern[128] = {0};
static uint8_t status = 0;

static const uint8_t mt25q_evconf_value[1] = {0x6F};

const WSPIConfig WSPIcfg1 = {
  .end_cb           = NULL,
  .error_cb         = NULL,
  .dcr              = STM32_DCR_FSIZE(25U) |        /* 64MB device.         */
                      STM32_DCR_CSHT(1U)            /* NCS 2 cycles delay.  */
};

static const wspi_command_t cmd_reset_enable_1 = {
  .cmd              = MT25Q_CMD_RESET_ENABLE,
  .cfg              = WSPI_CFG_CMD_MODE_ONE_LINE,
  .addr             = 0,
  .alt              = 0,
  .dummy            = 0
};

/* 1x MT25Q_CMD_RESET_MEMORY command.*/
static const wspi_command_t cmd_reset_memory_1 = {
  .cmd              = MT25Q_CMD_RESET_MEMORY,
  .cfg              = WSPI_CFG_CMD_MODE_ONE_LINE,
  .addr             = 0,
  .alt              = 0,
  .dummy            = 0
};

static const wspi_command_t mt25q_cmd_read_id = {
  .cmd              = MT25Q_CMD_READ_ID,
  .cfg              = 0U |
                      WSPI_CFG_CMD_MODE_ONE_LINE |
                      WSPI_CFG_DATA_MODE_ONE_LINE,
  .addr             = 0,
  .alt              = 0,
  .dummy            = 0
};

static const wspi_command_t mt25q_cmd_mul_read_id = {
  .cmd              = MT25Q_CMD_MULTIPLE_IO_READ_ID,
  .cfg              = 0U |
                      WSPI_CFG_CMD_MODE_FOUR_LINES |
                      WSPI_CFG_DATA_MODE_FOUR_LINES,
  .addr             = 0,
  .alt              = 0,
  .dummy            = 0
};

/* Initial MT25Q_CMD_WRITE_ENABLE command.*/
static wspi_command_t mt25q_cmd_write_enable = {
  .cmd              = MT25Q_CMD_WRITE_ENABLE,
  .cfg              = 0U |
                      WSPI_CFG_CMD_MODE_ONE_LINE,
  .addr             = 0,
  .alt              = 0,
  .dummy            = 0
};

/* Initial MT25Q_CMD_WRITE_ENHANCED_V_CONF_REGISTER command.*/
static const wspi_command_t mt25q_cmd_write_evconf = {
  .cmd              = MT25Q_CMD_WRITE_ENHANCED_V_CONF_REGISTER,
  .cfg              = 0U |
                      WSPI_CFG_CMD_MODE_ONE_LINE |
                      WSPI_CFG_DATA_MODE_ONE_LINE,
  .addr             = 0,
  .alt              = 0,
  .dummy            = 0
};

static const wspi_command_t mt25q_cmd_chip_erase = {
  .cmd              = MT25Q_CMD_BULK_ERASE,
  .cfg              = 0U |
                      WSPI_CFG_CMD_MODE_FOUR_LINES,
  .addr             = 0,
  .alt              = 0,
  .dummy            = 0
};

static const wspi_command_t mt25q_cmd_read_status = {
  .cmd              = MT25Q_CMD_READ_STATUS_REGISTER,
  .cfg              = 0U |
                      WSPI_CFG_CMD_MODE_FOUR_LINES     |
                      WSPI_CFG_ADDR_MODE_NONE          |
                      WSPI_CFG_ALT_MODE_NONE           |
                      WSPI_CFG_DATA_MODE_FOUR_LINES    |
                      WSPI_CFG_CMD_SIZE_8              |
                      WSPI_CFG_ADDR_SIZE_24,
 .addr             = 0,
 .alt              = 0,
 .dummy            = 0
};

static const wspi_command_t mt25q_cmd_page_program = {
  .cmd              = MT25Q_CMD_PAGE_PROGRAM,
  .cfg              = 0U |
                      WSPI_CFG_CMD_MODE_FOUR_LINES |
                      WSPI_CFG_ADDR_MODE_FOUR_LINES    |
                      WSPI_CFG_ALT_MODE_NONE           |
                      WSPI_CFG_DATA_MODE_FOUR_LINES    |
                      WSPI_CFG_ADDR_SIZE_24,
  .addr             = 0,
  .alt              = 0,
  .dummy            = 0
};

static const wspi_command_t mt25q_cmd_fast_read = {
  .cmd              = MT25Q_CMD_FAST_READ,
  .cfg              = 0U |
                      WSPI_CFG_CMD_MODE_FOUR_LINES     |
                      WSPI_CFG_ADDR_MODE_FOUR_LINES    |
                      WSPI_CFG_ALT_MODE_NONE           |
                      WSPI_CFG_DATA_MODE_FOUR_LINES    |
                      WSPI_CFG_CMD_SIZE_8              |
                      WSPI_CFG_ADDR_SIZE_24,
  .addr             = 0,
  .alt              = 0,
  .dummy            = 10
};

/*
 * This is a periodic thread that does absolutely nothing except flashing
 * a LED.
 */
static THD_WORKING_AREA(waThread1, 128);
static THD_FUNCTION(Thread1, arg) {

  (void)arg;
  chRegSetThreadName("blinker");
  while (true) {
    palSetLine(LINE_LED_GREEN);
    chThdSleepMilliseconds(50);
    palSetLine(LINE_LED_RED);
    chThdSleepMilliseconds(200);
    palClearLine(LINE_LED_GREEN);
    chThdSleepMilliseconds(50);
    palClearLine(LINE_LED_RED);
    chThdSleepMilliseconds(200);
  }
}

/*
 * Application entry point.
 */
int main(void) {

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();

  SCB_DisableDCache();
  SCB_DisableICache();

  uint8_t i;

  /*
   * Init pattern
   */
  for (i = 0; i < sizeof pattern; i++) {
    pattern[i] = i;
  }

  /*
   * Activates the serial driver 1 using the driver default configuration.
   */
  sdStart(&SD3, NULL);
  wspiStart(&WSPID1, &WSPIcfg1);

  wspiCommand(&WSPID1, &cmd_reset_enable_1);
  wspiCommand(&WSPID1, &cmd_reset_memory_1);

  /* Reading device ID and unique ID.*/
  wspiReceive(&WSPID1, &mt25q_cmd_read_id, 20, receive);

  /* Resetting receive */
  memset(receive, 0, sizeof receive);

  /* Write enable */
  wspiCommand(&WSPID1, &mt25q_cmd_write_enable);

  /* Enable QUAD SPI mode */
  wspiSend(&WSPID1, &mt25q_cmd_write_evconf, 1, mt25q_evconf_value);

  /* Reading devide ID in QUAD mode */
  wspiReceive(&WSPID1, &mt25q_cmd_mul_read_id, 20, receive);

  /* Resetting receive */
  memset(receive, 0, sizeof receive);

  /* Now work in QUADS SPI mode */
  mt25q_cmd_write_enable.cfg = (0 | WSPI_CFG_CMD_MODE_FOUR_LINES);

  /* Write enable */
  wspiCommand(&WSPID1, &mt25q_cmd_write_enable);

  /* Chip erase. */
  wspiCommand(&WSPID1, &mt25q_cmd_chip_erase);

  /*
   * Waiting for erasing complete
   * First bit of status register is inverted seven bit of flag status register
   */
  do {
    wspiReceive(&WSPID1, &mt25q_cmd_read_status, 1, &status);
  } while (status & 0x01);

  /* Write enable. */
  wspiCommand(&WSPID1, &mt25q_cmd_write_enable);

  /* Write on memory. */
  wspiSend(&WSPID1, &mt25q_cmd_page_program, sizeof pattern, pattern);

  /* Waiting for writing complete */
  do {
    wspiReceive(&WSPID1, &mt25q_cmd_read_status, 1, &status);
  } while (status & 0x01);

  /* Read on memory. */
  wspiReceive(&WSPID1, &mt25q_cmd_fast_read, 128, receive);

  /*
   * Creates the example thread.
   */
  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO+1, Thread1, NULL);

  /*
   * Normal main() thread activity, in this demo it does nothing except
   * sleeping in a loop and check the button state.
   */
  while (true) {
    chThdSleepMilliseconds(500);
  }
}
