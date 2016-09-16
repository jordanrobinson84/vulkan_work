# Use g++
CC = g++

CFLAGS = -Wall -std=c++11

test1: vulkanDriverInstance.o vulkanBuffer.o vulkanCommandPool.o vulkanRenderPass.o vulkanSwapchain.o test1.o
	$(CC) $(CFLAGS) vulkanCommandPool.o vulkanBuffer.o vulkanDriverInstance.o vulkanRenderPass.o vulkanSwapchain.o test1.o -o test1 -rdynamic -ldl -lxcb -lxcb-keysyms -lxcb-randr

test1.o: test1.cpp vulkanDriverInstance.h vulkanBuffer.h
	$(CC) $(CFLAGS) -c test1.cpp

vulkanDriverInstance.o: vulkanDriverInstance.cpp vulkanDriverInstance.h vulkanCommandPool.h
	$(CC) $(CFLAGS) -c vulkanDriverInstance.cpp

vulkanBuffer.o: vulkanBuffer.cpp vulkanBuffer.h vulkanDriverInstance.h vulkanCommandPool.h
	$(CC) $(CFLAGS) -c vulkanBuffer.cpp

vulkanCommandPool.o: vulkanCommandPool.cpp vulkanCommandPool.h vulkanDriverInstance.h
	$(CC) $(CFLAGS) -c vulkanCommandPool.cpp

vulkanRenderPass.o: vulkanRenderPass.cpp vulkanRenderPass.h vulkanDriverInstance.h
	$(CC) $(CFLAGS) -c vulkanRenderPass.cpp

vulkanSwapchain.o: vulkanSwapchain.cpp vulkanSwapchain.h vulkanDriverInstance.h
	$(CC) $(CFLAGS) -c vulkanSwapchain.cpp

clean:
	$(RM) test1 *.o *~