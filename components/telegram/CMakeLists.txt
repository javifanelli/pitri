idf_component_register(SRCS "telegram.c"
                       INCLUDE_DIRS "include"
                       REQUIRES wifi esp_http_client mbedtls)

# telegram_secretos.h no se sube al repo, asi que puede aparecer DESPUES de una
# compilacion. CONFIGURE_DEPENDS hace que en cada build se revise si el archivo
# aparecio o desaparecio, y que telegram.c se recompile si cambia.
file(GLOB secretos CONFIGURE_DEPENDS "${COMPONENT_DIR}/telegram_secretos.h")
if(secretos)
    target_compile_definitions(${COMPONENT_LIB} PRIVATE PITRI_HAY_SECRETOS_TELEGRAM=1)
    set_source_files_properties(telegram.c PROPERTIES OBJECT_DEPENDS "${secretos}")
endif()
