idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES board_config display eyes motion dht wifi botones menu reloj esp_timer)
