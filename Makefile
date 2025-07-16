SRC = src
INC = include

PROJECT = BodySim

SRCS = $(wildcard $(SRC)/*.c)
OBJS = $(SRCS:.c=.o)

CC = gcc
CFLAGS = -I$(INC) -DGRAPHICS_API_OPENGL_33 -Wall -Wextra
LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

all: $(PROJECT)

$(PROJECT): $(OBJS)
	$(CC) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(SRC)/*.o $(PROJECT)




