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
 * @file    wav.h
 * @brief   wav files.
 *
 * The header structure is 44 bytes long and has the following structure:
 *
 * Positions   Sample Value    Description
 *  1 – 4        “RIFF”        Marks the file as a riff file. Characters are each 1 byte long.
 *  5 – 8       File size      Size of the overall file – 8 bytes, in bytes (32-bit integer).
 *                             Typically, you’d fill this in after creation.
 *  9 -12       “WAVE”         File Type Header. For our purposes, it always equals “WAVE”.
 *  13-16       “fmt “         Format chunk marker. Includes trailing null
 *  17-20        16            Length of format data as listed above
 *  21-22         1            Type of format (1 is PCM) – 2 byte integer
 *  23-24         2            Number of Channels – 2 byte integer
 *  25-28        44100         Sample Rate – 32 byte integer.
 *                             Common values are 44100 (CD), 48000 (DAT).
 *                             Sample Rate = Number of Samples per second, or Hertz.
 *  29-32       176400         (Sample Rate * BitsPerSample * Channels) / 8.
 *  33-34         4            (BitsPerSample * Channels) /
 *                             8.1 – 8 bit mono2 – 8 bit stereo/16 bit mono4 – 16 bit stereo
 *  35-36        16            Bits per sample
 *  37-40       “data”         “data” chunk header. Marks the beginning of the data section.
 *  41-44     File size (data) Size of the data section.
 *
 *
 *  LIST and size list can be present before "data" section.
 *
 * @{
 */

#ifndef WAV_H
#define WAV_H

/* WAVE file header format */
typedef struct {
    unsigned char riff[4];                      /* RIFF string                                                        */
    unsigned int  overall_size;                 /* overall size of file in bytes                                      */
    unsigned char wave[4];                      /* WAVE string                                                        */
    unsigned char fmt_chunk_marker[4];          /* fmt string with trailing null char                                 */
    unsigned int  length_of_fmt;                /* length of the format data                                          */
    uint16_t      format_type;                  /* format type. 1-PCM, 3- IEEE float, 6 - 8bit A law, 7 - 8bit mu law */
    uint16_t      channels;                     /* no.of channels                                                     */
    unsigned int  sample_rate;                  /* sampling rate (blocks per second)                                  */
    unsigned int  byterate;                     /* SampleRate * NumChannels * BitsPerSample / 8                       */
    uint16_t      block_align;                  /* NumChannels * BitsPerSample/8                                      */
    uint16_t      bits_per_sample;              /* bits per sample, 8- 8bits, 16- 16 bits etc                         */
    unsigned char data_chunk_header [4];        /* DATA string or FLLR string                                         */
    unsigned int  data_size;                    /* NumSamples * NumChannels * BitsPerSample/8                         */
                                                /* size of the next chunk that will be read                           */
} wav_header_t;

/* Song object  */
typedef struct {
  char           *name;
  wav_header_t   information;
} wav_song_t;

/**
 * @brief   Driver state machine possible states.
 */
typedef enum {
  PLAYER_UNINIT = 0,               /**< Not initialized.                   */
  PLAYER_STOP =   1,               /**< Stopped.                           */
  PLAYER_READY =  2,               /**< Ready.                             */
  PLAYER_ACTIVE = 3,               /**< Ready.                             */
} player_state_t;

/**
 * @brief   Structure representing a PLAYER driver.
 */
typedef struct PLAYERDriver PLAYERDriver;

/**
 * @brief PLAYER configuration structure.
 */
typedef struct {
  /**
   * @brief I2S driver associated.
   */
  I2SDriver                 *i2sp;
} PLAYERConfig;

/**
 * @brief  PLAYER.
 */
struct PLAYERDriver {
  /**
   * @brief   Driver state.
   */
  player_state_t           state;

  /**
   * @brief   Current configuration data.
   */
  const PLAYERConfig       *config;

  /**
   * @brief   Current song played.
   */
  wav_song_t               song;
};

#ifdef __cplusplus
extern "C" {
#endif
void playerInit(PLAYERDriver *devp);
msg_t playerStart(PLAYERDriver *devp, const PLAYERConfig *config);
msg_t playerStop(PLAYERDriver *devp);
msg_t playerRun(PLAYERDriver *devp, const char *file_name);
#ifdef __cplusplus
}
#endif

#endif  /* WAV_H */

/** @} */
