/**
 * @file      port_i2c.c
 * @author:   Shubhendu B B
 * @date:     27/12/2025
 * @brief
 * @details
 *
 * @copyright
 *
 **/
#include "port_i2c.h"

port_i2c_Status_t port_i2c_init(void* pvInstanceConfig) {
    if (!pvInstanceConfig) {
        return PORT_I2C_STATUS_ERROR_NULL_ARG;
    }

    port_i2c_Status_t exeStatus = PORT_I2C_STATUS_OK;
    port_i2c_InstanceConfig_t* pInstanceConfig = (port_i2c_InstanceConfig_t*)pvInstanceConfig;

    return exeStatus;
}

port_i2c_Status_t port_i2c_deinit(void* pvInstanceConfig) {
    if (!pvInstanceConfig) {
        return PORT_I2C_STATUS_ERROR_NULL_ARG;
    }

    port_i2c_Status_t exeStatus = PORT_I2C_STATUS_OK;
    port_i2c_InstanceConfig_t* pInstanceConfig = (port_i2c_InstanceConfig_t*)pvInstanceConfig;

    return exeStatus;
}