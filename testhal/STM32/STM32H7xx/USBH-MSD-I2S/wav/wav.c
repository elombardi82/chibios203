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

/*===========================================================================*/
/* Driver local macros.                                                      */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver functions.                                                         */
/*===========================================================================*/
/**
 * @brief   Extract information from wav file.
 *
 * @param[in]  file_name file name
 * @param[out] headerp   pointer to header struct
 *
 * @return               The operation status.
 *
 * @api
 */
msg_t getWavInformation (const char *file_name, wav_header_t *headerp) {

  FIL file;
  msg_t res = FR_OK;
  UINT bred;

  if ((res = f_open(&file, file_name, FA_READ))) {
    return res;
  }

  if ((res = f_read(&file, headerp->riff, sizeof headerp->riff, &bred))) {
    goto exit;
  }

  if ((res = f_read(&file, &headerp->overall_size, sizeof headerp->overall_size, &bred))) {
    goto exit;
  }

  if ((res = f_read(&file, headerp->wave, sizeof headerp->wave, &bred))) {
    goto exit;
  }

  if ((res = f_read(&file, headerp->fmt_chunk_marker, sizeof headerp->fmt_chunk_marker, &bred))) {
    goto exit;
  }

  if ((res = f_read(&file, &headerp->length_of_fmt, sizeof headerp->length_of_fmt, &bred))) {
    goto exit;
  }

  if ((res = f_read(&file, &headerp->format_type, sizeof headerp->format_type, &bred))) {
    goto exit;
  }

  if ((res = f_read(&file, &headerp->channels, sizeof headerp->channels, &bred))) {
    goto exit;
  }

  if ((res = f_read(&file, &headerp->sample_rate, sizeof headerp->sample_rate, &bred))) {
    goto exit;
  }

  if ((res = f_read(&file, &headerp->byterate, sizeof headerp->byterate, &bred))) {
    goto exit;
  }

  if ((res = f_read(&file, &headerp->block_align, sizeof headerp->block_align, &bred))) {
    goto exit;
  }

  if ((res = f_read(&file, &headerp->bits_per_sample, sizeof headerp->bits_per_sample, &bred))) {
    goto exit;
  }

  if ((res = f_read(&file, &headerp->data_chunk_header, sizeof headerp->data_chunk_header, &bred))) {
    goto exit;
  }

  if ((res = f_read(&file, &headerp->data_size, sizeof headerp->data_size, &bred))) {
    goto exit;
  }

  /* if LIST field is present */
  if (headerp->data_chunk_header[0] == 'L') {
    char *tmp;
    tmp = chHeapAlloc(NULL, headerp->data_size);
    if (tmp == NULL) {
      res = -1;
      goto exit;
    }
    f_read(&file, tmp, headerp->data_size, &bred);
    chHeapFree(tmp);

    if ((res = f_read(&file, headerp->data_chunk_header, sizeof headerp->data_chunk_header, &bred))) {
      goto exit;
    }

    if ((res = f_read(&file, &headerp->data_size, sizeof headerp->data_size, &bred))) {
      goto exit;
    }
  }

exit:
  f_close(&file);
  return res;
}

/** @} */
