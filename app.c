/**
 * @file      app.c
 * @author:   Shubhendu B B
 * @date:     25/12/2025
 * @brief
 * @details
 *
 * @copyright
 *
 **/

/*!< An example of how to use the driver. */

#include "pcf8591x.h"

//
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>

static int app_i2c_write_bytes(void *pvCtx, uint8_t slaveAddr, uint8_t *pDataBuff,
                               uint16_t buffSize);
static int app_i2c_read_bytes(void *pvCtx, uint8_t slaveAddr, uint8_t *pDataBuff,
                              uint16_t buffSize);

int main(int argc, char *argv[]) {
    pcf8591x_InstanceConfig_t instanceConfig = {.addrSelect.a0State = true,
                                                .addrSelect.a1State = true,
                                                .addrSelect.a2State = true,
                                                .portI2CWriteFn = app_i2c_write_bytes,
                                                .portI2CReadFn = app_i2c_read_bytes};
}

static int app_i2c_write_bytes(void *pvCtx, uint8_t slaveAddr, uint8_t *pDataBuff,
                               uint16_t buffSize) {
    if (pvCtx) {
        printf("CTX: %p\r\n", (void *)pvCtx);
    }

    printf("Slave address: 0x%02X\r\n", slaveAddr);

    if (!pDataBuff || !buffSize) {
        return -1;
    }

    printf("Data:\r\n");

    for (uint16_t i = 0; i < buffSize; i++) {
        printf("0x%02X ", pDataBuff[i]);
    }

    printf("\r\n");

    return 0;
}

static int app_i2c_read_bytes(void *pvCtx, uint8_t slaveAddr, uint8_t *pDataBuff,
                              uint16_t buffSize) {
    if (pvCtx) {
        printf("CTX: %p\r\n", (void *)pvCtx);
    }

    printf("Slave address: 0x%02X\r\n", slaveAddr);

    if (!pDataBuff || !buffSize) {
        return -1;
    }

    /* Fill dummy data: simple incrementing pattern */
    for (uint16_t i = 0; i < buffSize; i++) {
        pDataBuff[i] = (uint8_t)(0xA0 + i);
    }

    /* Optional: dump read data for verification */
    printf("Read Data (%u bytes):\r\n", buffSize);
    for (uint16_t i = 0; i < buffSize; i++) {
        printf("0x%02X ", pDataBuff[i]);
    }
    printf("\r\n");

    return 0;
}
