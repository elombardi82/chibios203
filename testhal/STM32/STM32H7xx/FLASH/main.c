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

#include "hal_serial_nor.h"
#include "hal_mfs.h"

#include "mfs_test_root.h"

const WSPIConfig WSPIcfg1 = {
  .end_cb           = NULL,
  .error_cb         = NULL,
  .dcr              = STM32_DCR_FSIZE(25U) |        /* 64MB device.         */
                      STM32_DCR_CSHT(1U)            /* NCS 2 cycles delay.  */
};

const SNORConfig snorcfg1 = {
  .busp             = &WSPID1,
  .buscfg           = &WSPIcfg1
};

SNORDriver snor1;

const MFSConfig mfscfg1 = {
  .flashp           = (BaseFlash *)&snor1,
  .erased           = 0xFFFFFFFFU,
  .bank_size        = 4096U,
  .bank0_start      = 0U,
  .bank0_sectors    = 1U,
  .bank1_start      = 1U,
  .bank1_sectors    = 1U
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

  /*
   * Activates the serial driver 3 using the driver default configuration.
   */
  sdStart(&SD3, NULL);

  /* Initializing and starting snor1 driver.*/
  snorObjectInit(&snor1);
  snorStart(&snor1, &snorcfg1);
#if 1
  /* Testing memory mapped mode.*/
  {
    uint8_t *addr;

    snorMemoryMap(&snor1, &addr);
    chThdSleepMilliseconds(50);
    snorMemoryUnmap(&snor1);
  }
#endif

  /*
   * Creates the example thread.
   */
  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO+1, Thread1, NULL);

  /*
   * Normal main() thread activity, in this demo it does nothing except
   * sleeping in a loop and check the button state.
   */
  while (1) {
    if (palReadLine(LINE_BUTTON)) {
      test_execute((BaseSequentialStream *)&SD3, &mfs_test_suite);
    }
    chThdSleepMilliseconds(500);
  }
}
