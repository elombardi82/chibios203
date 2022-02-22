/*
    ChibiOS - Copyright (C) 2016..2019 Edoardo Lombardi

    This file is part of ChibiOS.

    ChibiOS is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    ChibiOS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

/**
 * @file    cs43l22.h
 * @brief   CS43L22 interface module header.
 *
 * @addtogroup CS43L22
 * @ingroup EX_ST
 * @{
 */
#ifndef _CS43L22_H_
#define _CS43L22_H_
/*
 * PB9  SDA I2C1
 * PB6  SCL I2C1
 *
 * PC7  I2S3_MCK
 * PC10 I2S3_SCK
 * PC12 I2S3_SD
 * PA4  I2S3_WS
 *
 * PD4  RESET
 */
/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @name    Version identification
 * @{
 */
/**
 * @brief   CS43L22 driver version string.
 */
#define CS43L22_VERSION                    "1.0.0"

/**
 * @brief   CS43L22 driver version major number.
 */
#define CS43L22_MAJOR                      1

/**
 * @brief   CS43L22 driver version minor number.
 */
#define CS43L22_MINOR                      0

/**
 * @brief   CS43L22 driver version patch number.
 */
#define CS43L22_PATCH                      0
/** @} */

/**
 * @name   CS43L22 register addresses
 * @{
 */
#define CS43L22_ID                         0x01
#define CS43L22_PWR_CTRL1                  0x02
#define CS43L22_PWR_CTRL2                  0x04
#define CS43L22_CLK_CTRL                   0x05
#define CS43L22_IFACE_CTRL1                0x06
#define CS43L22_IFACE_CTRL2                0x07
#define CS43L22_PASS_A                     0x08
#define CS43L22_PASS_B                     0x09
#define CS43L22_ANALOG_ZC                  0x0A
#define CS43L22_GANG_CTRL                  0x0C
#define CS43L22_PLAY_CTRL1                 0x0D
#define CS43L22_MISC_CTRL                  0x0E
#define CS43L22_PLAY_CTRL2                 0x0F
#define CS43L22_PASS_A_VOL                 0x14
#define CS43L22_PASS_B_VOL                 0x15
#define CS43L22_PCM_A_VOL                  0x1A
#define CS43L22_PCM_B_VOL                  0x1B
#define CS43L22_BEEP_FREQ                  0x1C
#define CS43L22_BEEP_VOL                   0x1D
#define CS43L22_BEEP_CONF                  0x1E
#define CS43L22_TONE_CTRL                  0x1F
#define CS43L22_MST_A_VOL                  0x20
#define CS43L22_MST_B_VOL                  0x21
#define CS43L22_HP_A_VOL                   0x22
#define CS43L22_HP_B_VOL                   0x23
#define CS43L22_SPK_A_VOL                  0x24
#define CS43L22_SPK_B_VOL                  0x25
#define CS43L22_PCM_CHAN_SWAP              0x26
#define CS43L22_LIM_CTRL1                  0x27
#define CS43L22_LIM_CTRL2                  0x28
#define CS43L22_LIM_ATTAC_RATE             0x29
#define CS43L22_STATUS                     0x2E
#define CS43L22_BATTERY_COMP               0x2F
#define CS43L22_BATTERY_LEV                0x30
#define CS43L22_SPK_STATUS                 0x31
#define CS43L22_CHARGE_PUMP_FREQ           0x34

/** @} */

/**
 * @name    CS43L22_ID register bits definitions
 * @{
 */
#define CS43L22_ID_REVID_Pos           (0U)
#define CS43L22_ID_REVID_Msk           (0x7U << CS43L22_ID_REVID_Pos)
#define CS43L22_ID_REVID               CS43L22_ID_REVID_Msk
#define CS43L22_ID_CHIPID_Pos          (3U)
#define CS43L22_ID_CHIPID_Msk          (0x1F << CS43L22_ID_CHIPID_Pos)
#define CS43L22_ID_CHIPID              CS43L22_ID_CHIPID_Msk
/** @} */

/**
 * @name    CS43L22_PWR_CTRL1 register bits definitions
 * @{
 */
#define CS43L22_PWR_CTRL1_PWR_Pos      (0U)
#define CS43L22_PWR_CTRL1_PWR_Msk      (0xFFU << CS43L22_PWR_CTRL1_PWR_Pos)
#define CS43L22_PWR_CTRL1_PWR          CS43L22_PWR_CTRL1_PWR_Msk
/** @} */

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Configuration options
 * @{
 */

/**
 * @brief   CS43L22 I2C interface switch.
 * @details If set to @p TRUE the support for I2C is included.
 * @note    The default is @p TRUE.
 */
#if !defined(CS43L22_USE_I2C) || defined(__DOXYGEN__)
#define CS43L22_USE_I2C                     TRUE
#endif

/**
 * @brief   CS43L22 I2C timeout.
 */
#if !defined(CS43L22_TIMEOUT) || defined(__DOXYGEN__)
#define CS43L22_TIMEOUT                     TIME_MS2I(500)
#endif

/**
 * @brief   CS43L22 shared I2C switch.
 * @details If set to @p TRUE the device acquires I2C bus ownership
 *          on each transaction.
 * @note    The default is @p FALSE. Requires I2C_USE_MUTUAL_EXCLUSION.
 */
#if !defined(CS43L22_SHARED_I2C) || defined(__DOXYGEN__)
#define CS43L22_SHARED_I2C                  FALSE
#endif

/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if CS43L22_USE_I2C && !HAL_USE_I2C
#error "CS43L22_USE_I2C requires HAL_USE_I2C"
#endif

#if CS43L22_SHARED_I2C && !I2C_USE_MUTUAL_EXCLUSION
#error "CS43L22_SHARED_I2C requires I2C_USE_MUTUAL_EXCLUSION"
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @name    CS43L22 data structures and types.
 * @{
 */
/**
 * @brief   Structure representing a CS43L22 driver.
 */
typedef struct CS43L22Driver CS43L22Driver;

/**
 * @brief  CS43L22 slave Address.
 */
/**
 * @brief  CS43L22 slave address
 */
typedef enum {
  CS43L22_SAD_DEFAULT = 0x4A,       /**< Default slave address             */
} cs4l22_sad_t;

/**
 * @brief CS43L22 power.
 */
typedef enum {
  CS43L22_POWER_DOWN = 0x01,        /**< Power Down.                        */
  CS43L22_POWER_UP = 0x9E           /**< Power Up.                        */
} cs43l22_pwr_t;

/**
 * @brief   Driver state machine possible states.
 */
typedef enum {
  CS43L22_UNINIT = 0,               /**< Not initialized.                   */
  CS43L22_STOP = 1,                 /**< Stopped.                           */
  CS43L22_READY = 2,                /**< Ready.                             */
} cs43l22_state_t;

/**
 * @brief CS43L22 configuration structure.
 */
typedef struct {
#if (CS43L22_USE_I2C) || defined(__DOXYGEN__)
  /**
   * @brief I2C driver associated to this CS43L22.
   */
  I2CDriver                 *i2cp;
  /**
   * @brief I2C configuration associated to this CS43L22 accelerometer
   *        subsystem.
   */
  const I2CConfig           *i2cconfig;
  /**
   * @brief CS43L22 Slave Address
   */
  cs4l22_sad_t              slaveaddress;
#endif /* CS43L22_USE_I2C */
} CS43L22Config;

/**
 * @brief  CS43L22.
 */
struct CS43L22Driver {
  /**
   * @brief   Driver state.
   */
  cs43l22_state_t           state;

  /**
   * @brief   Current configuration data.
   */
  const CS43L22Config       *config;
};

/** @} */

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
void cs43l22Init(CS43L22Driver *devp);
msg_t cs43l22Start(CS43L22Driver *devp, const CS43L22Config *config);
void cs43l22Stop(CS43L22Driver *devp);
msg_t cs43l22GetRevID(CS43L22Driver *devp, uint8_t *rev_id);
msg_t cs43l22GetChipID(CS43L22Driver *devp, uint8_t *chip_id);
msg_t cs43l22PowerOn(CS43L22Driver *devp);
msg_t cs43l22PowerOff(CS43L22Driver *devp);
msg_t cs43l22SetVolume(CS43L22Driver *devp, uint8_t volume);
#ifdef __cplusplus
}
#endif

#endif /* _CS43L22_H_ */

/** @} */
