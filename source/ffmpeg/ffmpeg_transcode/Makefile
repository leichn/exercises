CC=gcc
SRCS=$(wildcard *.c */*.c)
OBJS=$(patsubst %.c, %.o, $(SRCS))
FLAG=-g -std=c99
LIB=-lavutil -lavformat -lavcodec -lavutil -lswscale -lswresample -lavfilter -lSDL2
NAME=$(wildcard *.c)
TARGET=transcode

$(TARGET):$(OBJS)
	$(CC) $(LIB) -o $@ $^ $(FLAG)

%.o:%.c
	$(CC) -o $@ -c $< $(FLAG)

clean:
	rm -rf $(TARGET) $(OBJS)
