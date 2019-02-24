CC=gcc
SRCS_IFILE=vfilter_filesrc.c video_play.c video_filter.c
OBJS_IFILE=$(patsubst %.c, %.o, $(SRCS_IFILE))
SRCS_ITEST=vfilter_testsrc.c video_play.c video_filter.c
OBJS_ITEST=$(patsubst %.c, %.o, $(SRCS_ITEST))
FLAG=-g
LIB=-lavutil -lavformat -lavcodec -lavutil -lswscale -lswresample -lavfilter -lSDL2
NAME=$(wildcard *.c)
TARGET=vf_file vf_test

all:$(TARGET)

vf_file:$(OBJS_IFILE)
	$(CC) $(LIB) -o $@ $^ $(FLAG)

vf_test:$(OBJS_ITEST)
	$(CC) $(LIB) -o $@ $^ $(FLAG)

%.o:%.c
	$(CC) -o $@ -c $< -g

clean:
	rm -rf $(TARGET) $(OBJS_ITEST) $(OBJS_IFILE)
