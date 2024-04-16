#!/bin/bash

CC=gcc
WCC=x86_64-w64-mingw32-gcc
FLAGS=-Wall -Werror -Wextra
LINUX_LIBS=-pthread
WINDOWS_LIBS=-pthread -static-libgcc -lsw2_32

PATH_SRC=src
PATH_BUILD=build
FILE_SRC_CLIENT=client.c
FILE_SRC_SERVER=server.c
FILE_BIN_CLIENT=stm
FILE_BIN_SERVER=stm-server
TERMUI_PATH=$(PATH_SRC)/libs/termui

CLIENT_FILES := $(FILE_SRC_CLIENT) message.c
SERVER_FILES := $(FILE_SRC_SERVER)

build-linux:
	@echo Checking build path...
	@mkdir -p $(PATH_BUILD)
	@echo Building client...
	$(CC) $(FLAGS) -o $(PATH_BUILD)/$(FILE_BIN_CLIENT) $(addprefix $(PATH_SRC)/, $(CLIENT_FILES)) $(wildcard $(TERMUI_PATH)/*.c) $(LINUX_LIBS)
	@echo Building server...
	$(CC) $(FLAGS) -o $(PATH_BUILD)/$(FILE_BIN_SERVER) $(addprefix $(PATH_SRC)/, $(SERVER_FILES)) $(wildcard $(TERMUI_PATH)/*.c) $(LINUX_LIBS)

build-linux-debug:
	@echo Checking build path...
	@mkdir -p $(PATH_BUILD)
	@echo Building client...
	$(CC) $(FLAGS) -o $(PATH_BUILD)/$(FILE_BIN_CLIENT) $(addprefix $(PATH_SRC)/, $(CLIENT_FILES)) $(wildcard $(TERMUI_PATH)/*.c) $(LINUX_LIBS) -g
	@echo Building server...
	$(CC) $(FLAGS) -o $(PATH_BUILD)/$(FILE_BIN_SERVER) $(addprefix $(PATH_SRC)/, $(SERVER_FILES)) $(wildcard $(TERMUI_PATH)/*.c) $(LINUX_LIBS) -g

build-windows:
	@echo Checking build path...
	@mkdir -p $(PATH_BUILD)
	@echo Building client...
	$(WCC) $(WINDOWS_LIBS) -o $(PATH_BUILD)/$(FILE_BIN_CLIENT).exe $(addprefix $(PATH_SRC)/, $(CLIENT_FILES)) $(wildcard $(TERMUI_PATH)/*.c) $(FLAGS)
	@echo Building server...
	$(WCC) $(WINDOWS_LIBS) -o $(PATH_BUILD)/$(FILE_BIN_SERVER).exe $(addprefix $(PATH_SRC)/, $(CLIENT_FILES)) $(wildcard $(TERMUI_PATH)/*.c) $(FLAGS)
