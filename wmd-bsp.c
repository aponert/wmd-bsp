#include "wmd-bsp.h"

#include "driver/spi_master.h"

#include "esp_lcd_io_spi.h"
#include "esp_lcd_panel_dev.h"
#include "esp_lcd_panel_st7789.h"
#include "esp_lcd_panel_ops.h"

#include "driver/ledc.h"

#include "esp_lvgl_port.h"

#include "led_strip.h"

static led_strip_handle_t led_strip = NULL;

/**
 * Initialize SPI bus
 */
static void spi_init()
{
    const spi_bus_config_t spi_bus_config = {
        .sclk_io_num = WMD_LCD_SCLK,
        .quadwp_io_num = GPIO_NUM_NC,
        .quadhd_io_num = GPIO_NUM_NC,
        .mosi_io_num = WMD_LCD_MOSI,
        .miso_io_num = WMD_LCD_MISO,
        .max_transfer_sz = WMD_LCD_WIDTH * WMD_LCD_HEIGHT * sizeof(uint16_t),
    };

    spi_bus_initialize(WMD_LCD_SPI_HOST, &spi_bus_config, SPI_DMA_CH_AUTO);
}

/**
 * Initialize ESP32 LCD peripheral
 */
static void lcd_init(esp_lcd_panel_io_handle_t *panel_io_handle, esp_lcd_panel_handle_t *panel_handle)
{
    const esp_lcd_panel_io_spi_config_t spi_io_config = {
        .cs_gpio_num = WMD_LCD_CS,
        .dc_gpio_num = WMD_LCD_DC,
        .pclk_hz = WMD_LCD_SPI_SPEED,
        .spi_mode = WMD_LCD_SPI_MODE,
        .trans_queue_depth = 10,
        .lcd_cmd_bits = WMD_LCD_CMD_BITS,
        .lcd_param_bits = WMD_LCD_PARAM_BITS,
    };

    esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)WMD_LCD_SPI_HOST, &spi_io_config, panel_io_handle);

    const esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = WMD_LCD_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = WMD_LCD_BITS_PER_PIXEL,
    };

    esp_lcd_new_panel_st7789(*panel_io_handle, &panel_config, panel_handle);
}

/**
 * Initialize backlight with LEDC peripheral
 */
static void backlight_init()
{
    const ledc_timer_config_t tim_conf = {
        .clk_cfg = LEDC_AUTO_CLK,
        .freq_hz = WMD_LCD_BL_FREQ,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = WMD_LCD_BL_TIM,
        .duty_resolution = LEDC_TIMER_8_BIT,
    };
    ledc_timer_config(&tim_conf);

    const ledc_channel_config_t chan_conf = {
        .channel = WMD_LCD_BL_CHAN,
        .gpio_num = WMD_LCD_BL,
        .timer_sel = WMD_LCD_BL_TIM,
    };
    ledc_channel_config(&chan_conf);
    wmd_set_backlight(WMD_LCD_BL_LVL_MID);
}

/**
 * Initialize LVGL via lvgl port library
 */
static lv_disp_t* lvgl_init(esp_lcd_panel_io_handle_t *io_handle, esp_lcd_panel_handle_t *panel_handle)
{
    const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    lvgl_port_init(&lvgl_cfg);

    static lv_disp_t * disp_handle;

    /* Add LCD screen */
    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = *io_handle,
        .panel_handle = *panel_handle,
        .buffer_size = WMD_LCD_WIDTH*WMD_LCD_HEIGHT,
        .double_buffer = true,
        .hres = WMD_LCD_WIDTH,
        .vres = WMD_LCD_HEIGHT,
        .monochrome = false,
        .color_format = LV_COLOR_FORMAT_RGB565,
        .rotation = {
#if defined(CONFIG_WMD_LCD_ORIENTATION_PORTRAIT)
            .swap_xy = false,
#elif defined(CONFIG_WMD_LCD_ORIENTATION_LANDSCAPE)
            .swap_xy = true,
#endif
            .mirror_x = false,
#if defined(CONFIG_WMD_LCD_ORIENTATION_PORTRAIT)
            .mirror_y = false,
#elif defined(CONFIG_WMD_LCD_ORIENTATION_LANDSCAPE)
            .mirror_y = true,
#endif
        },
        .flags = {
            .buff_dma = true,
            .swap_bytes = true,
        }
    };
    disp_handle = lvgl_port_add_disp(&disp_cfg);
    return disp_handle;
}

/**
 * Initialize RGB LED
 */
static void rgb_led_init()
{
    const led_strip_config_t strip_conf = {
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_RGB,
        .max_leds = 1,
        .led_model = LED_MODEL_WS2812,
        .strip_gpio_num = WMD_RGB_LED,
    };

    const led_strip_rmt_config_t rmt_conf = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .flags.with_dma = false,
        .resolution_hz = 10 * 1000 * 1000,
        .mem_block_symbols = 64,
    };

    led_strip_new_rmt_device(&strip_conf, &rmt_conf, &led_strip);
}

/**
 * Set backlight brightness level 0-255
 */
void wmd_set_backlight(const uint8_t level)
{
    ledc_set_duty(LEDC_LOW_SPEED_MODE, WMD_LCD_BL_CHAN, level);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, WMD_LCD_BL_CHAN);
}

/**
 * Set RGB LED color/brightness
 */
void wmd_set_rgb_led(const uint32_t red, const uint32_t green, const uint32_t blue)
{
    led_strip_set_pixel(led_strip, 0, red, green, blue);
    led_strip_refresh(led_strip);
}

/**
 * Fully initialize display driver, display, LVGL and RGB LED
 */
void wmd_init()
{
    rgb_led_init();
    backlight_init();
    spi_init();
    esp_lcd_panel_handle_t panel_handle = {0};
    esp_lcd_panel_io_handle_t io_handle = {0};
    lcd_init(&io_handle, &panel_handle);

    esp_lcd_panel_reset(panel_handle);
    esp_lcd_panel_init(panel_handle);
    esp_lcd_panel_invert_color(panel_handle, true);
    esp_lcd_panel_disp_on_off(panel_handle, true);
#if defined(CONFIG_WMD_LCD_ORIENTATION_PORTRAIT)
    esp_lcd_panel_set_gap(panel_handle, WMD_LCD_GAP,0);
#elif defined(CONFIG_WMD_LCD_ORIENTATION_LANDSCAPE)
    esp_lcd_panel_set_gap(panel_handle, 0,WMD_LCD_GAP);
#endif

    lvgl_init(&io_handle, &panel_handle);
}