Work Flow 

COMPILE

1.make  [There is a bug,I don't know  how to add -pthread to "gcc -o ..." in makefile]
2.
[if the bug is fixed then those arguments  could be cancel]
gcc -pthread -o init init.o common.o
gcc -pthread -o sender sender.o common.o
gcc -pthread -o receiver receiver.o common.o


3.make clean
