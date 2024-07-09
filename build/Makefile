CXX = g++
CFLAGS = -std=c++14 -o2 -Wall -g

TARGET = server
OBJS = ../webserver/log/*.cpp \
	   ../webserver/buffer/*.cpp \
	   ../webserver/http/*.cpp \
	   ../webserver/pool/*.cpp \
	   ../webserver/server/*.cpp \
	   ../webserver/timer/*.cpp \
	   ../webserver/main.cpp \
	   ../webserver/lib/*.cpp \
	   ../webserver/urls/*.cpp

all: $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o /home/wy/CLionProjects/cpp/TinyWebServer/build/bin/$(TARGET) -pthread

clean:
	rm -r /home/wy/CLionProjects/cpp/TinyWebServer/build/bin/$(TARGET)
