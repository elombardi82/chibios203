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
#include "CS43L22/cs43l22.h"

#if 0
#define ADC_GRP1_NUM_CHANNELS   1
#define ADC_GRP1_BUF_DEPTH      8

static uint32_t volume = 0;
static adcsample_t samples[ADC_GRP1_NUM_CHANNELS * ADC_GRP1_BUF_DEPTH];

static void adccallback(ADCDriver *adcp) {

  if (adcIsBufferComplete(adcp)) {
    volume = (samples[0] + samples[1] + samples[2] + samples[3] +
             samples[4] + samples[5] + samples[6] + samples[7]) >> 3;
  }
}

/*
 * ADC conversion group.
 * Mode:        Circular buffer, 8 samples of 1 channel, SW triggered,  8 bit.
 * Channels:    IN11.
 */
static const ADCConversionGroup adcgrpcfg1 = {
  TRUE,
  ADC_GRP1_NUM_CHANNELS,
  adccallback,
  NULL,
  ADC_CR1_RES_1,            /* CR1 */
  ADC_CR2_SWSTART,          /* CR2 */
  ADC_SMPR1_SMP_AN11(ADC_SAMPLE_3),
  0,                        /* SMPR2 */
  0,                        /* HTR */
  0,                        /* LTR */
  0,                        /* SQR1 */
  0,                        /* SQR2 */
  ADC_SQR3_SQ1_N(ADC_CHANNEL_IN11)
};
#endif
/* Player Driver */
PLAYERDriver PLAYD0;
/* Codec Driver */
CS43L22Driver CS4D0;

/* I2C config for CS43L22 */
static const I2CConfig i2c_cfg = {
  OPMODE_I2C,
  400000,
  FAST_DUTY_CYCLE_2
};

static const CS43L22Config cs4_cfg = {
  &I2CD1,
  &i2c_cfg,
  CS43L22_SAD_DEFAULT
};

/* PLAYER cfg */
static const PLAYERConfig play_cfg = {
  &I2SD3
};

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
        //chprintf(chp, "Found %s", &fno.fname[0]);
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
        state = APP_STATE_PLAY;
        break;

      case APP_STATE_PLAY:
        playerRun(&PLAYD0, "0:/Trinity.wav");
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
    palSetPad(GPIOD, GPIOD_LED3);       /* Orange.  */
    chThdSleepMilliseconds(500);
    palClearPad(GPIOD, GPIOD_LED3);     /* Orange.  */
    chThdSleepMilliseconds(500);
  }
}

/*
 * This is a periodic thread that does absolutely nothing except flashing
 * a LED.
 */
static THD_WORKING_AREA(waThread3, 256);
static THD_FUNCTION(Thread3, arg) {

  (void)arg;
  chRegSetThreadName("volume");
  while (true) {
    cs43l22SetVolume(&CS4D0, 0);
    chThdSleepMilliseconds(2000);
    cs43l22SetVolume(&CS4D0, 25);
    chThdSleepMilliseconds(1000);
    cs43l22SetVolume(&CS4D0, 4);
    chThdSleepMilliseconds(1000);
  }
}

int main(void) {

  halInit();
  chSysInit();
  cs43l22Init(&CS4D0);
  playerInit(&PLAYD0);

  /* PA2(TX) and PA3(RX) are routed to USART2 */
  sdStart(&SD2, NULL);
  palSetPadMode(GPIOA, 2, PAL_MODE_ALTERNATE(7));
  palSetPadMode(GPIOA, 3, PAL_MODE_ALTERNATE(7));

#if 0
  /* ADC PC1 Vol PC2 */
  palSetGroupMode(GPIOC, PAL_PORT_BIT(1) | PAL_PORT_BIT(2),
                  0, PAL_MODE_INPUT_ANALOG);
  /*
   * Activates the ADC1 driver.
   */
  adcStart(&ADCD1, NULL);

  /*
   * Starts an ADC continuous conversion.
   */
  adcStartConversion(&ADCD1, &adcgrpcfg1, samples1, ADC_GRP1_BUF_DEPTH);
#endif
  /*******************************
   * Power Up CS43L22
   *******************************/
  /* Activates CS43L22 audio codec */
  /* Turn on Codec */
  palSetPad(GPIOD, GPIOD_RESET);

  cs43l22Start(&CS4D0, &cs4_cfg);

  playerStart(&PLAYD0, &play_cfg);

#if STM32_USBH_USE_OTG1
  //VBUS - configured in board.h
  //USB_FS - configured in board.h
#endif

#if STM32_USBH_USE_OTG2
#error "TODO: Initialize USB_HS pads"
#endif

  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);
#if HAL_USBH_USE_MSD
  chThdCreateStatic(waThread2, sizeof(waThread2), NORMALPRIO, Thread2, NULL);
#endif
//  chThdCreateStatic(waThread3, sizeof(waThread3), NORMALPRIO, Thread3, NULL);

  /* Turn on USB power */
  palClearPad(GPIOC, GPIOC_OTG_FS_POWER_ON);
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
