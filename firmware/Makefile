TARGET = rools

CPP_SOURCES = \
src/main.cpp \
src/app_shell.cpp \
src/display/st7735.cpp \
src/display/gfx.cpp \
src/dsp/fft_analyzer.cpp \
src/ui/spectrum_view.cpp \
src/apps/app_registry.cpp \
src/apps/spectrum_app.cpp

LIBDAISY_DIR ?= ../lib/libDaisy
DAISYSP_DIR  ?= ../lib/DaisySP

C_INCLUDES += -Isrc

SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
