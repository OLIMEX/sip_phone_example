#
# "main" pseudo-component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)

COMPONENT_ADD_INCLUDEDIRS := ./include

ifdef CONFIG_ESP32_ADF_REVB_BOARD
COMPONENT_ADD_INCLUDEDIRS += ./olimex_esp32_adf_b
COMPONENT_SRCDIRS += ./olimex_esp32_adf_b
endif

ifdef CONFIG_ESP32_ADF_REVC_BOARD
COMPONENT_ADD_INCLUDEDIRS += ./olimex_esp32_adf_c
COMPONENT_SRCDIRS += ./olimex_esp32_adf_c
endif
