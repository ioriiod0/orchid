##什么是orchid?
orchid是一个构建于boost库基础上的高性能协程/网络库，类似于python下的gevent/eventlet，为用户提供基于协程的并发模型。

####什么是协程：
协程，即协作式程序，其思想是，一系列互相依赖的协程间依次使用CPU，每次只有一个协程工作，而其他协程处于休眠状态。协程在控制离开时暂停执行，当控制再次进入时只能从离开的位置继续执行。
协程已经被证明是一种非常有用的程序组件，不仅被python、lua、ruby等脚本语言广泛采用，而且被新一代面向多核的编程语言如golang rust-lang等采用作为并发的基本单位。

####协程可以被认为是一种用户空间线程，与传统的抢占式线程相比，有2个主要的优点：
* 与线程不同，协程是自己主动让出CPU，并交付他期望的下一个协程运行，而不是在任何时候都有可能被系统调度打断。因此协程的使用更加清晰易懂，并且多数情况下不需要锁机制。
* 与线程相比，协程的切换由程序控制，发生在用户空间而非内核空间，因此切换的代价非常的小。

####green化
术语“green化”来自于python下著名的协程库greenlet，指改造IO对象以能和协程配合。某种意义上，协程与线程的关系类似与线程与进程的关系，多个协程会在同一个线程的上下文之中运行。因此，当出现IO操作的时候，为了能够与协程相互配合，只阻塞当前协程而非整个线程，需要将io对象“green化”。目前orchid提供的green化的io对象包括：

* tcp socket（还不支持udp）
* descriptor（目前仅支持非文件类型文件描述符，如管道和标准输入/输出，文件类型的支持会在以后版本添加）
* timer (定时器)
* signal (信号)

####chan：协程间通信
chan这个概念引用自golang的chan。每个协程是一个独立的执行单元，为了能够方便协程之间的通信/同步，orchid提供了chan这种机制。chan本质上是一个阻塞消息队列，后面我们将看到，chan不仅可以用于同一个调度器上的协程之间的通信，而且可以用于不同调度器上的协程之间的通信。

####多核
建议使用的scheduler per cpu的的模型来支持多核的机器，即为每个CPU核心分配一个调度器，有多少核心就创建多少个调度器。不同调度器的协程之间也可以通过chan来通信。协程应该被创建在那个调度器里由用户自己决定。

进一步信息请阅读doc目录下的tutial文件，如果您发现任何bug或者有好的建议请联系ioriiod0@gmail.com

####TODO

* 完善api文档
* 支持udp
* 支持timeout io
