/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio

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
#include "halconf_community.h"
#include "ff.h"
#include "wav/wav.h"

#define TX_SIZE 2048

/*
 * Synchronization semaphore.
 */
binary_semaphore_t full_bsem, half_bsem;

static void i2scallback(I2SDriver *i2sp);
static uint8_t i2s_tx[TX_SIZE];

/*
 * I2S configuration.
 * i2s_clock is 75MHz (see mcuconf.h)
 * 16 bit I2S philips standard, falling edge at 48KHz.
 * ODD = 1, I2SDIV = 24, CKPOL = 0, CHLEN = 0, DATALEN = 0, I2SSTD = 0.
 */
static const I2SConfig i2scfg = {
  (uint16_t *) i2s_tx,
  NULL,
  TX_SIZE >> 1,
  i2scallback,
  SPI_I2SCFGR_ODD |
  (24 << SPI_I2SCFGR_I2SDIV_Pos)   /* CFGR register */
};

static void i2scallback(I2SDriver *i2sp) {

  chSysLockFromISR();
  if (i2sIsBufferComplete(i2sp)) {
    chBSemSignalI(&full_bsem);
  }
  else {
    /* 1st buffer half processing.*/
    chBSemSignalI(&half_bsem);
  }
  chSysUnlockFromISR();
}

static msg_t play_wav(const char *file_name, wav_header_t *headerp) {
  FIL file;
  msg_t res = FR_OK;
  UINT bred;
  uint8_t full = 0;
  uint32_t size = headerp->data_size;

  if ((res = f_open(&file, file_name, FA_READ))) {
    return res;
  }

  /* Jump to data */
  f_lseek(&file, (headerp->overall_size - headerp->data_size + 8));

  while (size) {
    if (!full) {
      chBSemWait (&half_bsem);
      f_read(&file, &i2s_tx[0], ((sizeof i2s_tx) >> 1), &bred);
      full = 1;
    }
    else {
      chBSemWait (&full_bsem);
      f_read(&file, &i2s_tx[1024], ((sizeof i2s_tx) >> 1), &bred);
      full = 0;
    }
    size = size - bred;
  }
  return f_close(&file);
}

#if HAL_USBH_USE_MSD
#include "usbh/dev/msd.h"
#include "ff.h"
#include "fatfs_devices.h"

typedef enum {
  APP_STATE_CONNECTED = 0,
  APP_STATE_MOUNTED,
  APP_STATE_PLAY,
  APP_STATE_IDLE,
  APP_STATE_DISCONNECTED,
} app_state_t;

static FATFS MSDLUN0FS;
static app_state_t state = APP_STATE_DISCONNECTED;

#if 0
static void scan_files(USBHDriver *host, BaseSequentialStream *chp, char *path) {

  (void) host;

  DIR dir;
  static FILINFO fno;

  if (!f_opendir(&dir, path)) {
    while (!f_readdir(&dir, &fno)) {
      if ((fno.fname[0] == '\0')) {
        break;
      }

      if ((fno.fname[0] == '.')) {
        continue;
      }

      if (fno.fattrib != AM_DIR) {
        chprintf(chp, "Found %s", &fno.fname[0]);
      }
    }
  }
}
#endif

static THD_WORKING_AREA(waThread2, 2048);
static THD_FUNCTION(Thread2, arg) {
  (void)arg;
  chRegSetThreadName("msd_usbh");

  USBHDriver *host = NULL;
  wav_header_t head;

  while(true) {

    switch (state) {
      case APP_STATE_DISCONNECTED:
        if (blkGetDriverState(&MSBLKD[0]) == BLK_ACTIVE) {
          host = usbhmsdLUNGetHost(&MSBLKD[0]);
          usbhmsdLUNConnect(&MSBLKD[0]);
          state = APP_STATE_CONNECTED;
        }
        break;

      case APP_STATE_CONNECTED:
        if (!f_mount(&MSDLUN0FS, FATFSDEV_MSD_DRIVE, 1)) {
          state = APP_STATE_MOUNTED;
        }
        break;

      case APP_STATE_MOUNTED:
        getWavInformation("0:/Trinity.wav", &head);
        state = APP_STATE_PLAY;
        break;

      case APP_STATE_PLAY:
        /*
         * Starting continuous I2S transfer.
         */
        i2sStartExchange(&I2SD2);
        play_wav("0:/Trinity.wav", &head);
        i2sStopExchange(&I2SD2);
        state = APP_STATE_IDLE;
        break;

      case APP_STATE_IDLE:
        break;

      default:
        break;
    }

    /* Check usb */
    if (blkGetDriverState(&MSBLKD[0]) == BLK_STOP) {
      state = APP_STATE_DISCONNECTED;
    }
    chThdSleepMilliseconds(100);
  }
}
#endif

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
    chThdSleepMilliseconds(500);
    palClearLine(LINE_LED_GREEN);
    chThdSleepMilliseconds(500);
  }
}

int main(void) {

  halInit();
  chSysInit();

  chBSemObjectInit(&full_bsem, true);
  chBSemObjectInit(&half_bsem, true);

  /*
   * Starting and configuring the I2S driver 2.
   */
  i2sStart(&I2SD2, &i2scfg);

  /* I2S PLL MCO1 */
  palSetPadMode(GPIOA, 8, PAL_MODE_ALTERNATE(0));

  /* I2S CK */
  palSetPadMode(GPIOD, 3, PAL_MODE_ALTERNATE(5));

  /* I2S SDO/MOSI */
  palSetPadMode(GPIOB, 15, PAL_MODE_ALTERNATE(5));

  /* I2S SDI/MISO */
  palSetPadMode(GPIOI, 12, PAL_MODE_ALTERNATE(5));

  /* I2S WS */
  palSetPadMode(GPIOB, 4, PAL_MODE_ALTERNATE(7));

  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);
#if HAL_USBH_USE_MSD
  chThdCreateStatic(waThread2, sizeof(waThread2), NORMALPRIO, Thread2, NULL);
#endif

  /* Turn on USB power */
  palSetPad(GPIOA, GPIOA_PIN5);
  chThdSleepMilliseconds(100);

  /* Start USBH */
#if STM32_USBH_USE_OTG1
  usbhStart(&USBHD1);
#endif
#if STM32_USBH_USE_OTG2
  usbhStart(&USBHD2);
#endif

  while(true) {
#if STM32_USBH_USE_OTG1
    usbhMainLoop(&USBHD1);
#endif
#if STM32_USBH_USE_OTG2
    usbhMainLoop(&USBHD2);
#endif
    chThdSleepMilliseconds(100);
  }
}
