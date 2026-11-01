idf_component_register(SRCS "display.c"
                       INCLUDE_DIRS "include"
                       REQUIRES board_config esp_lcd driver sh1106-esp-idf)
