
#内容要求

（1） 用C语言完成线程的创建和撤消，并按先来先服务方式对多个线程进行调度。

（2） 将线程调度算法修改为时间片轮转算法，实现时间片轮转调度。（也可以结合优先权，实现优先权加时间片轮转算法的线程调度。）

（3） 改变时间片的大小，观察结果的变化。思考：为什么时间片不能太小或太大。

（4） 假设两个线程共用同一软件资源（如某一变量，或某一数据结构），请用记录型信号量来实现对它的互斥访问。

（5） 假设有两个线程共享一个可存放５个整数的缓冲，其中一个线程不停地计算１至５０的平方，并将结果放入缓冲中，另一个线程不断地从缓冲中取出结果，并将它们打印出来，请用记录型信号量实现这一生产者和消费者的同步问题。

（6） 实现消息缓冲通信，并与4、5中的简单通信进行比较。

（7） 思考：在线程间进行消息缓冲通信时，若对消息队列的访问没有满足互斥要求，情况将会怎样？


# 源文件详解
##文件列表
(1) T1_AB     : Print "a" "b"

(2) T2_Mutex  : Two thread  add to Sum

(3) T3_P_C    : Producer & Consumer

(4) T4_R_S    : Sender & Receiver


##运行

>github上查看gif可能会有一些卡，所以我也放到了我的简书博客上面 
[DOS 系列程序运行例程](http://www.jianshu.com/p/78e2542a9901)

###T1_AB

![Dos-T1-02.gif](http://upload-images.jianshu.io/upload_images/1199728-c6347ab382fa76ce.gif?imageMogr2/auto-orient/strip)

###T2_Mutex 


![T2_Mutex.gif](http://upload-images.jianshu.io/upload_images/1199728-423e3369f4da30b0.gif?imageMogr2/auto-orient/strip)

###T3_P_C

![T3_Producer_Consumer.gif](http://upload-images.jianshu.io/upload_images/1199728-cdb74016a9302bb2.gif?imageMogr2/auto-orient/strip)

###T4_R_S

![T4_Sender_Receiver.gif](http://upload-images.jianshu.io/upload_images/1199728-d38132734fbddee1.gif?imageMogr2/auto-orient/strip)
