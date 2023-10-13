#!/bin/bash

CC=gcc
FLAGS=-Wall -Werror -Wextra
LIBS=-pthread

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
	$(CC) $(LIBS) -o $(PATH_BUILD)/$(FILE_BIN_CLIENT) $(PATH_SRC)/$(FILE_SRC_CLIENT) $(FLAGS)
	@echo Building server...
	$(CC) $(LIBS) -o $(PATH_BUILD)/$(FILE_BIN_SERVER) $(PATH_SRC)/$(FILE_SRC_SERVER) $(FLAGS)

build-linux-debug:
	@echo Checking build path...
	@mkdir -p $(PATH_BUILD)
	@echo Building client...
	$(CC) $(LIBS) -o $(PATH_BUILD)/$(FILE_BIN_CLIENT) $(PATH_SRC)/$(FILE_SRC_CLIENT) $(FLAGS) -d
	@echo Building server...
	$(CC) $(LIBS) -o $(PATH_BUILD)/$(FILE_BIN_SERVER) $(PATH_SRC)/$(FILE_SRC_SERVER) $(FLAGS) -d
