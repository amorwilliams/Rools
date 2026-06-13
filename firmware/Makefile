TARGET = rools

CPP_SOURCES = \
src/main.cpp \
src/app_shell.cpp

LIBDAISY_DIR ?= $(or $(LIBDAISY_PATH),../../libDaisy)
DAISYSP_DIR  ?= $(or $(DAISYSP_PATH),../../DaisySP)

C_INCLUDES += -Isrc

SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
