idf_component_register(SRCS "wifi.c"
                       INCLUDE_DIRS "include"
                       REQUIRES esp_wifi esp_netif esp_event nvs_flash esp_timer)

# wifi_secretos.h no se sube al repo, así que puede aparecer DESPUÉS de una
# compilación. Sin esto, la compilación no se da cuenta y sigue usando el
# wifi.c viejo (sin datos). CONFIGURE_DEPENDS hace que en cada build se revise
# si el archivo apareció o desapareció, y que wifi.c se recompile si cambia.
file(GLOB secretos CONFIGURE_DEPENDS "${COMPONENT_DIR}/wifi_secretos.h")
if(secretos)
    target_compile_definitions(${COMPONENT_LIB} PRIVATE PITRI_HAY_SECRETOS_WIFI=1)
    set_source_files_properties(wifi.c PROPERTIES OBJECT_DEPENDS "${secretos}")
endif()
