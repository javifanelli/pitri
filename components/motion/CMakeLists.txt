idf_component_register(SRCS "motion.c"
                       INCLUDE_DIRS "include"
                       REQUIRES board_config esp_driver_gpio esp_timer)
