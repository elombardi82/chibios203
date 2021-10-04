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

/*
 * I2C address
 */
#define I2C_ADDR              0x18
#define I2C_BUF_SIZE            32

static uint8_t i2c_tx_buf[I2C_BUF_SIZE];
static uint8_t i2c_rx_buf[I2C_BUF_SIZE];

/* I2C config */
static const I2CConfig i2c_cfg = {
  //STM32_TIMINGR_PRESC(15U) |
  STM32_TIMINGR_SCLDEL(7U) | //STM32_TIMINGR_SDADEL(0U) |
  STM32_TIMINGR_SCLH(124U)  | STM32_TIMINGR_SCLL(187U),
  0,
  0
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
    chThdSleepMilliseconds(500);
    palClearLine(LINE_LED_GREEN);
    chThdSleepMilliseconds(500);
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

  /*
   * TODO: Handling cache
   */
  SCB_DisableDCache();
  SCB_DisableICache();

  uint32_t i;

  /* Fill buffer */
  for (i = 0; i < I2C_BUF_SIZE; i++) {
    i2c_tx_buf[i] = i;
  }

  /*
   * Starting and configuring the I2S driver 2.
   */
  i2cStart(&I2CD1, &i2c_cfg);

  /* I2C1 SCL - PB6 */
  palSetPadMode(GPIOB, 6, PAL_MODE_ALTERNATE(4) | PAL_STM32_OTYPE_OPENDRAIN | PAL_STM32_PUPDR_PULLUP);

  /* I2C1 SDA - PB7 */
  palSetPadMode(GPIOB, 7, PAL_MODE_ALTERNATE(4) | PAL_STM32_OTYPE_OPENDRAIN | PAL_STM32_PUPDR_PULLUP);

  /*
   * Creates the example thread.
   */
  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);

  /*
   * Normal main() thread activity, in this demo it does nothing except
   * sleeping in a loop and check the button state.
   */
  while (TRUE) {
    /*
     * Only transmission
     */
    i2cMasterTransmitTimeout(&I2CD1, I2C_ADDR, i2c_tx_buf, I2C_BUF_SIZE,
                             NULL, 0, TIME_INFINITE);
    chThdSleepMilliseconds(1000);

    /*
     * Transmit and receive
     */
    i2cMasterTransmitTimeout(&I2CD1, I2C_ADDR, i2c_tx_buf, I2C_BUF_SIZE,
                             i2c_rx_buf, 2, TIME_INFINITE);
    /* Reset buffer */
    memset(i2c_rx_buf, 0, I2C_BUF_SIZE);
    chThdSleepMilliseconds(1000);

    /*
     * Only receive
     */
    i2cMasterReceiveTimeout(&I2CD1, I2C_ADDR, i2c_rx_buf, 2, TIME_INFINITE);
    /* Reset buffer */
    memset(i2c_rx_buf, 0, I2C_BUF_SIZE);
    chThdSleepMilliseconds(1000);
  }
}
