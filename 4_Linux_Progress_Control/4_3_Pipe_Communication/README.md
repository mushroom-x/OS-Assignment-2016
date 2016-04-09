>FUNCTION DESCRIPTION

由父进程创建一个管道，然后再创建2个子进程，并由这两个兄弟进程利用管道进行进程通信：子进程1使用管道的写端，子进程2使用管道的读端。通信的具体内容可根据自己的需要随意设计，要求能试验阻塞型读写过程中的各种情况。运行程序，观察各种情况下，进程实际读写的字节数以及进程阻塞唤醒的情况。

>COMPILE

```
make
```

>RUN

```
./pipe_communication
```

>output

```
Write Process : Wanna input 70 characters 
Write Process : Wrote in 70 characters ... 
Write Process: Sleep - 10 Seconds ... 
Read Process : Time 1 
Read Process : Wanna read 50 characters. 
Read Process : Read 50 characters 
Read Process : Time 2
Read Process : Wanna read 50 characters. 
Read Process : Read 20 characters 
Read Process : Time 3
Read Process : Wanna read 70 characters.
Write Process: Wake up then Why not rewrite 70 characters ? 
Write Process : Wanna input 70 characters 
Write Process : Wrote in 70 characters ... 
Read Process : Read 70 characters 
Write Process : Done 
-------------End of Program-----------
```
