/**
 * @file      driver_i2c.c
 * @author:   Shubhendu B B
 * @date:     11/11/2024
 * @brief
 * @details
 *
 * @copyright
 *
 **/

//
#include "driver_i2c.h"

//
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define TAG "DRIVER_I2C"

#ifndef LOG_LEVEL

#define LOG_LEVEL_NONE 0
#define LOG_LEVEL_ERROR 1
#define LOG_LEVEL_INFO 2
#define LOG_LEVEL_DEBUG 3

#define LOG_LEVEL LOG_LEVEL_DEBUG
#endif

#if LOG_LEVEL < LOG_LEVEL_INFO
#define DRIVER_I2C_LOGI(...) \
    do {                     \
    } while (0)
#else
#define DRIVER_I2C_LOGI(tag, fmt, ...)                                           \
    do {                                                                         \
        printf("\x1b[32m%s:%d: " fmt "\x1b[0m\n", tag, __LINE__, ##__VA_ARGS__); \
    } while (0)
#endif

#if LOG_LEVEL < LOG_LEVEL_DEBUG
#define DRIVER_I2C_LOGD(...) \
    do {                     \
    } while (0)
#else
#define DRIVER_I2C_LOGD(tag, fmt, ...)                                           \
    do {                                                                         \
        printf("\x1b[34m%s:%d: " fmt "\x1b[0m\n", tag, __LINE__, ##__VA_ARGS__); \
    } while (0)
#endif

#if LOG_LEVEL < LOG_LEVEL_ERROR
#define DRIVER_I2C_LOGE(...) \
    do {                     \
    } while (0)
#else
#define DRIVER_I2C_LOGE(tag, fmt, ...)                                           \
    do {                                                                         \
        printf("\x1b[31m%s:%d: " fmt "\x1b[0m\n", tag, __LINE__, ##__VA_ARGS__); \
    } while (0)
#endif

#define I2C_TXN_DATA_MAX_LEN 127
#define RETRY_COUNT_MAX 10

typedef struct {
    struct {
        uint8_t isInitialized : 1;
    };
    driver_i2c_comm_port_t commPort;
    driver_i2c_txnData_t txnData;
} I2CInstanceStruct_t;
I2CInstanceStruct_t gInstanceArr[DRIVER_I2C_INSTANCE_COUNT];

static bool gIsDriverInitialized;

driver_i2c_Status_t driver_i2c_init(driver_i2c_comm_port_t* pI2CComPortDef) {
    static driver_i2c_Status_t exeStatus = DRIVER_I2C_STATUS_BUSY;

    if (NULL == pI2CComPortDef) {
        exeStatus = DRIVER_I2C_STATUS_ERROR_NULL_ARG;
        goto label_exitPoint;
    }

    if (!pI2CComPortDef->portInitFn) {
        exeStatus = DRIVER_I2C_STATUS_ERROR_NULL_ARG;
        goto label_exitPoint;
    }

    exeStatus = pI2CComPortDef->portInitFn((void*)pI2CComPortDef->portInitFn);

    if (DRIVER_I2C_STATUS_OK == exeStatus) {
        gIsDriverInitialized = true;
    }

label_exitPoint:
    return exeStatus;
}

driver_i2c_Status_t driver_i2c_deinit(driver_i2c_comm_port_t* pI2CComPortDef) {
    static driver_i2c_Status_t exeStatus = DRIVER_I2C_STATUS_BUSY;

    if (NULL == pI2CComPortDef) {
        exeStatus = DRIVER_I2C_STATUS_ERROR_NULL_ARG;
        goto label_exitPoint;
    }

    if (!pI2CComPortDef->portDeinitFn) {
        exeStatus = DRIVER_I2C_STATUS_ERROR_NULL_ARG;
        goto label_exitPoint;
    }

    exeStatus = pI2CComPortDef->portInitFn((void*)pI2CComPortDef->portDeinitFn);

    memset(gInstanceArr, 0, sizeof(gInstanceArr));
    gIsDriverInitialized = false;

label_exitPoint:
    return exeStatus;
}

driver_i2c_Status_t driver_i2c_transfer(driver_i2c_comm_port_t* pI2CComPortDef,
                                        driver_i2c_txnData_t* pTxnData) {
    typedef enum {
        DRIVER_I2C_TXN_STATE_IDLE = 0,
        DRIVER_I2C_TXN_STATE_WAIT,
        DRIVER_I2C_TXN_STATE_WRITE_REG_ADDR,
        DRIVER_I2C_TXN_STATE_WRITE_DATA_BYTES,
        DRIVER_I2C_TXN_STATE_READ_DATA_BYTES,
        DRIVER_I2C_TXN_STATE_EXIT,
    } driver_i2c_transfer_state_t;
    static driver_i2c_transfer_state_t i2cTransferState[5] = {DRIVER_I2C_TXN_STATE_IDLE};
    static driver_i2c_Status_t exeStatus = DRIVER_I2C_STATUS_OK;
    static driver_i2c_Status_t returnStatus = DRIVER_I2C_STATUS_OK;
    static uint64_t startMillis = 0;
    static uint64_t timeout = 0;
    static uint16_t slaveAddr = 0;
    static bool isSlaveAddr10Bit = false;
    static void* pI2CContext = NULL;
    static uint8_t regAddrBuff[sizeof(double)];
    static uint8_t regAddrSize = 0;
    static uint8_t dataBuff[I2C_TXN_DATA_MAX_LEN];
    static uint8_t dataBuffSize = 0;
    __attribute__((unused)) static uint8_t retryCount = RETRY_COUNT_MAX;  //

    if ((NULL == pI2CComPortDef) || (NULL == pTxnData)) {
#if LOG_LEVEL >= LOG_LEVEL_ERROR
        DRIVER_I2C_LOGE(TAG, "Error: Null argument(s).");
#endif /* @end LOG_LEVEL>=LOG_LEVEL_ERROR */
        return DRIVER_I2C_STATUS_ERROR_NULL_ARG;
    }

    if (!gIsDriverInitialized) {
#if LOG_LEVEL >= LOG_LEVEL_ERROR
        DRIVER_I2C_LOGE(TAG, "Error: Driver not initialized.");
#endif /* @end LOG_LEVEL>=LOG_LEVEL_ERROR */
        return DRIVER_I2C_STATUS_ERROR_DRV_INIT;
    }

    // TODO: test null for r/w/timer functions

    switch (i2cTransferState[0]) {
        case DRIVER_I2C_TXN_STATE_IDLE:
            memset(regAddrBuff, 0, sizeof(regAddrBuff));
            memset(dataBuff, 0, sizeof(dataBuff));
            startMillis = pI2CComPortDef->getTimestamp();  // port_timer_GetMillis();

            if (pTxnData->dataBuffSize > I2C_TXN_DATA_MAX_LEN) {
#if LOG_LEVEL >= LOG_LEVEL_ERROR
                DRIVER_I2C_LOGE(TAG,
                                "Error: Max PDU size exceeded, make data length shorter,below %d.",
                                I2C_TXN_DATA_MAX_LEN);
#endif /* @end LOG_LEVEL>=LOG_LEVEL_ERROR */
                i2cTransferState[0] = DRIVER_I2C_TXN_STATE_EXIT;
                exeStatus = DRIVER_I2C_STATUS_ERROR_MAX_PDU;
                returnStatus = DRIVER_I2C_STATUS_BUSY;
                break;
            }

            // setting up comm params
            timeout = pTxnData->timeout;
            pI2CContext = pI2CComPortDef->instanceConfig.pvCtx;
            slaveAddr = pTxnData->slaveAddr;
            isSlaveAddr10Bit = pTxnData->is10BitAddr;
            regAddrSize = pTxnData->regAddrSize;
            memcpy(regAddrBuff, pTxnData->pRegAddrBuff, regAddrSize);
            dataBuffSize = pTxnData->dataBuffSize;
            memcpy(dataBuff, pTxnData->pDataBuff, dataBuffSize);

            // switch state
            switch (pTxnData->txnType) {
                case DRIVER_I2C_TXN_TYPE_WRITE:
                    i2cTransferState[0] = DRIVER_I2C_TXN_STATE_WRITE_DATA_BYTES;
                    break;

                case DRIVER_I2C_TXN_TYPE_READ:
                    i2cTransferState[0] = DRIVER_I2C_TXN_STATE_READ_DATA_BYTES;
                    break;

                case DRIVER_I2C_TXN_TYPE_MEM_WRITE:
                case DRIVER_I2C_TXN_TYPE_MEM_READ:
                    i2cTransferState[0] =
                        DRIVER_I2C_TXN_STATE_WRITE_REG_ADDR;  // TODO: use switch to define
                                                              // next action type
                                                              // depenfding on request
                                                              // type(mem read, mem write
                                                              // etc.)
                    break;

                default:
                    break;
            }

            returnStatus = DRIVER_I2C_STATUS_BUSY;
            break;

        case DRIVER_I2C_TXN_STATE_WAIT:
            switch (i2cTransferState[1]) {
                case DRIVER_I2C_TXN_STATE_WRITE_REG_ADDR:
                    exeStatus = pI2CComPortDef->portWriteMasterFn(
                        pI2CContext, slaveAddr, isSlaveAddr10Bit, regAddrBuff, &regAddrSize);
                    break;

                case DRIVER_I2C_TXN_STATE_WRITE_DATA_BYTES:
                    exeStatus = pI2CComPortDef->portWriteMasterFn(
                        pI2CContext, slaveAddr, isSlaveAddr10Bit, dataBuff, &dataBuffSize);
                    break;

                case DRIVER_I2C_TXN_STATE_READ_DATA_BYTES:
                    exeStatus = pI2CComPortDef->portReadMasterFn(
                        pI2CContext, slaveAddr, isSlaveAddr10Bit, dataBuff, &dataBuffSize);
                    break;

                default:
                    break;
            }

            switch (exeStatus) {
                case DRIVER_I2C_STATUS_OK:
                    if (DRIVER_I2C_TXN_STATE_READ_DATA_BYTES == i2cTransferState[1]) {
                        memcpy(pTxnData->pDataBuff, dataBuff, dataBuffSize);
                    }

                    i2cTransferState[0] = i2cTransferState[1];
                    break;

                case DRIVER_I2C_STATUS_BUSY:
                    if (pI2CComPortDef->portIsTimeoutFn(startMillis, timeout)) {
                        exeStatus = DRIVER_I2C_STATUS_ERROR_TIMEOUT;
                        i2cTransferState[0] = DRIVER_I2C_TXN_STATE_EXIT;
                    }
                    break;

                default:
#if LOG_LEVEL >= LOG_LEVEL_ERROR
                    DRIVER_I2C_LOGD(TAG, "Slave NACK");
#endif /* @end LOG_LEVEL>=LOG_LEVEL_DEBUG */
                    exeStatus = DRIVER_I2C_STATUS_ERROR_INVALID_STATE;
                    i2cTransferState[0] = DRIVER_I2C_TXN_STATE_EXIT;
                    break;
            }

            returnStatus = DRIVER_I2C_STATUS_BUSY;
            break;

        case DRIVER_I2C_TXN_STATE_WRITE_REG_ADDR:
            exeStatus = pI2CComPortDef->portWriteMasterFn(pI2CContext, slaveAddr, isSlaveAddr10Bit,
                                                          regAddrBuff, &regAddrSize);
            switch (exeStatus) {
                case DRIVER_I2C_STATUS_OK:
                    switch (pTxnData->txnType) {
                        case DRIVER_I2C_TXN_TYPE_MEM_WRITE:
                            i2cTransferState[0] = DRIVER_I2C_TXN_STATE_WRITE_DATA_BYTES;
                            break;
                        case DRIVER_I2C_TXN_TYPE_MEM_READ:
                            i2cTransferState[0] = DRIVER_I2C_TXN_STATE_READ_DATA_BYTES;
                            break;

                        default:
                            break;
                    }
                    returnStatus = DRIVER_I2C_STATUS_BUSY;
                    break;
                case DRIVER_I2C_STATUS_BUSY:
                    i2cTransferState[0] = DRIVER_I2C_TXN_STATE_WAIT;
                    i2cTransferState[1] = DRIVER_I2C_TXN_STATE_WRITE_REG_ADDR;
                    break;

                default:
#if LOG_LEVEL >= LOG_LEVEL_ERROR
                    DRIVER_I2C_LOGD(TAG, "Slave NACK");
#endif /* @end LOG_LEVEL>=LOG_LEVEL_DEBUG */
                    i2cTransferState[0] = DRIVER_I2C_TXN_STATE_EXIT;
                    break;
            }

            returnStatus = DRIVER_I2C_STATUS_BUSY;
            break;

        case DRIVER_I2C_TXN_STATE_WRITE_DATA_BYTES:
            exeStatus = pI2CComPortDef->portWriteMasterFn(pI2CContext, slaveAddr, isSlaveAddr10Bit,
                                                          dataBuff, &dataBuffSize);
            switch (exeStatus) {
                case DRIVER_I2C_STATUS_OK:
                    i2cTransferState[0] = DRIVER_I2C_TXN_STATE_EXIT;
                    break;
                case DRIVER_I2C_STATUS_BUSY:
                    i2cTransferState[0] = DRIVER_I2C_TXN_STATE_WAIT;
                    i2cTransferState[1] = DRIVER_I2C_TXN_STATE_EXIT;
                    break;

                default:
#if LOG_LEVEL >= LOG_LEVEL_ERROR
                    DRIVER_I2C_LOGD(TAG, "Slave NACK");
#endif /* @end LOG_LEVEL>=LOG_LEVEL_DEBUG */
                    i2cTransferState[0] = DRIVER_I2C_TXN_STATE_EXIT;
                    break;
            }

            returnStatus = DRIVER_I2C_STATUS_BUSY;
            break;

        case DRIVER_I2C_TXN_STATE_READ_DATA_BYTES:
            exeStatus = pI2CComPortDef->portReadMasterFn(pI2CContext, slaveAddr, isSlaveAddr10Bit,
                                                         dataBuff, &dataBuffSize);
            switch (exeStatus) {
                case DRIVER_I2C_STATUS_OK: {
                    i2cTransferState[0] = DRIVER_I2C_TXN_STATE_EXIT;
                    memcpy(pTxnData->pDataBuff, dataBuff, dataBuffSize);
                    break;
                }
                case DRIVER_I2C_STATUS_BUSY: {
                    i2cTransferState[0] = DRIVER_I2C_TXN_STATE_WAIT;
                    i2cTransferState[1] = DRIVER_I2C_TXN_STATE_EXIT;
                    break;
                }

                default: {
#if LOG_LEVEL >= LOG_LEVEL_DEBUG
                    DRIVER_I2C_LOGD(TAG, "Slave NACK");
#endif /* @end LOG_LEVEL>=LOG_LEVEL_DEBUG */
                    i2cTransferState[0] = DRIVER_I2C_TXN_STATE_EXIT;
                    break;
                }
            }

            returnStatus = DRIVER_I2C_STATUS_BUSY;
            break;

        case DRIVER_I2C_TXN_STATE_EXIT:
            i2cTransferState[0] = DRIVER_I2C_TXN_STATE_IDLE;
            returnStatus = exeStatus;
            break;
    }

    return returnStatus;
}
