#pragma once

#include "driver/gpio.h"

#include "button_gpio.h"

// Pin Map
#if defined(CONFIG_WMD_DEVICE_C6)
    #define WMD_SPI_SCLK GPIO_NUM_7
    #define WMD_SPI_MISO GPIO_NUM_5
    #define WMD_SPI_MOSI GPIO_NUM_6

    #define WMD_LCD_MISO GPIO_NUM_NC
    #define WMD_LCD_MOSI WMD_SPI_MOSI
    #define WMD_LCD_SCLK WMD_SPI_SCLK
    #define WMD_LCD_CS GPIO_NUM_14
    #define WMD_LCD_BL GPIO_NUM_22
    #define WMD_LCD_DC GPIO_NUM_15
    #define WMD_LCD_RST GPIO_NUM_21

    #define WMD_SD_SCLK WMD_SPI_SCLK
    #define WMD_SD_MOSI WMD_SPI_MOSI
    #define WMD_SD_MISO WMD_SPI_MISO
    #define WMD_SD_CS GPIO_NUM_4

    #define WMD_RGB_LED GPIO_NUM_8

    #define WMD_BTN_BOOT GPIO_NUM_9
#elif defined(CONFIG_WMD_DEVICE_S3)
    #define WMD_LCD_MISO GPIO_NUM_NC
    #define WMD_LCD_MOSI GPIO_NUM_45
    #define WMD_LCD_SCLK GPIO_NUM_40
    #define WMD_LCD_CS GPIO_NUM_42
    #define WMD_LCD_BL GPIO_NUM_48
    #define WMD_LCD_DC GPIO_NUM_41
    #define WMD_LCD_RST GPIO_NUM_39

    #define WMD_RGB_LED GPIO_NUM_38

    #define WMD_BTN_BOOT GPIO_NUM_0
#endif

// SPI configuration
#define WMD_SPI_HOST SPI2_HOST
#define WMD_SPI_SPEED 42 * 1000 * 1000 // 42 MHz
#define WMD_SPI_MODE 0 // CPOL = 0 / CPHA = 0

// Configuration for LEDC periphal to steer backlight
#define WMD_LCD_BL_TIM LEDC_TIMER_0
#define WMD_LCD_BL_CHAN LEDC_CHANNEL_0
#define WMD_LCD_BL_SPD LEDC_LOW_SPEED_MODE
#define WMD_LCD_BL_FREQ 1000
#define WMD_LCD_BL_RES LEDC_TIMER_8_BIT
#define WMD_LCD_BL_LVL_FULL 255
#define WMD_LCD_BL_LVL_HIGH 192
#define WMD_LCD_BL_LVL_MID 128
#define WMD_LCD_BL_LVL_LOW 64
#define WMD_LCD_BL_LVL_OFF 0

// LCD configuration
#if defined(CONFIG_WMD_LCD_ORIENTATION_PORTRAIT)
    #define WMD_LCD_WIDTH 172
    #define WMD_LCD_HEIGHT 320
#elif defined(CONFIG_WMD_LCD_ORIENTATION_LANDSCAPE)
    #define WMD_LCD_WIDTH 320
    #define WMD_LCD_HEIGHT 172
#endif

#define WMD_LCD_CMD_BITS 8
#define WMD_LCD_PARAM_BITS 8
#define WMD_LCD_GAP 34
#define WMD_LCD_BITS_PER_PIXEL 16

/**
 * Fully initialize display driver, display, LVGL and RGB LED
 */
void wmd_init();

/**
 * Set backlight brightness level 0-255
 */
void wmd_set_backlight(const uint8_t level);

/**
 * Set RGB LED color/brightness
 */
void wmd_set_rgb_led(const uint32_t red, const uint32_t green, const uint32_t blue);

/**
 * Return the boot button handle to allow external code to bind callbacks to it
 */
button_handle_t* wmd_button_get_handle();

/**
 * Returns, if the sdcard is ready to use
 */
bool wmd_is_sdcard_ready();