/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2019 <ESPRESSIF SYSTEMS (SHANGHAI) CO., LTD>
 *
 * Permission is hereby granted for use on all ESPRESSIF SYSTEMS products, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef _AUDIO_BOARD_DEFINITION_H_
#define _AUDIO_BOARD_DEFINITION_H_


// === Screen orientation constants ===
#define PORTRAIT	0
#define LANDSCAPE	1
#define PORTRAIT_FLIP	2
#define LANDSCAPE_FLIP	3

#define SDCARD_OPEN_FILE_NUM_MAX  5
#define SDCARD_INTR_GPIO          -1 //GPIO_NUM_34

#define BUTTON_REC_ID             GPIO_NUM_36
#define BUTTON_MODE_ID            GPIO_NUM_39
#define BUTTON_SET_ID             TOUCH_PAD_NUM9
#define BUTTON_PLAY_ID            TOUCH_PAD_NUM8
#define BUTTON_VOLUP_ID           TOUCH_PAD_NUM7
#define BUTTON_VOLDOWN_ID         TOUCH_PAD_NUM0

#define AUXIN_DETECT_GPIO         -1 //GPIO_NUM_12
#define HEADPHONE_DETCET          -1 //GPIO_NUM_19
#define PA_ENABLE_GPIO            -1 //GPIO_NUM_21

#define GREEN_LED_GPIO          GPIO_NUM_2
#define IR_RED_RXD		GPIO_NUM_22

#define SPI_PORT HSPI_HOST
#define DISP_SPI_MOSI 13
#define DISP_SPI_CLK  14
#define DISP_SPI_CS   15
#define ILI9341_DC   12
#define ILI9341_RST  -1
#define ILI9341_BCKL -1
#define I2C_PORT I2C_NUM_0
#define I2C_SDA	18
#define I2C_SCL	23

#define UARTTX	GPIO_NUM_19
#define UARTRX 	34
#define AUDIO_CODEC_DEFAULT_CONFIG(){                   \
        .adc_input  = AUDIO_HAL_ADC_INPUT_LINE1,        \
        .dac_output = AUDIO_HAL_DAC_OUTPUT_ALL,         \
        .codec_mode = AUDIO_HAL_CODEC_MODE_BOTH,        \
        .i2s_iface = {                                  \
            .mode = AUDIO_HAL_MODE_SLAVE,               \
            .fmt = AUDIO_HAL_I2S_NORMAL,                \
            .samples = AUDIO_HAL_48K_SAMPLES,           \
            .bits = AUDIO_HAL_BIT_LENGTH_16BITS,        \
        },                                              \
};

#define INPUT_KEY_NUM     6

#define INPUT_KEY_DEFAULT_INFO() {                      \
     {                                                  \
        .type = PERIPH_ID_BUTTON,                       \
        .user_id = INPUT_KEY_USER_ID_REC,               \
        .act_id = BUTTON_REC_ID,                        \
    },                                                  \
    {                                                   \
        .type = PERIPH_ID_BUTTON,                       \
        .user_id = INPUT_KEY_USER_ID_MODE,              \
        .act_id = BUTTON_MODE_ID,                       \
    },                                                  \
    {                                                   \
        .type = PERIPH_ID_TOUCH,                        \
        .user_id = INPUT_KEY_USER_ID_SET,               \
        .act_id = BUTTON_SET_ID,                        \
    },                                                  \
    {                                                   \
        .type = PERIPH_ID_TOUCH,                        \
        .user_id = INPUT_KEY_USER_ID_PLAY,              \
        .act_id = BUTTON_PLAY_ID,                       \
    },                                                  \
    {                                                   \
        .type = PERIPH_ID_TOUCH,                        \
        .user_id = INPUT_KEY_USER_ID_VOLUP,             \
        .act_id = BUTTON_VOLUP_ID,                      \
    },                                                  \
    {                                                   \
        .type = PERIPH_ID_TOUCH,                        \
        .user_id = INPUT_KEY_USER_ID_VOLDOWN,           \
        .act_id = BUTTON_VOLDOWN_ID,                    \
    }                                                   \
}

#endif
