###########################################  
#Makefile for simple programs  
###########################################  
INC=
LIB= -lpthread

CC=CC 
CC_FLAG=-Wall

PRG=threadpooltest  
OBJ=CThreadManage.o CThreadPool.o CThread.o CWorkerThread.o threadpooltest.o

$(PRG):$(OBJ)
	$(CC) $(INC) $(LIB) -o $@ $(OBJ)

.SUFFIXES: .c .o .cpp
.cpp.o:
	$(CC) $(CC_FLAG) $(INC) -c $*.cpp -o $*.o

.PRONY:clean
clean:
	@echo "Removing linked and compiled files......"
	rm -f $(OBJ) $(PRG)