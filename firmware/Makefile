TARGET = rools

CPP_SOURCES = \
src/main.cpp \
src/app_shell.cpp \
src/board/cv_reference.cpp \
src/board/enc_debug.cpp \
src/display/st7735.cpp \
src/display/gfx.cpp \
src/dsp/fft_analyzer.cpp \
src/dsp/dsp_memory_pool.cpp \
src/input/input_gesture_controller.cpp \
src/ui/spectrum_view.cpp \
src/ui/oscilloscope_view.cpp \
src/ui/bottom_bar_view.cpp \
src/ui/app_menu_view.cpp \
src/ui/layout_view.cpp \
src/apps/app_registry.cpp \
src/apps/display_test_app.cpp \
src/apps/oscilloscope_app.cpp \
src/apps/spectrum_app.cpp

LIBDAISY_DIR ?= ../lib/libDaisy
DAISYSP_DIR  ?= ../lib/DaisySP

C_INCLUDES += -Isrc

SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
