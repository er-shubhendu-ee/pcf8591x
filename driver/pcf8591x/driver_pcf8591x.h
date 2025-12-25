/**
 * @file      driver_pcf8591x.h
 * @author:   Shubhendu B B
 * @date:     25/12/2025
 * @brief
 * @details
 * NOTE:      This driver is currently single-threaded. RTOS synchronization (mutex/semaphore) is
 *            intentionally deferred. API boundaries are designed to be lock insertion points.
 * @copyright
 *
 **/
#ifndef _DRIVER_PCF8591X_H_
#define _DRIVER_PCF8591X_H_

#include <stdint.h>

#define DRIVER_PCF8591X_INSTANCE_COUNT_MAX 8
#define DRIVER_PCF8591X_CHANNEL_COUNT_MAX 8
#define DRIVER_PCF8591X_ADDR_FIXED_BITS 0x09

typedef enum driver_pcf8591x_tagStatus {
    DRIVER_PCF8591X_STATUS_OK = 0,
    DRIVER_PCF8591X_STATUS_BUSY = 1,

    DRIVER_PCF8591X_STATUS_ERROR = -1,
    DRIVER_PCF8591X_STATUS_ERROR_NULL_ARG = -2,
    DRIVER_PCF8591X_STATUS_ERROR_BAD_ARG = -3,
    DRIVER_PCF8591X_STATUS_ERROR_INVALID_DEVICE_ID = -4,
    DRIVER_PCF8591X_STATUS_ERROR_INVALID_CHANNEL = -5,
    DRIVER_PCF8591X_STATUS_ERROR_CHANNEL_NOT_CONFIGURED = -6,
} driver_pcf8591x_Status_t;

typedef enum driver_pcf8591x_tagChannelId {
    DRIVER_PCF8591X_CHANNEL_ID_AIN0 = 0,
    DRIVER_PCF8591X_CHANNEL_ID_AIN1,
    DRIVER_PCF8591X_CHANNEL_ID_AIN2,
    DRIVER_PCF8591X_CHANNEL_ID_AIN3,
    DRIVER_PCF8591X_CHANNEL_ID_AOUT,
    DRIVER_PCF8591X_CHANNEL_ID_MAX
} driver_pcf8591x_ChannelId_t;

typedef enum driver_pcf8591x_tagChannelType {
    DRIVER_PCF8591X_CHANNEL_TYPE_SINGLE_ENDED,
    DRIVER_PCF8591X_CHANNEL_TYPE_DIFFERENTIAL,
    DRIVER_PCF8591X_CHANNEL_TYPE_MAX
} driver_pcf8591x_ChannelType_t;

typedef enum driver_pcf8591x_tagMode {
    DRIVER_PCF8591X_MODE_NONE,  //
    DRIVER_PCF8591X_MODE_INCREMENTAL
} driver_pcf8591x_Mode_t;

/* Platform I2C callbacks */
typedef int (*driver_pcf8591x_I2CWriteBytes_t)(void *pvCtx, uint8_t slaveAddr, uint8_t *pDataBuff,
                                               uint16_t buffSize);

typedef int (*driver_pcf8591x_I2CReadBytes_t)(void *pvCtx, uint8_t slaveAddr, uint8_t *pDataBuff,
                                              uint16_t buffSize);

typedef struct driver_pcf8591x_tagInstanceConfig {
    struct {
        uint8_t a0State : 1;
        uint8_t a1State : 1;
        uint8_t a2State : 1;
    } addrSelect;

    void *pvCtx;  // Intended for platform specific context(hardware handle etc.)

    driver_pcf8591x_I2CWriteBytes_t portI2CWriteFn;
    driver_pcf8591x_I2CReadBytes_t portI2CReadFn;
    driver_pcf8591x_Mode_t mode;
} driver_pcf8591x_InstanceConfig_t;

typedef struct driver_pcf8591x_tagChannelConfig {
    driver_pcf8591x_ChannelId_t channelId;
    driver_pcf8591x_ChannelType_t channelType;
} driver_pcf8591x_ChannelConfig_t;

#ifdef __cplusplus
extern "C" {
#endif

driver_pcf8591x_Status_t driver_pcf8591x_init(uint8_t deviceId,
                                              driver_pcf8591x_InstanceConfig_t *pCommConfig);

driver_pcf8591x_Status_t driver_pcf8591x_configure_channel(
    uint8_t deviceId, driver_pcf8591x_ChannelConfig_t *pChannelConfig);
driver_pcf8591x_Status_t driver_pcf8591x_write_channel(uint8_t deviceId,
                                                       driver_pcf8591x_ChannelId_t channelId);

#ifdef __cplusplus
}
#endif

#endif /* _DRIVER_PCF8591X_H_ */
