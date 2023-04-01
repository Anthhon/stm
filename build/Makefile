SRC_PATH = ../src/
CLIENT_FILE_NAME_LINUX = client
SERVER_FILE_NAME_LINUX = server
CLIENT_FILE_NAME_WINDOWS = client.exe
SERVER_FILE_NAME_WINDOWS = server.exe
LIN_FLAGS = -Wall -Wextra 
WIN_FLAGS = -Wall -Wextra -static-libgcc -lws2_32

build-linux:
	gcc -o $(CLIENT_FILE_NAME_LINUX) $(SRC_PATH)client.c $(LIN_FLAGS)
	gcc -o $(SERVER_FILE_NAME_LINUX) $(SRC_PATH)server.c $(LIN_FLAGS)
build-windows:
	x86_64-w64-mingw32-gcc -o $(CLIENT_FILE_NAME_WINDOWS) $(SRC_PATH)client.c $(WIN_FLAGS)
	x86_64-w64-mingw32-gcc -o $(SERVER_FILE_NAME_WINDOWS) $(SRC_PATH)client.c $(WIN_FLAGS)
clean:
	rm client server
