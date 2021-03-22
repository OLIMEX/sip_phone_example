set(COMPONENT_ADD_INCLUDEDIRS ./include
                            ./driver/es8388
                            ./driver/include)

# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES )
set(COMPONENT_PRIV_REQUIRES audio_sal audio_board mbedtls esp_peripherals display_service esp_dispatcher)

set(COMPONENT_SRCS ./audio_hal.c
                    ./driver/es8388/es8388.c
                    ./driver/es8388/headphone_detect.c)

register_component()
