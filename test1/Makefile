# Use g++
CC = g++

CFLAGS = -Wall -std=c++11

test1: vulkanDriverInstance.o vulkanBuffer.o test1.o
	$(CC) $(CFLAGS) vulkanBuffer.o vulkanDriverInstance.o test1.o -o test1 -ldl

test1.o: test1.cpp vulkanDriverInstance.h vulkanBuffer.h
	$(CC) $(CFLAGS) -c test1.cpp

vulkanDriverInstance.o: vulkanDriverInstance.cpp vulkanDriverInstance.h
	$(CC) $(CFLAGS) -c vulkanDriverInstance.cpp

vulkanBuffer.o: vulkanBuffer.cpp vulkanBuffer.h vulkanDriverInstance.h
	$(CC) $(CFLAGS) -c vulkanBuffer.cpp

clean:
	$(RM) test1 *.o *~