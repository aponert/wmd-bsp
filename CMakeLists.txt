idf_component_register(SRCS "wmd-bsp.c"
                    INCLUDE_DIRS "include"
                    REQUIRES "esp_driver_gpio" "esp_driver_spi"
                            "esp_lcd" "esp_lvgl_port" "led_strip"
                            "button" "esp_driver_sdspi" "fatfs")
