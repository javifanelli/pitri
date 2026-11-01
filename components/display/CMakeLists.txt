idf_component_register(SRCS "display.c"
                       INCLUDE_DIRS "include"
                       REQUIRES board_config esp_lcd esp_driver_i2c sh1106-esp-idf)
