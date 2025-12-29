/**
 * @file      driver_i2c.h
 * @author:   Shubhendu B B
 * @date:     25/12/2025
 * @brief
 * @details
 *
 * @copyright
 *
 **/
#ifndef _DRIVER_I2C_H_
#define _DRIVER_I2C_H_

#include <stdbool.h>
#include <stdint.h>

#define DRIVER_I2C_INSTANCE_COUNT 2

/**
 * @brief Function status enumeration.
 */
typedef enum driver_i2c_tagFnStatus {
    DRIVER_I2C_STATUS_OK = 0,      /**< Operation successful */
    DRIVER_I2C_STATUS_BUSY = 1,    /**< I2C bus is busy */
    DRIVER_I2C_STATUS_TIMEOUT = 2, /**< Operation timed out */

    DRIVER_I2C_STATUS_ERROR = -1, /**< Error status */
    DRIVER_I2C_STATUS_ERROR_NULL_ARG = -2,
    DRIVER_I2C_STATUS_ERROR_REG_ADD_LEN_ZERO = -3,
    DRIVER_I2C_STATUS_ERROR_INVALID_STATE = -4,
    DRIVER_I2C_STATUS_ERROR_MAX_PDU = -5,
    DRIVER_I2C_STATUS_ERROR_DRV_INIT = -6,
    DRIVER_I2C_STATUS_ERROR_TIMEOUT = -7,
    DRIVER_I2C_STATUS_ERROR_NACK = -8, /**< NACK received during transfer. */
    DRIVER_I2C_STATUS_ERROR_BUS = -9,  /**< Bus error during transfer (misplaced START/STOP). */
    DRIVER_I2C_STATUS_ERROR_ARB_LOST = -10,  /**< Arbitration lost during transfer. */
    DRIVER_I2C_STATUS_ERROR_USG_FAULT = -11, /**< Usage fault. */
    DRIVER_I2C_STATUS_ERROR_SW = -12,        /**< SW fault. */
} driver_i2c_Status_t;

/**
 * @brief Transaction type enumeration.
 */
typedef enum driver_i2c_tagTxnType {
    DRIVER_I2C_TXN_TYPE_READ = 0,  /**< Read transaction */
    DRIVER_I2C_TXN_TYPE_WRITE,     /**< Write transaction */
    DRIVER_I2C_TXN_TYPE_MEM_READ,  /**< Memory read transaction */
    DRIVER_I2C_TXN_TYPE_MEM_WRITE, /**< Memory write transaction */
    DRIVER_I2C_TXN_TYPE_CMD_READ   /**< Command read transaction */
} driver_i2c_txnType_t;

// typedef struct driver_i2c_tagTxnHandle driver_i2c_comm_port_t;

typedef int (*driver_i2c_initFn_t)(void* pvI2CCommPortDef);
typedef int (*driver_i2c_deinitFn_t)(void* pvI2CCommPortDef);
typedef int (*driver_i2c_writeMasterFn_t)(
    void* pvCtx, uint16_t slaveAddr, bool is10BitAddr, uint8_t* pDataBuff,
    uint8_t* pDataBuffSize); /* pDataBuffSize is limited to 8-bit by design (max 255 bytes per
                                transfer) */

typedef int (*driver_i2c_readMasterFn_t)(
    void* pvCtx, uint16_t slaveAddr, bool is10BitAddr, uint8_t* pDataBuff,
    uint8_t* pDataBuffSize); /* pDataBuffSize is limited to 8-bit by design (max 255 bytes per
                                transfer) */
typedef bool (*driver_i2c_isTimeoutFn_t)(uint64_t startMillis, uint64_t timeout);
typedef uint64_t (*driver_i2c_getTimestampFn_t)(void);

/* Example timeout function ::
static bool is_timeout(uint64_t startMillis, uint64_t timeout) {
if ((port_timer_GetMillis() - startMillis) > timeout) {
return true;
} else {
return false;
}
} */

typedef struct driver_i2c_tagInstanceConfig {
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
} driver_i2c_InstanceConfig_t;

/**
 * @brief Transaction handle structure.
 */
typedef struct driver_i2c_tagTxnHandle {
    driver_i2c_InstanceConfig_t instanceConfig;
    driver_i2c_initFn_t portInitFn;
    driver_i2c_deinitFn_t portDeinitFn;
    driver_i2c_writeMasterFn_t portWriteMasterFn;
    driver_i2c_readMasterFn_t portReadMasterFn;
    driver_i2c_getTimestampFn_t getTimestamp;
    driver_i2c_isTimeoutFn_t portIsTimeoutFn;
} driver_i2c_comm_port_t;

/**
 * @brief Transaction data structure.
 */
typedef struct driver_i2c_tagTxnData {
    uint16_t slaveAddr;
    struct {
        uint8_t is10BitAddr;
    };
    uint8_t endianness;           /*!< Data endianness */
    driver_i2c_txnType_t txnType; /*!< Type of transaction */
    uint8_t* pRegAddrBuff;        /*!< Memory address */
    uint8_t regAddrSize;          /*!< Memory address size */
    union {
        uint8_t* pDataBuff;         /*!< Legacy generic pointer (for backward compatibility).
                 Prefer pTxDataBuff or pRxDataBuff in new code. */
        uint8_t* pRxDataBuff;       /*!< Pointer for RX operations (data will be written here). */
        const uint8_t* pTxDataBuff; /*!< Pointer for TX operations (data will not be modified). */
    };
    uint16_t dataBuffSize; /*!< Length of transaction data */
    uint16_t timeout;      /*!< Timeout for the transaction in milliseconds*/
} driver_i2c_txnData_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes the I2C peripheral.
 *
 * This function configures the I2C peripheral based on the settings
 * specified in the transaction handle.
 *
 * @param pI2CCommPortDef Pointer to the transaction handle.
 * @return Status of the initialization (success, busy, error, timeout).
 */
driver_i2c_Status_t driver_i2c_init(driver_i2c_comm_port_t* pI2CCommPortDef);

/**
 * @brief Deinitializes the I2C peripheral.
 *
 * This function releases any resources allocated for the I2C peripheral.
 *
 * @param pI2CCommPortDef Pointer to the transaction handle.
 * @return Status of the deinitialization (success, busy, error, timeout).
 */
driver_i2c_Status_t driver_i2c_deinit(driver_i2c_comm_port_t* pI2CCommPortDef);

/**
 * @brief Initiates an I2C data transfer.
 *
 * This function executes the I2C transaction as specified by the
 * transaction handle and transaction data provided.
 *
 * @param pI2CCommPortDef Pointer to the transaction handle.
 * @param pTxnData Pointer to the transaction data.
 * @return Status of the transaction (success, busy, error, timeout).
 */
driver_i2c_Status_t driver_i2c_transfer(driver_i2c_comm_port_t* pI2CCommPortDef,
                                        driver_i2c_txnData_t* pTxnData);

driver_i2c_Status_t driver_i2c_transfer_async(driver_i2c_comm_port_t* pI2CCommPortDef,
                                              driver_i2c_txnData_t* pTxnData);

driver_i2c_Status_t driver_i2c_abort(driver_i2c_comm_port_t* pI2CCommPortDef,
                                     driver_i2c_txnData_t* pTxnData);

#ifdef __cplusplus
}
#endif

#endif /* @end  _DRIVER_I2C_H_ */
