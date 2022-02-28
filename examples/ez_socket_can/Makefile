

all: clean  main


SV_SOCKET_DIR = ../../ez_socket

SRCS  = ./main.cpp 
SRCS += $(SV_SOCKET_DIR)/src/ez_log.cpp 
SRCS += $(SV_SOCKET_DIR)/src/ez_socket.cpp
SRCS += $(SV_SOCKET_DIR)/src/ez_utils.cpp
SRCS += $(SV_SOCKET_DIR)/src/ez_qbuffer.cpp
SRCS += ./src/bsp/bsp.c
SRCS += ./src/hw/cmd_can.c

INCS  = -I.
INCS += -I$(SV_SOCKET_DIR)/include
INCS += -I./src/bsp
INCS += -I./src/common
INCS += -I./src/hw

CFLAGS = -O2 -std=gnu++14 


OBJS = $(SRCS:.c=.o)


main:
	g++ -o main  $(CFLAGS) $(INCS) $(SRCS)  -lm -lws2_32


clean:
	rm -f main