#include "mock_i2c_backend.h"

//
#include <stdio.h>
#include <string.h>
#include <time.h>

static mock_i2c_device_t devices[MOCK_I2C_MAX_DEVICES];

void mock_i2c_reset(void) { memset(devices, 0, sizeof(devices)); }

void mock_i2c_register_device(uint16_t slaveAddr) {
    for (int i = 0; i < MOCK_I2C_MAX_DEVICES; i++) {
        if (!devices[i].present) {
            devices[i].present = true;
            devices[i].slaveAddr = slaveAddr;
            memset(devices[i].memory, 0xFF, MOCK_I2C_MEM_SIZE);
            return;
        }
    }
}

static mock_i2c_device_t* find_device(uint16_t addr) {
    for (int i = 0; i < MOCK_I2C_MAX_DEVICES; i++) {
        if (devices[i].present && devices[i].slaveAddr == addr)
            return &devices[i];
    }
    return NULL;
}

/* ---- Port API ---- */

driver_i2c_Status_t mock_i2c_init(void* pvPort) {
    (void)pvPort;
    return DRIVER_I2C_STATUS_OK;
}

driver_i2c_Status_t mock_i2c_deinit(void* pvPort) {
    (void)pvPort;
    return DRIVER_I2C_STATUS_OK;
}

driver_i2c_Status_t mock_i2c_write(void* ctx, uint16_t slaveAddr, bool is10BitAddr, uint8_t* pData,
                                   uint8_t* pLen) {
    (void)ctx;
    (void)is10BitAddr;

    mock_i2c_device_t* dev = find_device(slaveAddr);
    if (!dev)
        return DRIVER_I2C_STATUS_ERROR_NACK;

    if (*pLen > MOCK_I2C_MEM_SIZE)
        return DRIVER_I2C_STATUS_ERROR_MAX_PDU;

    memcpy(dev->memory, pData, *pLen);

    printf("Data sent: %d bytes\r\n", *pLen);
    for (int indexI = 0; indexI < *pLen; indexI++) {
        printf("0x%.2X\r\n", dev->memory[indexI]);
    }
    printf("\r\n");

    return DRIVER_I2C_STATUS_OK;
}

driver_i2c_Status_t mock_i2c_read(void* ctx, uint16_t slaveAddr, bool is10BitAddr, uint8_t* pData,
                                  uint8_t* pLen) {
    (void)ctx;
    (void)is10BitAddr;

    mock_i2c_device_t* dev = find_device(slaveAddr);
    if (!dev)
        return DRIVER_I2C_STATUS_ERROR_NACK;

    memcpy(pData, dev->memory, *pLen);

    printf("Data received: %d bytes\r\n", *pLen);
    for (int indexI = 0; indexI < *pLen; indexI++) {
        printf("0x%.2X\r\n", pData[indexI]);
    }
    printf("\r\n");

    return DRIVER_I2C_STATUS_OK;
}

uint64_t mock_i2c_get_timestamp(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

bool mock_i2c_is_timeout(uint64_t start, uint64_t timeout) {
    (void)start;
    (void)timeout;
    return false; /* deterministic: no timeout */
}
