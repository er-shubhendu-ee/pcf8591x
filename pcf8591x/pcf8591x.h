/**
 * @file      pcf8591x.h
 * @author:   Shubhendu B B
 * @date:     25/12/2025
 * @brief
 * @details
 *
 * @copyright
 *
 **/

#ifndef __PCF8591X_H__
#define __PCF8591X_H__

#include <stdint.h>

typedef enum pcf8591x_tagStatus {
    PCF8591X_STATUS_OK = 0,
    PCF8591X_STATUS_BUSY = 1,

    PCF8591X_STATUS_ERROR = -1,
    PCF8591X_STATUS_ERROR_NULL_ARG = -2,
    PCF8591X_STATUS_ERROR_BAD_ARG = -3,
} pcf8591x_Status_t;

typedef int (*pcf8591x_I2CWriteBytes_t)(void *pCtx, uint8_t slaveAddr, uint8_t *pDataBuff,
                                        uint16_t buffSize);
typedef int (*pcf8591x_I2CReadBytes_t)(void *pCtx, uint8_t slaveAddr, uint8_t *pDataBuff,
                                       uint16_t buffSize);

typedef struct pcf8591x_tagConfig {
    struct {
        uint8_t a0State : 1;
        uint8_t a1State : 1;
        uint8_t a2State : 1;
    } addrSelect;
    pcf8591x_I2CWriteBytes_t portI2CWriteFn;
    pcf8591x_I2CWriteBytes_t portI2CReadFn;
} pcf8591x_Config_t;

#ifdef __cplusplus
extern "C" {
#endif

int pcf8591x_init(uint8_t deviceId, pcf8591x_Config_t *pCommConfig);

#ifdef __cplusplus
}
#endif

#endif /* @end  _PCF8591X_H_*/