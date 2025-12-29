/**
 * @file      app_i2c.c
 * @author:   Shubhendu B B
 * @date:     27/12/2025
 * @brief
 * @details
 *
 * @copyright
 *
 **/
#include "app_i2c.h"

#include "driver_i2c.h"
#include "port_i2c.h"

static driver_i2c_comm_port_t gI2CCommPort;

app_i2c_Status_t app_i2c_init(void) {
    gI2CCommPort.portDeinitFn = port_i2c_init;
    gI2CCommPort.portDeinitFn = port_i2c_deinit;

    driver_i2c_init(&gI2CCommPort);
}
