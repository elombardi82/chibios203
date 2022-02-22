/*
    ChibiOS - Copyright (C) 2016..2019 Rocco Marco Guglielmi

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
 * @file    cs43l22.c
 * @brief   CS43L22 MEMS interface module code.
 *
 * @addtogroup CS43L22
 * @ingroup EX_ST
 * @{
 */

#include "hal.h"
#include "cs43l22.h"

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/
#if 0
static int map(int x, int in_min, int in_max, int out_min, int out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
#endif
/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

#if (CS43L22_USE_I2C) || defined(__DOXYGEN__)
/**
 * @brief   Reads registers value using I2C.
 * @pre     The I2C interface must be initialized and the driver started.
 *
 * @param[in]  devp      pointer to the @p CS43L22Driver object
 * @param[in]  reg       first sub-register address
 * @param[out] rxbuf     pointer to an output buffer
 * @param[in]  n         number of consecutive register to read
 * @return               the operation status.
 *
 * @notapi
 */
static msg_t cs43l22I2CReadRegister(CS43L22Driver *devp, uint8_t reg, uint8_t *rxbuf, size_t n) {

  uint8_t txbuf = reg;
  msg_t status;

  if (devp->config->i2cp->state == I2C_STOP) {
    i2cStart(devp->config->i2cp, devp->config->i2cconfig);
  }

  status =  i2cMasterTransmitTimeout(devp->config->i2cp, devp->config->slaveaddress, &txbuf,
                                     1, rxbuf, n, CS43L22_TIMEOUT);

  if (status != MSG_OK) {
    i2cStop(devp->config->i2cp);
  }

  return status;
}

/**
 * @brief   Writes a value into a register using I2C.
 * @pre     The I2C interface must be initialized and the driver started.
 *
 * @param[in] devp       pointer to the @p CS43L22Driver object
 * @param[in] txbuf      buffer containing sub-address value in first position
 *                       and values to write
 * @param[in] n          size of txbuf.
 * @return               the operation status.
 *
 * @notapi
 */
static msg_t cs43l22I2CWriteRegister(CS43L22Driver *devp, uint8_t *txbuf, size_t n) {

  msg_t status;

  if (devp->config->i2cp->state == I2C_STOP) {
    i2cStart(devp->config->i2cp, devp->config->i2cconfig);
  }

  status = i2cMasterTransmitTimeout(devp->config->i2cp, devp->config->slaveaddress, txbuf, n, NULL, 0,
                                    CS43L22_TIMEOUT);

  if (status != MSG_OK) {
    i2cStop(devp->config->i2cp);
  }

  return status;
}
#endif /* CS43L22Driver */

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
  * @brief Sets higher or lower the codec volume level.
  * @param DeviceAddr: Device address on communication Bus.
  * @param Volume: a byte value from 0 to 255 (refer to codec registers
  *         description for more details).
  * @retval 0 if correct communication, else wrong communication
  */
msg_t cs43l22SetVolume(CS43L22Driver *devp, uint8_t volume) {
  osalDbgCheck(devp != NULL);
  osalDbgAssert(devp->state == CS43L22_READY,
                "cs43l22SetVolume(), invalid state");

  msg_t status;
#if 0
  uint8_t convertedvol = VOLUME_CONVERT(volume);

  if(volume > 0xE6) {
    /* Set the Master volume */
    counter += CODEC_IO_Write(DeviceAddr, CS43L22_REG_MASTER_A_VOL, convertedvol - 0xE7);
    counter += CODEC_IO_Write(DeviceAddr, CS43L22_REG_MASTER_B_VOL, convertedvol - 0xE7);
  }
  else {
    /* Set the Master volume */
    counter += CODEC_IO_Write(DeviceAddr, CS43L22_REG_MASTER_A_VOL, convertedvol + 0x19);
    counter += CODEC_IO_Write(DeviceAddr, CS43L22_REG_MASTER_B_VOL, convertedvol + 0x19);
  }

  return counter;
#endif

  uint8_t cmd[2] = {CS43L22_MST_A_VOL, volume};
  cs43l22I2CWriteRegister(devp, cmd, sizeof cmd);
  cmd[0] = CS43L22_MST_B_VOL;
  return cs43l22I2CWriteRegister(devp, cmd, sizeof cmd);
}

/**
 * @brief   Get Revision ID.
 *
 * @param[in] devp      pointer to the @p CS43L22Driver object
 * @param[out] rev_id   pointer to the @p revision id
 *
 * @api
 */
msg_t cs43l22GetRevID(CS43L22Driver *devp, uint8_t *rev_id) {

  osalDbgCheck(devp != NULL);
  osalDbgAssert(devp->state == CS43L22_READY,
                "cs43l22GetRevID(), invalid state");
  uint8_t tmp;
  msg_t status;

  status = cs43l22I2CReadRegister(devp, CS43L22_ID, &tmp, 1);

  if (!status) {
    *rev_id = tmp & CS43L22_ID_REVID;
  }

  return status;
}

/**
 * @brief   Get Chip ID.
 *
 * @param[in] devp      pointer to the @p CS43L22Driver object
 * @param[out] chip_id   pointer to the @p chip id
 *
 * @api
 */
msg_t cs43l22GetChipID(CS43L22Driver *devp, uint8_t *chip_id) {

  osalDbgCheck(devp != NULL);
  osalDbgAssert(devp->state == CS43L22_READY,
                "cs43l22GetChipID(), invalid state");
  uint8_t tmp;
  msg_t status;

  status = cs43l22I2CReadRegister(devp, CS43L22_ID, &tmp, 1);

  if (!status) {
    *chip_id = (tmp & CS43L22_ID_CHIPID) >> CS43L22_ID_CHIPID_Pos;
  }

  return status;
}

/**
 * @brief   Power on.
 *
 * @param[in] devp      pointer to the @p CS43L22Driver object
 *
 * @api
 */
msg_t cs43l22PowerOn(CS43L22Driver *devp) {

  osalDbgCheck(devp != NULL);
  osalDbgAssert(devp->state == CS43L22_READY,
                "cs43l22PowerOn(), invalid state");

  uint8_t cmd[2] = {CS43L22_PWR_CTRL1, CS43L22_POWER_UP};

  return cs43l22I2CWriteRegister(devp, cmd, sizeof cmd);
}

/**
 * @brief   Power off.
 *
 * @param[in] devp      pointer to the @p CS43L22Driver object
 *
 * @api
 */
msg_t cs43l22PowerOff(CS43L22Driver *devp) {

  osalDbgCheck(devp != NULL);
  osalDbgAssert(devp->state == CS43L22_READY,
                "cs43l22PowerOff(), invalid state");

  uint8_t cmd[2] = {CS43L22_PWR_CTRL1, CS43L22_POWER_DOWN};

  return cs43l22I2CWriteRegister(devp, cmd, sizeof cmd);
}

/**
 * @brief   Initializes an instance.
 *
 * @param[out] devp     pointer to the @p CS43L22Driver object
 *
 * @init
 */
void cs43l22Init(CS43L22Driver *devp) {
  devp->config = NULL;
  devp->state = CS43L22_STOP;
}

/**
 * @brief   Configures and activates CS43L22 Driver peripheral.
 *
 * @param[in] devp      pointer to the @p CS43L22Driver object
 * @param[in] config    pointer to the @p CS43L22Config object
 *
 * @api
 */
msg_t cs43l22Start(CS43L22Driver *devp, const CS43L22Config *config) {

  osalDbgCheck((devp != NULL) && (config != NULL));
  osalDbgAssert((devp->state == CS43L22_STOP) || (devp->state == CS43L22_READY),
              "cs43l22Start(), invalid state");

  uint8_t cmd[2];
  devp->config = config;

#if CS43L22_USE_I2C
  /* Start I2C. */
  i2cStart(devp->config->i2cp, devp->config->i2cconfig);
#endif /* CS43L22_USE_I2C */
  osalThreadSleepMilliseconds(10);

  /* Power Down */
  cmd[0] = CS43L22_PWR_CTRL1;
  cmd[1] = CS43L22_POWER_DOWN;

  cs43l22I2CWriteRegister(devp, cmd, sizeof cmd);

  /* Sequence Power Up */
  cmd[0] = 0x00;
  cmd[1] = 0x99;
  cs43l22I2CWriteRegister(devp, cmd, sizeof cmd);

  cmd[0] = 0x47;
  cmd[1] = 0x80;
  cs43l22I2CWriteRegister(devp, cmd, sizeof cmd);

  cmd[0] = 0x32;
  cmd[1] = 0x80;
  cs43l22I2CWriteRegister(devp, cmd, sizeof cmd);

  cmd[0] = 0x32;
  cmd[1] = 0x00;
  cs43l22I2CWriteRegister(devp, cmd, sizeof cmd);

  cmd[0] = 0x00;
  cmd[1] = 0x00;
  cs43l22I2CWriteRegister(devp, cmd, sizeof cmd);

  cmd[0] = CS43L22_IFACE_CTRL1;
  cmd[1] = 0x07;
  cs43l22I2CWriteRegister(devp, cmd, sizeof cmd);

  /* Power On */
  cmd[0] = CS43L22_PWR_CTRL1;
  cmd[1] = CS43L22_POWER_UP;

  cs43l22I2CWriteRegister(devp, cmd, sizeof cmd);

  devp->state = CS43L22_READY;

  return MSG_OK;
}

/**
 * @brief   Deactivates the CS43L22 Driver peripheral.
 *
 * @param[in] devp       pointer to the @p CS43L22Driver object
 *
 * @api
 */
void cs43l22Stop(CS43L22Driver *devp) {

  osalDbgCheck(devp != NULL);
  osalDbgAssert((devp->state == CS43L22_STOP) ||
                (devp->state == CS43L22_READY),
                "cs43l22Stop(), invalid state");

  if (devp->state == CS43L22_READY) {
#if CS43L22_USE_I2C
    i2cStop(devp->config->i2cp);
#endif /* CS43L22_USE_I2C */
  }
  devp->state = CS43L22_STOP;
}
/** @} */
