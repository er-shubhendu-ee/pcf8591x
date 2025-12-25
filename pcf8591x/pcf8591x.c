/**
 * @file      pcf8591x.c
 * @author:   Shubhendu B B
 * @date:     25/12/2025
 * @brief
 * @details
 *
 * @copyright
 *
 **/

#include "pcf8591x.h"

#define PCF8591X_MAX_COUNT 8

/*!
 * @def         PCF8591X_AT_INDEX
 * @brief       //  Currently supports single chip on an I2C bus. Multi-instance support to be added
 *              later on
 * @details
 *
 **/
#define PCF8591X_MULTIINDEX_SUPPORTED 0

#ifndef LOG_LEVEL
#define LOG_LEVEL 3
#endif

#define TAG "PCF8591X"

typedef struct pcf8591x_tagInstanceStruct {
    union {
        uint8_t addrByte;  //
        struct {
            uint8_t a0State : 1;
            uint8_t a1State : 1;
            uint8_t a2State : 1;
            uint8_t fixedBits : 5
        } addr;
    };
    pcf8591x_I2CWriteBytes_t I2CWriteFn;
    pcf8591x_I2CWriteBytes_t I2CReadFn;
} pcf8591x_InstanceStruct_t;

static pcf8591x_InstanceStruct_t gInstanceArr[PCF8591X_MAX_COUNT];

int pcf8591x_init(uint8_t deviceId, pcf8591x_Config_t *pConfig) {
    if (!pConfig) {                             //!< NULL pointer
        return PCF8591X_STATUS_ERROR_NULL_ARG;  //!< Return error
    }

    /*!< Multi-index not supported */
    if (0 == PCF8591X_MULTIINDEX_SUPPORTED) {
        if (!deviceId) {
            return PCF8591X_STATUS_ERROR_BAD_ARG;
        }

        gInstanceArr[0].addr.a0State = pConfig->addrSelect.a0State;
        gInstanceArr[0].addr.a1State = pConfig->addrSelect.a1State;
        gInstanceArr[0].addr.a2State = pConfig->addrSelect.a2State;
        gInstanceArr[0].I2CWriteFn = pConfig->portI2CWriteFn;
        gInstanceArr[0].I2CReadFn = pConfig->portI2CReadFn;
    }

    gInstanceArr[0].addr.a0State = pConfig->addrSelect.a0State;
    gInstanceArr[0].addr.a1State = pConfig->addrSelect.a1State;
    gInstanceArr[0].addr.a2State = pConfig->addrSelect.a2State;
    gInstanceArr[0].I2CWriteFn = pConfig->portI2CWriteFn;
    gInstanceArr[0].I2CReadFn = pConfig->portI2CReadFn;

    return PCF8591X_STATUS_OK;  //!< Return no error
}