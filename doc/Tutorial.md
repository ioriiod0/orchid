#简介
##什么是协程？
协程，顾名思义，协作式程序，其思想是，一系列互相依赖的协程间依次使用CPU，每次只有一个协程工作，而其他协程处于休眠状态。

####定义：
* 协程是一种程序组件，是由子例程（过程、函数、例程、方法、子程序）的概念泛化而来的，子例程只有一个入口点且只返回一次，而协程允许多个入口点，可以在指定位置挂起和恢复执行。
* 协程的本地数据在后续调用中始终保持。
* 协程在控制离开时暂停执行，当控制再次进入时只能从离开的位置继续执行。

####协程可以被认为是一种用户空间线程，与传统的抢占式线程相比，有2个主要的优点：
* 与线程不同，协程是自己主动让出CPU，并交付他期望的下一个协程运行，而不是在任何时候都有可能被系统调度打断。因此协程的使用更加清晰易懂，并且多数情况下不需要锁机制。
* 与线程相比，协程的切换发生由程序控制，发生在用户空间而非内核空间，因此切换的代价非常的小。

####green化
术语“green化”来自于python下著名的协程库greenlet，指改造IO对象以能和协程配合。某种意义上，协程与线程的关系类似与线程与进程的关系，多个协程会在同一个线程的上下文之中运行。因此，当出现IO操作的时候，为了能够与协程相互配合，只阻塞当前协程而非整个线程，需要将io对象“green化”。目前orchid提供的green化的io对象包括：

* tcp socket（还不支持udp）
* descriptor（目前仅支持非文件类型文件描述符，如管道和标准输入，文件类型的支持会在以后版本添加）
* timer 
* signal

####chan：协程间通信
chan这个概念引用自golang的chan。每个协程是一个相互独立的执行单元，为了能够方便协程之间的通信/同步，orchid提供了chan这种机制。chan本质上是一个阻塞消息队列，后面我们将看到，chan不仅可以用于同一个调度器上的协程之间的通信，而且可以用于不同调度器上的协程之间的通信。


##预备知识
orchid的实现严重依赖于boost，依赖的主要子库包括：boost.context boost.asio boost.iostreams shared_ptr boost.bind 等等。如果用户对这些子库，尤其是boost.asio和boost.bind、shared_ptr具有一定的了解的话，会更加有利于了解和使用orchid。当然如果不了解也没有关系，本文会在后面的例子中对涉及的相关知识进行简单的介绍。

##编译与安装
暂无


#第一个栗子:一个复杂一些的hello world
国际惯例，让我们从hello world开始。在这个例子中，我们将看到如何创建和执行一个协程。
    
    //函数签名为 void(orchid::coroutine_handle) 的函数
    void f(orchid::coroutine_handle co) {
        std::cout<<"f:hello world 1"<<std::endl;
    }

    //函数签名为 void(orchid::coroutine_handle,const char*) 的函数
    void f1(orchid::coroutine_handle co,const char* str) {
        std::cout<<str<<std::endl;
    }

    //函数签名为 void(orchid::coroutine_handle) 的仿函数。
    struct printer {
        printer(const std::string& s):str(s) {

        }
        void operator()(orchid::coroutine_handle co) {
            std::cout<<str<<std::endl;
        }
        std::string str;
    };

    int main(int argc,char* argv[]) {
        orchid::scheduler sche;
        printer f2("f2:hello world");
        sche.spawn(f,orchid::coroutine::minimum_stack_size());
        sche.spawn(boost::bind(f1,_1,"f1:hello world"),orchid::coroutine::default_stack_size());
        sche.spawn(f2);
        sche.run();
        std::cout<<"done!"<<std::endl;
    }

程序的输出：

    f:hello world
    f1:hello world
    f2:hello world
    done!

在这个例子中，我们首先声明一个调度器sche,然后调用sche的spawn方法以3种方式创建了3个协程来输出hello world，最后调用调度器的run方法来执行整个程序。当程序执行时，3个协程依次被创建和执行。需要注意的是，在调用run方法之前，被创建的协助程序并不会被执行，只有调用了run方法之后，被创建的协程才会被调度执行。调用run方法的线程会被阻塞，直到所有的协程都执行完毕或调度器的stop方法被调用为止。（实际上，在这个例子中我们并没有对std::cout green化，因此每次调用std::cout的时候，整个调度器/线程都会被阻塞，在后面的介绍中我们将看到如何将IO对象green化）。

spawn方法有2个参数：

    template <typename F>
    void spawn(const F& func,std::size_t stack_size = coroutine_type::default_stack_size()) 

第一个参数func是协程被执行时所执行的函数。可以认为是协程的main函数。func可以是任何符合函数签名 

    void(orchid::coroutine_handle) 

的函数或函数对象，如上面例子中的f 和 f2。其中，orchid::coroutine_handle是协程的句柄，代表了func本身所处的协程。
往往要完成协程所执行的任务，仅仅有orchid::coroutine_handle是不够的，比如函数f1，f1需要额外传入一个const char* 的参数，对于这种函数，我们可以通过boost.bind、std::bind(tr1或者C++11)、或者lambda表达式（c++11）将他们适配成要求的函数类型。如上例中的：

    sche.spawn(boost::bind(f1,_1,"f1:hello world"),orchid::coroutine::default_stack_size())

boost::bind将f1从void(orchid::coroutine,const char*)适配成了void(orchid::coroutine_handle)类型。其中_1是占位符，表示空出f1的第一个参数，而f1的第二个参数由"f1:hello world"填充。在后面的例子中将会大量出现这种用法，如果用户对boost.bind并不是很清楚，可以先阅读一下boost文档中的相关章节。

第二个参数指定了协程栈空间的大小，orchid::coroutine提供了3个相关的函数：

    std::size_t default_stack_size();//默认的栈大小，一般为16个内存页
    std::size_t minimum_stack_size();//栈的最小值，一般为2个内存页面
    std::size_t maximum_stack_size();//栈的最大值，一般与进程的栈的最大值（软限制）相同。

用户可以根据自己的情况指定所需栈空间的大小。当协程使用的栈空间超过了提供的栈空间的大小的时候，程序会异常结束， orchid并不会自动增加栈空间的大小，所以用户必须预估出足够的栈空间。

这个hello world的例子过于简单，下面我们将通过创建一个echo server的客户端和服务器端来进一步了解orchid。



#第二个栗子:echo server
第二个栗子，让我们从网络编程届的hello world：echo server开始。echo server首先必须要处理连接事件，在orchid中，我们创建一个协程来专门处理连接事件，处理连接事件的协程的main函数如下：

    typedef boost::shared_ptr<orchid::socket> socket_ptr;

    //处理ACCEPT事件的协程
    void handle_accept(orchid::coroutine_handle co) {
        try {
            orchid::acceptor acceptor(co -> get_scheduler().get_io_service());
            acceptor.bind_and_listen("5678",true);
            for(;;) {
                socket_ptr sock(new orchid::socket(co -> get_scheduler().get_io_service()));
                acceptor.accept(*sock,co);
                co -> get_scheduler().spawn(boost::bind(handle_io,_1,sock),orchid::minimum_stack_size());
            }
        }
        catch(boost::system::system_error& e) {
            cerr<<e.code()<<" "<<e.what()<<endl;
        }
    }

在上面的代码中，我们创建了一个green化的acceptor，并让它监听5678端口，然后在"阻塞"等待连接到来，当连接事件发生时，创建一个新的协程来服务新得到的socket。green化的socket被包裹在智能指针中以参数形式传递给处理socket io事件的协程。处理套接字IO的协程的main函数如下：

    //处理SOCKET IO事件的协程
    void handle_io(orchid::coroutine_handle co,socket_ptr sock) {
        orchid::tcp_ostream out(*sock,co);
        orchid::tcp_istream in(*sock,co);
        for(std::string str;std::getline(in, str) && out;)
        {
            out<<str<<endl;
        }
      
    }

orchid可以使用户以流的形式来操作套接字;协程首先在传入的套接字上创建了一个输入流和一个输出流，分别代表了TCP的输入和输出。然后从输入流中读取一行，并输出到输出流当中。当socket上的TCP连接断开时，输入流和输出流的eof标志为会被置位。

最后是main函数：

    int main() {
        orchid::scheduler sche;
        sche.spawn(handle_accept,orchid::coroutine::minimum_stack_size());//创建协程
        sche.run();
    }

然后我们来看客户端的代码,首先是处理socket io的协程：

    void handle_io(orchid::coroutine_handle co) {
        orchid::descriptor stdout(co -> get_scheduler().get_io_service(),STDOUT_FILENO);
        orchid::socket sock_(co -> get_scheduler().get_io_service());
        try {
            sock_.connect("127.0.0.1","5678",co);
            orchid::tcp_istream in(sock_,co);
            orchid::tcp_ostream out(sock_,co);
            orchid::descriptor_ostream console(stdout,co);
            out << "hello world !!!!" <<endl;
            for (string str;std::getline(in,str);) {
                console << str << endl;
                out << "hello world !!!!" <<endl;
            }
        } catch (const boost::system::system_error& e) {
            cerr<<e.code()<<" "<<e.what()<<endl;
        }
    }

处理socket io的协程分别创建了一个green化的socket和一个green话的标准输出，然后连接到echo server上，不断执行 输出 -> 接收 -> 打印 这个流程。 

为了能够从外部打断client的执行，我们还需要一个协程来处理中断信号：

    void handle_sig(orchid::coroutine_handle co) {
        orchid::signal sig(co -> get_scheduler().get_io_service());
        try {
            sig.add(SIGINT);
            sig.add(SIGTERM);
            sig.wait(co);
            co->get_scheduler().stop();

        } catch (const boost::system::system_error& e) {
            cerr<<e.code()<<" "<<e.what()<<endl;
        }
    }

在这个协程中，协程“阻塞”在SIGINT 和 SIGTERM信号上，当信号发生时，调用调度器的stop方法来中断程序的执行，并安全的回收资源。

    int main() {
        orchid::scheduler sche;
        sche.spawn(handle_sig,orchid::coroutine::minimum_stack_size());
        for (int i=0;i<100;++i) {
            sche.spawn(handle_io);
        }
        sche.run();
    }

在客户端的main函数中，我们创建100个协程，同时向服务器发送请求。

在上面这个echo server的例子中，我们采用了一种 coroutine per connection 的编程模型，与传统的 thread per connection 模型一样的简洁清晰，但是整个程序实际上运行在同一线程当中。

由于协程的切换和内存开销远远小于线程，因此我们可以轻易的同时启动上千协程来同时服务上千连接，这是 thread per connection的模型很难做到的；

在性能方面，整个green化的IO系统实际上是使用boost.asio这种高性能的异步io库实现的,与原始的asio相比，orchid的性能损耗非常小，基本持平。

因此通过orchid，我们可以在保持同步IO模型简洁性的同时，获得近似于异步IO模型的高性能。


#第三个栗子:生产者-消费者
在这个例子中，我们将主要介绍orchid提供的协程间的通信机制：chan。chan这个概念引用自golang的chan。


#第四个栗子:chat server



#第五个栗子:benchmark


