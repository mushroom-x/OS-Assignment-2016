>Function 

编写程序创建两个线程：sender线程和receive线程，其中sender线程运行函数sender()，
它创建一个消息队列，然后，循环等待用户通过终端输入一串字符，将这串字符通过消息队列发送给receiver线程，
直到用户输入“exit”为止；最后，它向receiver线程发送消息“end”，并且等待receiver的应答，等到应答消息后，
将接收到的应答信息显示在终端屏幕上，删除相关消息队列，结束程序的运行。Receiver线程运行receive()，
它通过消息队列接收来自sender的消息，将消息显示在终端屏幕上，直至收到内容为“end”的消息为止，此时，
它向sender发送一个应答消息“over”，结束程序的运行。
使用无名信号量实现两个线程之间的同步与互斥。


>使用说明

gif动画演示 
[4_4 message queue 例程演示](http://www.jianshu.com/p/b4a6699ede6a)
