/**
 * @file      app_i2c.h
 * @author:   Shubhendu B B
 * @date:     27/12/2025
 * @brief
 * @details
 *
 * @copyright
 *
 **/
#ifndef _APP_I2C_H_
#define _APP_I2C_H_

#include <stdint.h>

typedef enum app_i2c_tagStatus_t {
    APP_I2C_STATUS_OK = 0,  //
    APP_I2C_STATUS_BUSY = 1,

    APP_I2C_STATUS_ERROR = -1,
} app_i2c_Status_t;

#ifdef __cplusplus
extern "C" {
#endif

app_i2c_Status_t app_i2c_init(void);

#ifdef __cplusplus
}
#endif

#endif /* @end  _APP_I2C_H_*/