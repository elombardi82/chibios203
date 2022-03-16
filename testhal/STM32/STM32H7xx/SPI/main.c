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

#define SPI_BUF_SIZE            16

static uint8_t spi_tx_buf[SPI_BUF_SIZE];

/*
 * Circular SPI configuration (25MHz, CPHA=0, CPOL=0, MSb first).
 */
const SPIConfig spicfg = {
  .circular         = false,
  .end_cb           = NULL,
  .ssport           = GPIOB,
  .sspad            = 4U,
  .cfg1             = SPI_CFG1_MBR_DIV8 | SPI_CFG1_DSIZE_VALUE(7),
  .cfg2             = 0U
};

/*
 * This is a periodic thread that does absolutely nothing except flashing
 * a LED.
 */
static THD_WORKING_AREA(waThread2, 512);
static THD_FUNCTION(Thread2, arg) {

  (void)arg;
  chRegSetThreadName("sender");
  while (true) {
    spiSelect(&SPID2);
    spiSend(&SPID2, SPI_BUF_SIZE, spi_tx_buf);
    spiUnselect(&SPID2);
    chThdSleepMilliseconds(500);
  }
}

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

  uint16_t i;

  /* Fill buffer */
  for (i = 0; i < SPI_BUF_SIZE; i++) {
    spi_tx_buf[i] = i;
  }

  /*
   * Starting and configuring the I2S driver 2.
   */
  spiStart(&SPID2, &spicfg);

  /* SPI CK */
  palSetPadMode(GPIOD, 3, PAL_MODE_ALTERNATE(5));

  /* SPI MOSI */
  palSetPadMode(GPIOB, 15, PAL_MODE_ALTERNATE(5));

  /* SPI MISO */
  palSetPadMode(GPIOI, 2, PAL_MODE_ALTERNATE(5));

  /* SPI CS */
  palSetPadMode(GPIOB, 4, PAL_MODE_ALTERNATE(7));

  /*
   * Creates the example thread.
   */
  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);
  chThdCreateStatic(waThread2, sizeof(waThread2), NORMALPRIO+1, Thread2, NULL);

  /*
   * Normal main() thread activity, in this demo it does nothing except
   * sleeping in a loop and check the button state.
   */
  while (TRUE) {
    if (palReadLine(LINE_BUTTON)) {
      spiStop(&SPID2);
    }
    chThdSleepMilliseconds(500);
  }
}
