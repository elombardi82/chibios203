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
 * @file    wav.c
 * @brief   WAV module code.
 *
 * @author  Edoardo Lombardi
 *
 * @{
 */
#include "hal.h"
#include "wav.h"
#include "ff.h"
/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/
#define TX_SIZE 2048

/*===========================================================================*/
/* Driver local macros.                                                      */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/
/**
 * @brief   Driver state machine possible states.
 */
typedef struct {
  uint32_t fs_8k;
  uint32_t fs_16k;
  uint32_t fs_32k;
  uint32_t fs_44_1k;
  uint32_t fs_48k ;
} wav_freqs_t;

static wav_freqs_t freqs = {
  0x21,                         /**<   8  KHz                   */
  SPI_I2SPR_ODD  | 0x10,        /**<   16 KHz                   */
  SPI_I2SPR_ODD  | 0x8,         /**<   32 KHz                   */
  0x6,                          /**<   44 100 KHz               */
  SPI_I2SPR_ODD  | 0x5,         /**<   48 KHz                   */
};

/*
 * Synchronization semaphore.
 */
binary_semaphore_t full_bsem, half_bsem;
static uint8_t i2s_tx[TX_SIZE];
static void i2scallback(I2SDriver *i2sp);

/*
 * @brief I2SConfig
 *
 * I2SxCLK = 135.5 MHz
 * default 48 KHz
 *
 */
static const I2SConfig i2scfg = {
  (uint16_t *) i2s_tx,
  NULL,
  TX_SIZE >> 1,
  i2scallback,
  0,
  SPI_I2SPR_MCKOE | SPI_I2SPR_ODD  | 0x5
};
/*===========================================================================*/
/* Driver functions.                                                         */
/*===========================================================================*/
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

/**
 * @brief   Extract information from wav file.
 *
 * @param[in]  file_name file name
 * @param[out] song   pointer to header struct
 *
 * @return               The operation status.
 *
 * @api
 */
static msg_t get_wav_information (const char *file_name, wav_song_t *song) {

  osalDbgCheck((file_name != NULL) && (song != NULL));

  FIL file;
  msg_t res = FR_OK;
  UINT bred;

  if ((res = f_open(&file, file_name, FA_READ))) {
    return res;
  }

  if ((res = f_read(&file, song->information.riff, sizeof song->information.riff, &bred))) {
    goto exit;
  }

  if ((res = f_read(&file, &song->information.overall_size, sizeof song->information.overall_size, &bred))) {
    goto exit;
  }

  if ((res = f_read(&file, song->information.wave, sizeof song->information.wave, &bred))) {
    goto exit;
  }

  if ((res = f_read(&file, song->information.fmt_chunk_marker, sizeof song->information.fmt_chunk_marker, &bred))) {
    goto exit;
  }

  if ((res = f_read(&file, &song->information.length_of_fmt, sizeof song->information.length_of_fmt, &bred))) {
    goto exit;
  }

  if ((res = f_read(&file, &song->information.format_type, sizeof song->information.format_type, &bred))) {
    goto exit;
  }

  if ((res = f_read(&file, &song->information.channels, sizeof song->information.channels, &bred))) {
    goto exit;
  }

  if ((res = f_read(&file, &song->information.sample_rate, sizeof song->information.sample_rate, &bred))) {
    goto exit;
  }

  if ((res = f_read(&file, &song->information.byterate, sizeof song->information.byterate, &bred))) {
    goto exit;
  }

  if ((res = f_read(&file, &song->information.block_align, sizeof song->information.block_align, &bred))) {
    goto exit;
  }

  if ((res = f_read(&file, &song->information.bits_per_sample, sizeof song->information.bits_per_sample, &bred))) {
    goto exit;
  }

  if ((res = f_read(&file, song->information.data_chunk_header, sizeof song->information.data_chunk_header, &bred))) {
    goto exit;
  }

  if ((res = f_read(&file, &song->information.data_size, sizeof song->information.data_size, &bred))) {
    goto exit;
  }

  /* if LIST field is present */
  if (song->information.data_chunk_header[0] == 'L') {
    char *tmp;
    tmp = chHeapAlloc(NULL, song->information.data_size);
    if (tmp == NULL) {
      res = -1;
      goto exit;
    }
    f_read(&file, tmp, song->information.data_size, &bred);
    chHeapFree(tmp);

    if ((res = f_read(&file, song->information.data_chunk_header, sizeof song->information.data_chunk_header, &bred))) {
      goto exit;
    }

    if ((res = f_read(&file, &song->information.data_size, sizeof song->information.data_size, &bred))) {
      goto exit;
    }
  }

  song->name = file_name;
exit:
  f_close(&file);
  return res;
}

/**
 * @brief   Play song.
 *
 * @param[in] song   pointer to header struct
 *
 * @return               The operation status.
 *
 * @api
 */
msg_t playerRun(PLAYERDriver *devp, const char *file_name) {
  osalDbgCheck((devp != NULL) && (file_name != NULL));
  osalDbgAssert(devp->state == PLAYER_READY,
              "playerRun(), invalid state");

  msg_t res = FR_OK;
  res = get_wav_information(file_name, &devp->song);
  if (res) {
    return res;
  }

  /*
   * TODO change configuration for each song
   * Starting and configuring the I2S driver 2.
   */
  i2sStart(devp->config->i2sp, &i2scfg);

  FIL file;
  UINT bred;
  uint8_t full = 0;
  uint32_t size = devp->song.information.data_size;

  if ((res = f_open(&file, devp->song.name, FA_READ))) {
    return res;
  }

  /* Jump to data */
  f_lseek(&file, (devp->song.information.overall_size - devp->song.information.data_size + 8));

  /*
   * Starting continuous I2S transfer.
   */
  i2sStartExchange(devp->config->i2sp);

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

  i2sStopExchange(devp->config->i2sp);
  i2sStop(devp->config->i2sp);
  return f_close(&file);
}

/**
 * @brief   Initializes an instance.
 *
 * @param[out] devp     pointer to the @p PLAYERDriver object
 *
 * @init
 */
void playerInit(PLAYERDriver *devp) {

  chBSemObjectInit(&full_bsem, true);
  chBSemObjectInit(&half_bsem, true);

  devp->config = NULL;
  devp->state = PLAYER_STOP;
}

/**
 * @brief   Configures and activates CS43L22 Driver peripheral.
 *
 * @param[in] devp      pointer to the @p PLAYERDriver object
 * @param[in] config    pointer to the @p PLAYERConfig object
 *
 * @api
 */
msg_t playerStart(PLAYERDriver *devp, const PLAYERConfig *config) {

  osalDbgCheck((devp != NULL) && (config != NULL));
  osalDbgAssert((devp->state == PLAYER_STOP) || (devp->state == PLAYER_READY),
              "playerStart(), invalid state");

  devp->config = config;
  devp->state = PLAYER_READY;

  return MSG_OK;
}

/**
 * @brief   Configures and activates CS43L22 Driver peripheral.
 *
 * @param[in] devp      pointer to the @p PLAYERDriver object
 * @param[in] config    pointer to the @p PLAYERConfig object
 *
 * @api
 */
msg_t playerStop(PLAYERDriver *devp) {

  osalDbgCheck(devp != NULL);
  osalDbgAssert((devp->state == PLAYER_STOP) || (devp->state == PLAYER_READY),
              "playerStop(), invalid state");

  devp->config = NULL;
  devp->state = PLAYER_STOP;

  return MSG_OK;
}

/** @} */
