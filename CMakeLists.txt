cmake_minimum_required(VERSION 3.22.1)

# Default project config
project(stm VERSION 0.1) # Project name and version
add_executable(stm src/server.c src/client.c) # Defines the executable name
# target_link_libraries(stm sys/socket) # Include some libs if needed
install(TARGETS stm DESTINATION bin) # Tells where to store the binarie
