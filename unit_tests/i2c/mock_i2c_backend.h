#ifndef MOCK_I2C_BACKEND_H
#define MOCK_I2C_BACKEND_H

#include <stdbool.h>
#include <stdint.h>

#include "driver_i2c.h"

#define MOCK_I2C_MAX_DEVICES 8
#define MOCK_I2C_MEM_SIZE 256

typedef struct {
    uint16_t slaveAddr;
    uint8_t memory[MOCK_I2C_MEM_SIZE];
    bool present;
} mock_i2c_device_t;

/* Registration API */
void mock_i2c_register_device(uint16_t slaveAddr);
void mock_i2c_reset(void);

/* Port functions (plug into driver_i2c_comm_port_t) */
driver_i2c_Status_t mock_i2c_init(void* pvPort);
driver_i2c_Status_t mock_i2c_deinit(void* pvPort);
driver_i2c_Status_t mock_i2c_write(void* ctx, uint16_t slaveAddr, bool is10BitAddr, uint8_t* pData,
                                   uint8_t* pLen);
driver_i2c_Status_t mock_i2c_read(void* ctx, uint16_t slaveAddr, bool is10BitAddr, uint8_t* pData,
                                  uint8_t* pLen);
uint64_t mock_i2c_get_timestamp(void);
bool mock_i2c_is_timeout(uint64_t start, uint64_t timeout);

#endif
