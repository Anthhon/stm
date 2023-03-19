SRC_PATH = '../src/'
SERVER_IP = '127.0.0.1'
CLIENT_FILE_NAME = 'client'
SERVER_FILE_NAME = 'server'

build:
	gcc -o $(CLIENT_FILE_NAME) $(SRC_PATH)/client.c $(FLAGS)
	gcc -o $(SERVER_FILE_NAME) $(SRC_PATH)/server.c $(FLAGS)
run_server:
	./$(CLIENT_FILE_NAME) $(SERVER_IP) & ./$(SERVER_FILE_NAME)
clean:
	rm client server
