/**
 * @file      pcf8591x.h
 * @author:   Shubhendu B B
 * @date:     25/12/2025
 * @brief
 * @details
 * NOTE:      This driver is currently single-threaded. RTOS synchronization (mutex/semaphore) is
 *            intentionally deferred. API boundaries are designed to be lock insertion points.
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
    PCF8591X_STATUS_ERROR_INVALID_DEVICE_ID = -4,
    PCF8591X_STATUS_ERROR_INVALID_CHANNEL = -5,
    PCF8591X_STATUS_ERROR_CHANNEL_NOT_CONFIGURED = -6,
} pcf8591x_Status_t;

typedef enum pcf8591x_tagChannelId {
    PCF8591X_CHANNEL_ID_AIN0 = 0,
    PCF8591X_CHANNEL_ID_AIN1,
    PCF8591X_CHANNEL_ID_AIN2,
    PCF8591X_CHANNEL_ID_AIN3,
    PCF8591X_CHANNEL_ID_AOUT,
    PCF8591X_CHANNEL_ID_MAX
} pcf8591x_ChannelId_t;

typedef enum pcf8591x_tagChannelType {
    PCF8591X_CHANNEL_TYPE_SINGLE_ENDED,
    PCF8591X_CHANNEL_TYPE_DIFFERENTIAL,
    PCF8591X_CHANNEL_TYPE_MAX
} pcf8591x_ChannelType_t;

typedef enum pcf8591x_tagMode {
    PCF8591X_MODE_NONE,  //
    PCF8591X_MODE_INCREMENTAL
} pcf8591x_Mode_t;

/* Platform I2C callbacks */
typedef int (*pcf8591x_I2CWriteBytes_t)(void *pvCtx, uint8_t slaveAddr, uint8_t *pDataBuff,
                                        uint16_t buffSize);

typedef int (*pcf8591x_I2CReadBytes_t)(void *pvCtx, uint8_t slaveAddr, uint8_t *pDataBuff,
                                       uint16_t buffSize);

typedef struct pcf8591x_tagInstanceConfig {
    struct {
        uint8_t a0State : 1;
        uint8_t a1State : 1;
        uint8_t a2State : 1;
    } addrSelect;

    void *pvCtx;  // Intended for platform specific context(hardware handle etc.)

    pcf8591x_I2CWriteBytes_t portI2CWriteFn;
    pcf8591x_I2CReadBytes_t portI2CReadFn;
    pcf8591x_Mode_t mode;
} pcf8591x_InstanceConfig_t;

typedef struct pcf8591x_tagChannelConfig {
    pcf8591x_ChannelId_t channelId;
    pcf8591x_ChannelType_t channelType;
} pcf8591x_ChannelConfig_t;

#ifdef __cplusplus
extern "C" {
#endif

pcf8591x_Status_t pcf8591x_init(uint8_t deviceId, pcf8591x_InstanceConfig_t *pCommConfig);

pcf8591x_Status_t pcf8591x_configure_channel(uint8_t deviceId,
                                             pcf8591x_ChannelConfig_t *pChannelConfig);
pcf8591x_Status_t pcf8591x_write_channel(uint8_t deviceId, pcf8591x_ChannelId_t channelId);

#ifdef __cplusplus
}
#endif

#endif /* __PCF8591X_H__ */
