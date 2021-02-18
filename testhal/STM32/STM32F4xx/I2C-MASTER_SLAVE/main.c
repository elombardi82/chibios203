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

/*
 * Master operations
 */
#define MASTER_TRAS_RECV      0x00
#define MASTER_TRAS           0x01
#define MASTER_RECV           0x02

uint8_t master_recv[2] = {0};
uint8_t master_tras[] = "ChibiOS I2C Slave Master Demo";

uint8_t slave_recv[50] = {0};
uint8_t slave_tras[2] = {'O','K'};

/* I2C config */
static const I2CConfig cfg = {
  OPMODE_I2C,
  100000,
  STD_DUTY_CYCLE
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
    palSetPad(GPIOD, GPIOD_LED3);       /* Orange.  */
    chThdSleepMilliseconds(500);
    palClearPad(GPIOD, GPIOD_LED3);     /* Orange.  */
    chThdSleepMilliseconds(500);
  }
}

/*
 * This is a periodic thread waiting for master transmission.
 */
static THD_WORKING_AREA(waThread2, 256);
static THD_FUNCTION(Thread2, arg) {

  (void)arg;
  chRegSetThreadName("i2cslave");

  while (true) {
	/* Wait for a master transmission */
    i2cSlaveReceiveTimeout(&I2CD2, slave_recv, sizeof slave_recv, TIME_INFINITE);

    /* If master need reply */
    if (I2CD2.reply_required) {
      i2cSlaveTransmitTimeout(&I2CD2, slave_tras, sizeof slave_tras, TIME_INFINITE);
    }
    memset(slave_recv, 0, sizeof slave_recv);
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

  uint8_t op = MASTER_TRAS_RECV;

  /* Master I2C1 SCL on PB6, SDA on PB9 */
  palSetPadMode(GPIOB, 6, PAL_MODE_ALTERNATE(4) | PAL_STM32_OTYPE_OPENDRAIN);
  palSetPadMode(GPIOB, 9, PAL_MODE_ALTERNATE(4) | PAL_STM32_OTYPE_OPENDRAIN);

  /* Slave I2C2 SCL on PB10, SDA on PB11 */
  palSetPadMode(GPIOB, 10, PAL_MODE_ALTERNATE(4) | PAL_STM32_OTYPE_OPENDRAIN);
  palSetPadMode(GPIOB, 11, PAL_MODE_ALTERNATE(4) | PAL_STM32_OTYPE_OPENDRAIN);

  i2cStart(&I2CD1, &cfg);
  i2cStart(&I2CD2, &cfg);
  i2cSlaveMatchAddress(&I2CD2, I2C_ADDR);

  /*
   * Creates the example thread.
   */
  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);
  chThdCreateStatic(waThread2, sizeof(waThread2), NORMALPRIO, Thread2, NULL);

  /*
   * Normal main() thread activity, in this demo it does nothing except
   * sleeping in a loop and check the button state to test I2C Master.
   */
  while (true) {
    if (palReadPad(GPIOA, GPIOA_BUTTON)) {
      if (op == MASTER_TRAS_RECV) {
        i2cMasterTransmitTimeout(&I2CD1, I2C_ADDR, master_tras, sizeof master_tras - 1,
        		                          master_recv, 2, TIME_INFINITE);
      }
      else if (op == MASTER_TRAS) {
        i2cMasterTransmitTimeout(&I2CD1, I2C_ADDR, master_tras, sizeof master_tras - 1,
          		                          NULL, 0, TIME_INFINITE);
      }
      else {
        i2cMasterReceiveTimeout(&I2CD1, I2C_ADDR, master_recv, 2, TIME_INFINITE);
      }

      if (op == MASTER_RECV) {
        op = MASTER_TRAS_RECV;
      }
      else {
        op++;
      }

      /* Reset buffer */
      memset(master_recv, 0, sizeof master_recv);
    }
    chThdSleepMilliseconds(500);
  }
}
