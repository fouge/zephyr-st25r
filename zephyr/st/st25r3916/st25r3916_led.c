/**
 ******************************************************************************
 * @file    st25r3916_led.c
 * @author  MMY Application Team
 * @brief   ST25R3916 LEDs handling
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

/*
******************************************************************************
* INCLUDES
******************************************************************************
*/

#include "st25r3916_led.h"
#include "st25r3916.h"
#include "st25r3916_com.h"
#include "st25r3916_irq.h"

LOG_MODULE_REGISTER(st25r3916_led, CONFIG_ST25R_LOG_LEVEL);

/*
******************************************************************************
* MACROS
******************************************************************************
*/

#ifdef PLATFORM_LED_RX_PIN
#define st25r3916ledRxOn()                                                     \
    platformLedOn(PLATFORM_LED_RX_PORT,                                        \
                  PLATFORM_LED_RX_PIN); /*!< LED Rx Pin On from system HAL */
#define st25r3916ledRxOff()                                                    \
    platformLedOff(                                                            \
        PLATFORM_LED_RX_PORT,                                                  \
        PLATFORM_LED_RX_PIN); /*!< LED Rx Pin Off from system HAL           */
#else                         /* PLATFORM_LED_RX_PIN */
#define st25r3916ledRxOn()
#define st25r3916ledRxOff()
#endif /* PLATFORM_LED_RX_PIN */

#ifdef PLATFORM_LED_FIELD_PIN
#define st25r3916ledFieldOn()                                                  \
    platformLedOn(                                                             \
        PLATFORM_LED_FIELD_PORT,                                               \
        PLATFORM_LED_FIELD_PIN); /*!< LED Field Pin On from system HAL */
#define st25r3916ledFieldOff()                                                 \
    platformLedOff(                                                            \
        PLATFORM_LED_FIELD_PORT,                                               \
        PLATFORM_LED_FIELD_PIN); /*!< LED Field Pin Off from system HAL */
#else                            /* PLATFORM_LED_FIELD_PIN */
#define st25r3916ledFieldOn()
#define st25r3916ledFieldOff()
#endif /* PLATFORM_LED_FIELD_PIN */

/*
******************************************************************************
* GLOBAL FUNCTIONS
******************************************************************************
*/

void
st25r3916ledInit(void)
{
    /* Initialize LEDs if existing and defined */
    platformLedsInitialize();

    st25r3916ledRxOff();
    st25r3916ledFieldOff();
}

/*******************************************************************************/
void
st25r3916ledEvtIrq(uint32_t irqs)
{
    if ((irqs & (ST25R3916_IRQ_MASK_TXE | ST25R3916_IRQ_MASK_CAT)) != 0U) {
        st25r3916ledFieldOn();
    }

    if ((irqs & (ST25R3916_IRQ_MASK_RXS | ST25R3916_IRQ_MASK_NFCT)) != 0U) {
        st25r3916ledRxOn();
    }

    if ((irqs & (ST25R3916_IRQ_MASK_RXE | ST25R3916_IRQ_MASK_NRE |
                 ST25R3916_IRQ_MASK_RX_REST | ST25R3916_IRQ_MASK_RXE_PTA |
                 ST25R3916_IRQ_MASK_WU_A | ST25R3916_IRQ_MASK_WU_A_X |
                 ST25R3916_IRQ_MASK_WU_F | ST25R3916_IRQ_MASK_RFU2)) != 0U) {
        st25r3916ledRxOff();
    }
}

/*******************************************************************************/
void
st25r3916ledEvtWrReg(uint8_t reg, uint8_t val)
{
    if (reg == ST25R3916_REG_OP_CONTROL) {
        if ((ST25R3916_REG_OP_CONTROL_tx_en & val) != 0U) {
            st25r3916ledFieldOn();
        } else {
            st25r3916ledFieldOff();
        }
    }
}

/*******************************************************************************/
void
st25r3916ledEvtWrMultiReg(uint8_t reg, const uint8_t *vals, uint8_t len)
{
    uint8_t i;

    for (i = 0; i < (len); i++) {
        st25r3916ledEvtWrReg((reg + i), vals[i]);
    }
}

/*******************************************************************************/
void
st25r3916ledEvtCmd(uint8_t cmd)
{
    if ((cmd >= ST25R3916_CMD_TRANSMIT_WITH_CRC) &&
        (cmd <= ST25R3916_CMD_RESPONSE_RF_COLLISION_N)) {
        st25r3916ledFieldOff();
    }

    if (cmd == ST25R3916_CMD_UNMASK_RECEIVE_DATA) {
        st25r3916ledRxOff();
    }

    if (cmd == ST25R3916_CMD_SET_DEFAULT) {
        st25r3916ledFieldOff();
        st25r3916ledRxOff();
    }
}
