#
# "main" pseudo-component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)

COMPONENT_ADD_INCLUDEDIRS := ./include
COMPONENT_SRCDIRS := .
COMPONENT_PRIV_INCLUDEDIRS := ./driver/include

COMPONENT_ADD_INCLUDEDIRS += ./driver/es8388
COMPONENT_SRCDIRS += ./driver/es8388
