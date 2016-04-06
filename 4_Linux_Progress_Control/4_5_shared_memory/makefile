objects = init.o common.o sender.o

edit : $(objects)
	cc -o edit $(objects)

common.o : common.c

init.o : common.h

sender.o : common.h

receiver.o : common.h

#init : init.o common.o
#	cc -pthread -o init init.o common.o

.PHONY : clean

clean : 
	rm edit $(objects)
