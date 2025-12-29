/**
 * @file      port_i2c.h
 * @author:   Shubhendu B B
 * @date:     27/12/2025
 * @brief
 * @details
 *
 * @copyright
 *
 **/
#ifndef _PORT_I2C_H_
#define _PORT_I2C_H_

#include <stdint.h>

typedef enum port_i2c_tagFnStatus {
    PORT_I2C_STATUS_OK = 0,      /**< Operation successful */
    PORT_I2C_STATUS_BUSY = 1,    /**< I2C bus is busy */
    PORT_I2C_STATUS_TIMEOUT = 2, /**< Operation timed out */

    PORT_I2C_STATUS_ERROR = -1, /**< Error status */
    PORT_I2C_STATUS_ERROR_NULL_ARG = -2,
    PORT_I2C_STATUS_ERROR_REG_ADD_LEN_ZERO = -3,
    PORT_I2C_STATUS_ERROR_INVALID_STATE = -4,
    PORT_I2C_STATUS_ERROR_MAX_PDU = -5,
    PORT_I2C_STATUS_ERROR_DRV_INIT = -6,
    PORT_I2C_STATUS_ERROR_TIMEOUT = -7,
    PORT_I2C_STATUS_ERROR_NACK = -8,       /**< NACK received during transfer. */
    PORT_I2C_STATUS_ERROR_BUS = -9,        /**< Bus error during transfer (misplaced START/STOP). */
    PORT_I2C_STATUS_ERROR_ARB_LOST = -10,  /**< Arbitration lost during transfer. */
    PORT_I2C_STATUS_ERROR_USG_FAULT = -11, /**< Usage fault. */
    PORT_I2C_STATUS_ERROR_SW = -12,        /**< SW fault. */
} port_i2c_Status_t;

typedef struct port_i2c_tagInstanceConfig {
    void* pvCtx;
    uint16_t id;      /*!< I2C identifier */
    uint16_t sdaPort; /*!< I2C SDA GPIO port and pin */
    uint16_t sdaPin;  /*!< I2C SDA GPIO port and pin */
    uint16_t sclPort; /*!< I2C SCL GPIO port and pin */
    uint16_t sclPin;  /*!< I2C SCL GPIO port and pin */
    struct {
        uint8_t isClockStretching : 1;        //!< (applicable only for master role)
        uint8_t isBroadCastingSupported : 1;  //!< (applicable only for slave role)
    } flags;
} port_i2c_InstanceConfig_t;

#ifdef __cplusplus
extern "C" {
#endif

int port_i2c_init(void* pvInstanceConfig);
int port_i2c_deinit(void* pvInstanceConfig);

#ifdef __cplusplus
}
#endif

#endif /* @end  _PORT_I2C_H_*/