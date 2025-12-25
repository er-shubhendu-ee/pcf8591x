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
#include "test_config.h"

//
#include "driver_i2c.h"
#include "mock_i2c_backend.h"

//
#include <assert.h>
#include <stdio.h>

static int test_i2c(void);

int main(void) {
#if (0 < TEST_CONFIG_I2C_TEST)
    test_i2c();
#endif
}

static int test_i2c(void) {
    /* ---------------- Mock setup ---------------- */
    mock_i2c_reset();
    mock_i2c_register_device(0x50);

    driver_i2c_comm_port_t port = {.pvCtx = NULL,
                                   .i2cInstId = 0,
                                   .portInitFn = mock_i2c_init,
                                   .portDeinitFn = mock_i2c_deinit,
                                   .portWriteMasterFn = mock_i2c_write,
                                   .portReadMasterFn = mock_i2c_read,
                                   .portTimeoutFn = mock_i2c_is_timeout};

    driver_i2c_Status_t st = driver_i2c_init(&port);
    assert(st == DRIVER_I2C_STATUS_OK);

    /* ---------------- WRITE transaction ---------------- */
    uint8_t txData[4] = {1, 2, 3, 4};

    driver_i2c_txnData_t writeTxn = {.slaveAddr = 0x50,
                                     .txnType = DRIVER_I2C_TXN_TYPE_WRITE,
                                     .pTxDataBuff = txData,
                                     .dataBuffSize = sizeof(txData),
                                     .timeout = 100};

    st = driver_i2c_transfer(&port, &writeTxn);
    assert(st == DRIVER_I2C_STATUS_OK);

    /* ---------------- READ transaction ---------------- */
    uint8_t rxData[4] = {0};

    driver_i2c_txnData_t readTxn = {.slaveAddr = 0x50,
                                    .txnType = DRIVER_I2C_TXN_TYPE_READ,
                                    .pRxDataBuff = rxData,
                                    .dataBuffSize = sizeof(rxData),
                                    .timeout = 100};

    st = driver_i2c_transfer(&port, &readTxn);
    assert(st == DRIVER_I2C_STATUS_OK);

    /* ---------------- Validate ---------------- */
    assert(memcmp(txData, rxData, sizeof(txData)) == 0);

    driver_i2c_txnData_t txn = {.slaveAddr = 0x77, /* not registered */
                                .txnType = DRIVER_I2C_TXN_TYPE_READ,
                                .dataBuffSize = 1,
                                .timeout = 50};

    driver_i2c_Status_t st = driver_i2c_transfer(&port, &txn);
    assert(st == DRIVER_I2C_STATUS_ERROR_NACK);

    printf("driver_i2c_transfer() basic READ/WRITE/NACK test PASSED\n");

    driver_i2c_deinit(&port);
    return 0;
}
