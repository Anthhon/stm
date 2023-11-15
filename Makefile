#!/bin/bash

CC=gcc
WCC=x86_64-w64-mingw32-gcc
FLAGS=-Wall -Werror -Wextra
LINUX_LIBS=-pthread -ltermui
WINDOWS_LIBS=-pthread -static-libgcc -lsw2_32

PATH_SRC=src
PATH_BUILD=build
FILE_SRC_CLIENT=client.c
FILE_SRC_SERVER=server.c
FILE_BIN_CLIENT=stm
FILE_BIN_SERVER=stm-server

build-linux:
	@echo Checking build path...
	@mkdir -p $(PATH_BUILD)
	@echo Building client...
	$(CC) $(FLAGS) -o $(PATH_BUILD)/$(FILE_BIN_CLIENT) $(PATH_SRC)/$(FILE_SRC_CLIENT) $(LINUX_LIBS)
	@echo Building server...
	$(CC) $(FLAGS) -o $(PATH_BUILD)/$(FILE_BIN_SERVER) $(PATH_SRC)/$(FILE_SRC_SERVER) $(LINUX_LIBS)

build-linux-debug:
	@echo Checking build path...
	@mkdir -p $(PATH_BUILD)
	@echo Building client...
	$(CC) $(FLAGS) -o $(PATH_BUILD)/$(FILE_BIN_CLIENT) $(PATH_SRC)/$(FILE_SRC_CLIENT) $(LINUX_LIBS) -g
	@echo Building server...
	$(CC) $(FLAGS) -o $(PATH_BUILD)/$(FILE_BIN_SERVER) $(PATH_SRC)/$(FILE_SRC_SERVER) $(LINUX_LIBS) -g

build-windows:
	@echo Checking build path...
	@mkdir -p $(PATH_BUILD)
	@echo Building client...
	$(WCC) $(WINDOWS_LIBS) -o $(PATH_BUILD)/$(FILE_BIN_CLIENT).exe $(PATH_SRC)/$(FILE_SRC_CLIENT) $(FLAGS)
	@echo Building server...
	$(WCC) $(WINDOWS_LIBS) -o $(PATH_BUILD)/$(FILE_BIN_SERVER).exe $(PATH_SRC)/$(FILE_SRC_SERVER) $(FLAGS)
