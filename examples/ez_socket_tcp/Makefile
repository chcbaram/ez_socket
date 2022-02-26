

all: clean  main


SV_SOCKET_DIR = ../../ez_socket

SRCS  = ./main.cpp 
SRCS += $(SV_SOCKET_DIR)/src/ez_log.cpp 
SRCS += $(SV_SOCKET_DIR)/src/ez_socket.cpp
SRCS += $(SV_SOCKET_DIR)/src/ez_utils.cpp

INCS  = -I.
INCS += -I$(SV_SOCKET_DIR)/include

CFLAGS = -O2 


OBJS = $(SRCS:.c=.o)


main:
	g++ -o main  $(CFLAGS) $(INCS) $(SRCS)  -lm -lws2_32


clean:
	rm -f main