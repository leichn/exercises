CC 			= gcc
BUILD_DIR 	= ./build
OBJ_DIR		= $(BUILD_DIR)/obj/
BIN_DIR		= $(BUILD_DIR)/bin/
SRC_DIR		= ./
SRCS_IFILE	= vfilter_filesrc.c video_play.c video_filter.c
OBJS_IFILE	= $(patsubst %.c, %.o, $(SRCS_IFILE))
OBJS_IFILE := $(addprefix $(OBJ_DIR), $(OBJS_IFILE)) 
SRCS_ITEST	= vfilter_testsrc.c video_play.c video_filter.c
OBJS_ITEST	= $(patsubst %.c, %.o, $(SRCS_ITEST))
OBJS_ITEST := $(addprefix $(OBJ_DIR), $(OBJS_ITEST)) 
FLAG		= -g
LIBS		= -lavutil -lavformat -lavcodec -lavutil -lswscale -lswresample -lavfilter -lSDL2
#NAME		= $(wildcard *.c)
TARGET		= vf_file vf_test

ifneq ($(BUILD_DIR),)
# Attempt to create a output directory.
$(shell [ -d ${BIN_DIR} ] || mkdir -p ${BIN_DIR})
$(shell [ -d ${OBJ_DIR} ] || mkdir -p ${OBJ_DIR})
endif

all : $(TARGET)

vf_file:	$(BIN_DIR)vf_file

vf_test:	$(BIN_DIR)vf_test

$(BIN_DIR)vf_file:	$(OBJS_IFILE)
	@$(CC) $(LIBS) -o $@ $^ $(FLAG)
	@echo Generating $(BIN_DIR)vf_file done.

$(BIN_DIR)vf_test:	$(OBJS_ITEST)
	@$(CC) $(LIBS) -o $@ $^ $(FLAG)
	@echo Generating $(BIN_DIR)vf_test done.

$(OBJ_DIR)%.o:	$(SRC_DIR)%.c
	@echo Compiling $< ......
	@$(CC) -o $@ -c $< -g

clean:
	rm -rf $(BUILD_DIR)
#	rm -rf $(TARGET) $(OBJS_ITEST) $(OBJS_IFILE)
