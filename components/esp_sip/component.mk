#
# "main" pseudo-component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)

COMPONENT_ADD_INCLUDEDIRS :=    esp_sip/include

COMPONENT_SRCDIRS := . esp_codec

LIBS := esp_sip

COMPONENT_ADD_LDFLAGS +=  -L$(COMPONENT_PATH)/esp_sip/lib \
                           $(addprefix -l,$(LIBS)) \

ALL_LIB_FILES += $(patsubst %,$(COMPONENT_PATH)/%/lib/lib%.a,$(LIBS))
