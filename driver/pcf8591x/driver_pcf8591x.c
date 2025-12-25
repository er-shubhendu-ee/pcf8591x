/**
 * @file      driver_pcf8591x.c
 * @author:   Shubhendu B B
 * @date:     25/12/2025
 * @brief
 * @details
 *
 * @copyright
 *
 **/

#include "driver_pcf8591x.h"

//
#include <stdbool.h>

/*!
 * @def         DRIVER_PCF8591X_AT_INDEX
 * @brief       //  Currently supports single chip on an I2C bus. Multi-instance support to be added
 *              later on
 * @details
 *
 **/
#define DRIVER_PCF8591X_MULTIINDEX_SUPPORTED 0

#ifndef LOG_LEVEL
#define LOG_LEVEL 3
#endif

#define TAG "DRIVER_PCF8591X"

#define DRIVER_PCF8591X_CTRL_BIT_AOE (1u << 6)
#define DRIVER_PCF8591X_CTRL_BIT_AI (1u << 5)

#define DRIVER_PCF8591X_CTRL_CH_SHIFT 2u
#define DRIVER_PCF8591X_CTRL_CH_MASK (0x07u << DRIVER_PCF8591X_CTRL_CH_SHIFT)

typedef struct driver_pcf8591x_tagChannelState {
    driver_pcf8591x_ChannelType_t type;
    struct {
        uint8_t isConfigured : 1;
    };
} driver_pcf8591x_ChannelState_t;

/* PCF8591 I2C address format:
 * 7-bit: 1001 A2 A1 A0
 * fixedBits = 0b1001 = 0x09
 */
typedef struct driver_pcf8591x_tagInstanceStruct {
    driver_pcf8591x_Mode_t mode;
    void *pvCtx;
    uint8_t addrByte;  //
    driver_pcf8591x_I2CWriteBytes_t I2CWriteFn;
    driver_pcf8591x_I2CReadBytes_t I2CReadFn;
    driver_pcf8591x_ChannelState_t channelStateArr[DRIVER_PCF8591X_CHANNEL_COUNT_MAX];
} driver_pcf8591x_InstanceStruct_t;

static driver_pcf8591x_InstanceStruct_t gInstanceArr[DRIVER_PCF8591X_INSTANCE_COUNT_MAX];

static bool is_channel_valid(driver_pcf8591x_ChannelId_t channelId);
static uint8_t encode_channel_to_ctrl(driver_pcf8591x_ChannelId_t channelId);
/* Builds control byte for a specific channel of an instance */
static uint8_t build_control_byte(uint8_t deviceId, driver_pcf8591x_ChannelId_t channelId);
static int configure_channel(driver_pcf8591x_ChannelId_t channelId,
                             driver_pcf8591x_ChannelConfig_t *pChannelConfiguration);
static int write_channel(driver_pcf8591x_ChannelId_t channelId, uint8_t *pDataByteBuff);
static int read_channel(driver_pcf8591x_ChannelId_t channelId, uint8_t *pDataByteBuff);

driver_pcf8591x_Status_t driver_pcf8591x_init(uint8_t deviceId,
                                              driver_pcf8591x_InstanceConfig_t *pConfig) {
    if (!pConfig) {                                    //!< NULL pointer
        return DRIVER_PCF8591X_STATUS_ERROR_NULL_ARG;  //!< Return error
    }

    if (!pConfig->portI2CWriteFn || !pConfig->portI2CReadFn) {
        return DRIVER_PCF8591X_STATUS_ERROR_BAD_ARG;
    }

    /*!< Multi-index not supported */
#if (0 == DRIVER_PCF8591X_MULTIINDEX_SUPPORTED)

    if (deviceId) {
        return DRIVER_PCF8591X_STATUS_ERROR_BAD_ARG;
    }
#else

    if (DRIVER_PCF8591X_INSTANCE_COUNT_MAX <= deviceId) {
        return DRIVER_PCF8591X_STATUS_ERROR_BAD_ARG;
    }
#endif

    gInstanceArr[deviceId].addrByte = DRIVER_PCF8591X_ADDR_FIXED_BITS << 3U;
    gInstanceArr[deviceId].addrByte |= (0x01 & (uint8_t)pConfig->addrSelect.a0State);
    gInstanceArr[deviceId].addrByte |= ((0x01 & (uint8_t)pConfig->addrSelect.a1State) << 1);
    gInstanceArr[deviceId].addrByte |= ((0x01 & (uint8_t)pConfig->addrSelect.a2State) << 2);
    gInstanceArr[deviceId].mode = pConfig->mode;
    gInstanceArr[deviceId].pvCtx = pConfig->pvCtx;
    gInstanceArr[deviceId].I2CWriteFn = pConfig->portI2CWriteFn;
    gInstanceArr[deviceId].I2CReadFn = pConfig->portI2CReadFn;

    return DRIVER_PCF8591X_STATUS_OK;  //!< Return no error
}

driver_pcf8591x_Status_t driver_pcf8591x_configure_channel(
    uint8_t deviceId, driver_pcf8591x_ChannelConfig_t *pChannelConfig) {
    if (NULL == pChannelConfig) {
        return DRIVER_PCF8591X_STATUS_ERROR_NULL_ARG;
    }

#if (0 == DRIVER_PCF8591X_MULTIINDEX_SUPPORTED)
    if (deviceId) {
        return DRIVER_PCF8591X_STATUS_ERROR_BAD_ARG;
    }
#else
    if (DRIVER_PCF8591X_INSTANCE_COUNT_MAX <= deviceId) {
        return DRIVER_PCF8591X_STATUS_ERROR_BAD_ARG;
    }
#endif

    if (!is_channel_valid(pChannelConfig->channelId)) {
        return DRIVER_PCF8591X_STATUS_ERROR_INVALID_CHANNEL;
    }

    if ((DRIVER_PCF8591X_CHANNEL_TYPE_SINGLE_ENDED != pChannelConfig->channelType) &&
        (DRIVER_PCF8591X_CHANNEL_TYPE_DIFFERENTIAL != pChannelConfig->channelType)) {
        return DRIVER_PCF8591X_STATUS_ERROR_BAD_ARG;
    }

    /* DAC channel cannot be differential */
    if ((DRIVER_PCF8591X_CHANNEL_ID_AOUT == pChannelConfig->channelId) &&
        (DRIVER_PCF8591X_CHANNEL_TYPE_SINGLE_ENDED != pChannelConfig->channelType)) {
        return DRIVER_PCF8591X_STATUS_ERROR_BAD_ARG;
    }

    gInstanceArr[deviceId].channelStateArr[pChannelConfig->channelId].type =
        pChannelConfig->channelType;
    gInstanceArr[deviceId].channelStateArr[pChannelConfig->channelId].isConfigured = true;

    return DRIVER_PCF8591X_STATUS_OK;
}

driver_pcf8591x_Status_t driver_pcf8591x_write_channel(uint8_t deviceId,
                                                       driver_pcf8591x_ChannelId_t channelId) {
#if (0 == DRIVER_PCF8591X_MULTIINDEX_SUPPORTED)
    if (deviceId != 0) {
        return DRIVER_PCF8591X_STATUS_ERROR_INVALID_DEVICE_ID;
    }
#else
    if (deviceId >= DRIVER_PCF8591X_INSTANCE_COUNT_MAX) {
        return DRIVER_PCF8591X_STATUS_ERROR_INVALID_DEVICE_ID;
    }
#endif

    if (!is_channel_valid(channelId)) {
        return DRIVER_PCF8591X_STATUS_ERROR_INVALID_CHANNEL;
    }

    driver_pcf8591x_InstanceStruct_t *pInst = &gInstanceArr[deviceId];
    driver_pcf8591x_ChannelState_t *pChState = &pInst->channelStateArr[channelId];

    /* Channel must be configured */
    if (!pChState->isConfigured) {
        return DRIVER_PCF8591X_STATUS_ERROR_CHANNEL_NOT_CONFIGURED;
    }

    return DRIVER_PCF8591X_STATUS_OK;
}

static inline bool is_channel_valid(driver_pcf8591x_ChannelId_t channelId) {
    switch (channelId) {
        case DRIVER_PCF8591X_CHANNEL_ID_AIN0:
        case DRIVER_PCF8591X_CHANNEL_ID_AIN1:
        case DRIVER_PCF8591X_CHANNEL_ID_AIN2:
        case DRIVER_PCF8591X_CHANNEL_ID_AIN3:
        case DRIVER_PCF8591X_CHANNEL_ID_AOUT: {
            return true;
            break;
        }

        default: {
            return false;
            break;
        }
    }
}

static uint8_t encode_channel_to_ctrl(driver_pcf8591x_ChannelId_t channelId) {
    uint8_t chSel = 0u;

    /*
     * CH[2:0] encoding as per PCF8591 datasheet
     * Explicit mapping — enum order is irrelevant
     *
     * CH2 CH1 CH0
     *  0   0   0   → AIN0
     *  0   0   1   → AIN1
     *  0   1   0   → AIN2
     *  0   1   1   → AIN3
     * (AOUT uses CH=000, DAC enabled via AOE)
     */
    switch (channelId) {
        case DRIVER_PCF8591X_CHANNEL_ID_AIN0:
            chSel = 0u;
            break;

        case DRIVER_PCF8591X_CHANNEL_ID_AIN1:
            chSel = 1u;
            break;

        case DRIVER_PCF8591X_CHANNEL_ID_AIN2:
            chSel = 2u;
            break;

        case DRIVER_PCF8591X_CHANNEL_ID_AIN3:
            chSel = 3u;
            break;

        case DRIVER_PCF8591X_CHANNEL_ID_AOUT:
            chSel = 0u; /* DAC shares channel 0 */
            break;

        default:
            /* Defensive fallback — should never occur */
            chSel = 0u;
            break;
    }

    return (uint8_t)((chSel << DRIVER_PCF8591X_CTRL_CH_SHIFT) & DRIVER_PCF8591X_CTRL_CH_MASK);
}

/* Builds control byte for a specific channel of an instance */
static uint8_t build_control_byte(uint8_t deviceId, driver_pcf8591x_ChannelId_t channelId) {
    uint8_t ctrl = 0u;
    driver_pcf8591x_InstanceStruct_t *pInst = &gInstanceArr[deviceId];
    driver_pcf8591x_ChannelState_t *pChState = &pInst->channelStateArr[channelId];

    /* --------------------------------------------------------------------
     * AOE bit (bit 6)
     * Enabled ONLY for DAC output channel
     * ------------------------------------------------------------------ */
    switch (channelId) {
        case DRIVER_PCF8591X_CHANNEL_ID_AOUT:
            ctrl |= DRIVER_PCF8591X_CTRL_BIT_AOE;
            break;

        case DRIVER_PCF8591X_CHANNEL_ID_AIN0:
        case DRIVER_PCF8591X_CHANNEL_ID_AIN1:
        case DRIVER_PCF8591X_CHANNEL_ID_AIN2:
        case DRIVER_PCF8591X_CHANNEL_ID_AIN3:
            /* ADC channels → AOE = 0 */
            break;

        default:
            /* Defensive: should never happen */
            break;
    }

    /* --------------------------------------------------------------------
     * AI bit (bit 5)
     * Instance-wide auto-increment mode
     * ------------------------------------------------------------------ */
    switch (pInst->mode) {
        case DRIVER_PCF8591X_MODE_INCREMENTAL:
            ctrl |= DRIVER_PCF8591X_CTRL_BIT_AI;
            break;

        case DRIVER_PCF8591X_MODE_NONE:
        default:
            /* AI disabled */
            break;
    }

    if (pChState->type == DRIVER_PCF8591X_CHANNEL_TYPE_DIFFERENTIAL) {
        /* not supported yet */
        /* either assert or return error via caller */
    }

    /* --------------------------------------------------------------------
     * NOTE:
     * Channel type (single-ended / differential) will affect CH mapping
     * when differential mode support is added.
     * CH [2:0] bits(bits 4..2) * Explicit channel encoding(NO enum arithmetic)
     * Channel TYPE may influence mapping(future - safe)
     * -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --*/
    ctrl |= encode_channel_to_ctrl(channelId);

    /* --------------------------------------------------------------------
     * Fixed bits [1:0] remain 0 as per datasheet
     * ------------------------------------------------------------------ */

    return ctrl;
}

static int configure_channel(driver_pcf8591x_ChannelId_t channelId,
                             driver_pcf8591x_ChannelConfig_t *pChannelConfiguration) {
    if (!pChannelConfiguration) {
        return DRIVER_PCF8591X_STATUS_ERROR_NULL_ARG;
    }

    return DRIVER_PCF8591X_STATUS_OK;
}

static int write_channel(driver_pcf8591x_ChannelId_t channelId, uint8_t *pDataByteBuff) {
    if (!pDataByteBuff) {
        return DRIVER_PCF8591X_STATUS_ERROR_NULL_ARG;
    }

    return DRIVER_PCF8591X_STATUS_OK;
}

static int read_channel(driver_pcf8591x_ChannelId_t channelId, uint8_t *pDataByteBuff) {
    if (!pDataByteBuff) {
        return DRIVER_PCF8591X_STATUS_ERROR_NULL_ARG;
    }

    return DRIVER_PCF8591X_STATUS_OK;
}
