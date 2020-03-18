CMAKE_MINIMUM_REQUIRED(VERSION 3.15)

PROJECT(mongoose_test)

SET(CMAKE_CXX_STANDARD 20)

INSTALL(FILES index.html DESTINATION /)

ADD_EXECUTABLE(mongoose_test main.cpp mongoose/mongoose.c mongoose/mongoose.h api/api_server.cpp api/api_server.hpp api/routing_context.cpp api/routing_context.hpp)