#orchid简介

##什么是orchid?
orchid是一个构建于boost库基础上的C++库，类似于python下的gevent/eventlet，为用户提供基于协程的并发模型。

####什么是协程：
协程，即协作式程序，其思想是，一系列互相依赖的协程间依次使用CPU，每次只有一个协程工作，而其他协程处于休眠状态。协程在yield后暂停执行，当resume后从离开的位置继续执行。
协程已经被证明是一种非常有用的程序组件，不仅被python、lua、ruby等脚本语言广泛采用，而且被新一代面向多核的编程语言如golang rust-lang等采用作为并发的基本单位。

####协程可以被认为是一种用户空间线程，与传统的抢占式线程相比，有2个主要的优点：
* 与线程不同，协程是自己主动让出CPU，并交付他期望的下一个协程运行，而不是在任何时候都有可能被系统调度打断。因此协程的使用更加清晰易懂，并且多数情况下不需要锁机制。
* 与线程相比，协程的切换由程序控制，发生在用户空间而非内核空间，因此切换的代价非常的小。
* 某种意义上，协程与线程的关系类似与线程与进程的关系，多个协程会在同一个线程的上下文之中运行。

####green化
术语“green化”来自于python下著名的协程库greenlet，指改造IO对象以能和协程配合。
当出现IO操作的时候，为了能够与协程相互配合，只阻塞当前协程而非整个线程，需要将io对象“green化”。目前orchid提供的green化的io对象包括：

* tcp socket（还不支持udp）
* descriptor（目前仅支持非文件类型文件描述符，如管道和标准输入/输出）
* timer (定时器)
* signal (信号)

green化的io对象实际上是boost.asio中对应io对象的子类。因此依然可以调用asio中的各种方法（参见asio/socket.hpp）

####chan：协程间通信
chan这个概念引用自golang的chan。每个协程是一个独立的执行单元，为了能够方便协程之间的通信/同步，orchid提供了chan这种机制。chan本质上是一个阻塞消息队列，后面我们将看到，chan不仅可以用于同一个调度器上的协程之间的通信，而且可以用于不同调度器上的协程之间的通信。

####多核
建议使用的scheduler per cpu的的模型来支持多核的机器，即为每个CPU核心分配一个调度器，有多少核心就创建多少个调度器。不同调度器的协程之间也可以通过chan来通信。协程应该被创建在那个调度器里由用户自己决定。


##预备知识
orchid的实现严重依赖于boost，依赖的主要子库包括：context asio shared_ptr bind function等等。如果用户对这些子库，尤其是asio和bind、function、shared_ptr具有一定的了解的话，会更加有利于了解和使用orchid。当然如果不了解也没有关系，本文会在后面的例子中对涉及的相关知识进行简单的介绍。

##编译与安装
orchid 本身是个只包含头文件的模板库，拷贝到指定的目录即可，但是orchid依赖的boost库需要编译。而且在使用orchid时需要链接 boost_context boost_iostreams boost_system boost_thread 等子库（参见unit_test里的CMakeLists.txt）。

boost库的版本需要至少1.53。

    git clone https://github.com/ioriiod0/orchid.git
    cd orchid
    cp -r orchid <安装路径>


#例子
##第一个栗子:一个复杂一些的hello world
国际惯例，让我们从hello world开始。在这个例子中，我们将看到如何创建和执行一个协程。
    
    //函数签名为 void(orchid::coroutine_handle) 的函数
    void f(orchid::coroutine_handle co) {
        std::cout<<"f:hello world"<<std::endl;
    }

    //函数签名为 void(orchid::coroutine_handle,const char*) 的函数
    void f1(orchid::coroutine_handle co,const char* str) {
        std::cout<<str<<std::endl;
    }

    void stop(orchid::coroutine_handle co) {
        co -> get_scheduler().stop();
    }

    int main(int argc,char* argv[]) {
        orchid::scheduler sche;
        sche.spawn(f,orchid::coroutine::minimum_stack_size());
        sche.spawn(boost::bind(f1,_1,"f1:hello world"),orchid::coroutine::default_stack_size());
        sche.spawn(stop);
        sche.run();
        std::cout<<"done!"<<std::endl;
    }

程序的输出：

    f:hello world
    f1:hello world
    done!

在这个例子中，我们首先声明一个调度器sche,然后调用sche的spawn方法以2种方式创建了2个协程来输出hello world，最后调用调度器的run方法来执行整个程序。当程序执行时，2个协程依次被执行。需要注意的是，在调用run方法之前，被创建的协助程序并不会被执行，只有调用了run方法之后，被创建的协程才会被调度执行。
调用run方法的线程会被阻塞，直到调度器的stop方法被调用为止。
（实际上，在这个例子中我们并没有对std::cout进行green化，因此每次调用std::cout的时候，整个调度器/线程都会被阻塞，在后面的介绍中我们将看到如何将IO对象green化）。

spawn方法有2个参数：

    template <typename F>
    void spawn(const F& func,std::size_t stack_size = coroutine_type::default_stack_size()) 

第一个参数func是协程被执行时所执行的函数。可以认为是协程的main函数。func可以是任何符合函数签名 

    void(orchid::coroutine_handle) 

的函数或函数对象，如上面例子中的f。其中，orchid::coroutine_handle是协程的句柄，代表了func本身所处的协程。
往往要完成协程所执行的任务，仅仅有orchid::coroutine_handle是不够的，比如函数f1，f1需要额外传入一个const char* 的参数，对于这种函数，我们可以通过boost.bind、std::bind(tr1或者C++11)、或者lambda表达式（c++11）将他们适配成要求的函数类型。如上例中的：

    sche.spawn(boost::bind(f1,_1,"f1:hello world"),orchid::coroutine::default_stack_size())

boost::bind将f1从void(orchid::coroutine,const char*)适配成了void(orchid::coroutine_handle)类型。其中_1是占位符，表示空出f1的第一个参数，而f1的第二个参数由"f1:hello world"填充。在后面的例子中将会大量出现这种用法，如果用户对boost.bind并不是很清楚，可以先阅读一下boost文档中的相关章节。

第二个参数指定了协程栈空间的大小，orchid::coroutine提供了3个相关的函数：

    std::size_t default_stack_size();//默认的栈大小，一般为16个内存页
    std::size_t minimum_stack_size();//栈的最小值，一般为2个内存页面
    std::size_t maximum_stack_size();//栈的最大值，一般与进程的栈的最大值（软限制）相同。

用户可以根据自己的情况指定所需栈空间的大小。当协程使用的栈空间超过了提供的栈空间的大小的时候，程序会异常结束， orchid并不会自动增加栈空间的大小，所以用户必须预估出足够的栈空间。

这个hello world的例子过于简单，下面我们将通过创建一个echo server的客户端和服务器端来进一步了解orchid。


##第二个栗子:echo server
第二个栗子，让我们从网络编程届的hello world：echo server开始（完整代码参见unit_test/client.cpp和unit_test/server.cpp）。echo server首先必须要处理连接事件，在orchid中，我们创建一个协程来专门处理网络链接事件：

    typedef boost::shared_ptr<orchid::socket> socket_ptr;

    //处理ACCEPT事件的协程
    void handle_accept(orchid::coroutine_handle co) {
        try {
            orchid::acceptor acceptor(co -> get_io_service());
            acceptor.bind_and_listen("5678",false);
            for(;;) {
                socket_ptr sock(new orchid::socket(co -> get_io_service()));
                acceptor.accept(*sock,co);
                co -> get_scheduler().spawn(boost::bind(handle_io,_1,sock));
            }
        }
        catch(orchid::io_error& e) {
            ORCHID_ERROR("id %lu msg:%s",co->id(),e.what());
        }
    }

在上面的代码中，我们创建了一个green化的acceptor，并让它监听5678端口，然后在"阻塞"等待连接到来，当连接事件发生时，创建一个新的协程来服务新得到的socket。green化的socket被包裹在智能指针中以参数形式传递给处理socket io事件的协程。用于处理套接字IO的协程如下：

    //处理SOCKET IO事件的协程
    void handle_io(orchid::coroutine_handle co,socket_ptr sock) {
    orchid::buffered_reader<orchid::socket> reader(*sock,co,16);
    orchid::buffered_writer<orchid::socket> writer(*sock,co,16);

    try {
        std::string line;
        std::size_t n = 0;

        for(;;) {
            n = reader.read_until(line,'\n');
            ORCHID_DEBUG("id %lu recv: %s",co->id(),line.c_str());
            writer.write(line.c_str(),line.size());
            writer.flush();
        }

    } catch (const orchid::io_error& e) {
        if (e.code() == boost::asio::error::eof) {
            ORCHID_DEBUG("id %lu msg:%s",co->id(),"socket closed by remote side!");
        } else {
            ORCHID_ERROR("id %lu msg:%s",co->id(),e.what());
        }
    }

}

协程首先在传入的套接字上创建了一个输入流和一个输出流，分别代表了TCP的输入和输出。然后不断地从输入流中读取一行，并输出到输出流当中。当socket上的TCP连接断开时，会抛出orchid::io_error的异常，循环结束，值得注意的是eof事件也被当成异常来抛出。对于不喜欢使用异常的用户，orchid提供了另外一套使用boost::system::error_code的接口（参见asio/io_interface.hpp）。同时，对于熟悉asio的用户，orchid提供了一套boost asio风格的接口（参见asio/io_funcs.hpp）。

最后是main函数：

    int main() {
        orchid::scheduler sche;
        sche.spawn(handle_accept,orchid::coroutine::minimum_stack_size());//创建协程
        sche.run();
    }


然后我们来看客户端的代码，在客户端中，我们创建100个并发的TCP连接不断的向echo server发送hello world。

首先是用于处理socket的协程：

    void handle_io(orchid::coroutine_handle co) {
        orchid::socket sock_(co -> get_io_service());
        std::size_t n = 0;

        try {
            sock_.connect("127.0.0.1","5678",co);
            orchid::buffered_reader<orchid::socket> reader(sock_,co,16);
            orchid::buffered_writer<orchid::socket> writer(sock_,co,16);

            std::string line("hello world!\r\n");
            for(;;) {
                writer.write(line.c_str(),line.size());
                writer.flush(); //带缓冲的输出流，不要忘记flush。
                n = reader.read_until(line,"\r\n");
            }

        } catch (const orchid::io_error& e) {
            ORCHID_ERROR("id %lu msg:%s",co->id(),e.what());
            sock_.close();
        }
    }

处理socket io的协程创建了一个green化的socket，然后连接到echo server上，不断执行 (输出 -> 接收) 这个流程。 

为了能够从外部打断client的执行，我们还需要一个协程来处理中断信号，这样我们就可以用ctrl+c来正确的中断程序的执行：

    void handle_sig(orchid::coroutine_handle co) {
        orchid::signal sig(co -> get_scheduler().get_io_service());
        try {
            sig.add(SIGINT);
            sig.add(SIGTERM);
            sig.wait(co);
            co->get_scheduler().stop();

        } catch (const orchid::io_error& e) {
            ORCHID_ERROR("id %lu msg:%s",co->id(),e.what());
        }
    }

在这个协程中，协程“阻塞”在SIGINT 和 SIGTERM信号上，当信号发生时，调用调度器的stop方法来中断程序的执行，并安全的回收资源。

最后是客户端的main函数：

    int main() {
        orchid::scheduler sche;
        sche.spawn(handle_sig,orchid::coroutine::minimum_stack_size());
        for (int i=0;i<100;++i) {
            sche.spawn(handle_io);
        }
        sche.run();
    }

在客户端的main函数中，我们创建100个协程，同时向服务器发送hello world。

在上面这个echo server的例子中，我们采用了一种 coroutine per connection 的编程模型，与传统的 thread per connection 模型一样的简洁清晰，但是整个程序实际上运行在同一线程当中。
由于协程的切换和内存开销远远小于线程，因此我们可以轻易的同时启动上千协程来同时服务上千连接，这是 thread per connection的模型很难做到的；
在性能方面，整个green化的IO系统实际上是使用boost.asio这种高性能的异步io库实现的,与原始的asio相比，orchid的性能损耗非常小，性能基本持平。
因此通过orchid，我们可以在保持同步IO模型简洁性的同时，获得近似于异步IO模型的高性能。


##第三个栗子:生产者-消费者
在这个例子中，我们将主要介绍orchid提供的协程间的通信机制：chan。chan这个概念引用自golang的chan。chan表现为一个阻塞消息队列。

在下面的例子中，代码orchid::chan < int > ch(10) 表示创建一个大小为10，装载类型为int的chan。
chan 有3个重要的接口：

    // 向chan中发送一个对象。t为要发送的对象，co为当前协程的协程句柄。
    // 当队列为空时，co会阻塞，当队列不在为空时，阻塞的协程会被唤醒；
    // 发送成功返回true，否则false。
    bool send(const U& t,coroutine_pointer co); 

    // 从chan中接收一对象。
    // 当队列满时，co会阻塞，当队列不再满时，阻塞的协程会被唤醒。
    // 接收成功返回true，否则false。
    bool recv(U& t,coroutine_pointer co); 

    // 关闭一个chan，当chan关闭后，再向chan里发送数据均会失败。但此时残留在chan里的数据依旧可读，当最后一个数据被读出后，recv会返回失败。
    void close(); 

下面是一个简单的生产者和消费者的例子：

    //生产者，不断发送自己的ID给消费者
    void sender(orchid::coroutine_handle co,int id,orchid::chan<int>& ch) {
        orchid::descriptor stdout(co -> get_io_service(),::dup(STDOUT_FILENO)); //green化的标准输出
        orchid::writer<orchid::descriptor> console(stdout,co);
        char buf[128] = {0};

        for (;;) {
            ch.send(id,co);
            int n = sprintf(buf,"sender %d send:%d\r\n",id,id);
            console.write(buf,n);
        }
    }

    //消费者，不断接收生产者发送的ID并打印ID.
    void receiver(orchid::coroutine_handle co,int id,orchid::chan<int>& ch) {
        orchid::descriptor stdout(co -> get_io_service(),::dup(STDOUT_FILENO));
        orchid::writer<orchid::descriptor> console(stdout,co);
        int sender = 0;
        char buf[128] = {0};

        for (;;) {
            ch.recv(sender,co);
            int n = sprintf(buf,"receiver %d receive:%d\r\n",id,sender);
            console.write(buf,n);
        }
    }

    //生产者和消费者运行在同一个调度器/线程中。
    int main(int argc,const char* argv[]) {
        orchid::scheduler sche;
        orchid::chan<int> ch(10);
        for (int i=0;i<100;++i) {
            sche.spawn(boost::bind(sender,_1,i,boost::ref(ch)));
        }
        sche.spawn(boost::bind(receiver,_1,boost::ref(ch)));
        sche.run();
    }

    //生产者和消费者运行在不同的调度器/线程中。
    int main(int argc,const char* argv[]) {
        orchid::scheduler_group group(2);
        orchid::chan<int> ch(10);
        for (int i=0;i<100;++i) {
            group[i%2].spawn(boost::bind(sender,_1,i,boost::ref(ch)));
        }
        group[0].spawn(boost::bind(receiver,_1,boost::ref(ch)));
        group.run();
    }

通过scheduler_group类我们可以方便的创建一组调度器，每个调度器运行在一个单独的线程中。可以通过下标来访问某个调度器；通过调用其run方法同时启动多个调度器；通过调用其stop方法，同时停止多个调度器。

##第四个例子：worker & worker pool
有时候我们需要运行一些长时间的任务，比如长时间的io或者大量的计算任务，但是又不想阻塞掉整个调度器。一种做法是将这些任务移动到工作线程中去做，然后当前协程睡眠，让其他协程运行，当任务做完后，再唤醒那个协程。orchid提供了这种机制：

    void test(orchid::coroutine_handle co) {
        int a = 1,b = 2;
        int c;
        orchid::run_in_thread([a,b,&c](){
            boost::this_thread::sleep(boost::posix_time::seconds(3));
            c = a+b;
        },co);

        ORCHID_DEBUG("c:%d",c);
    }

    int main(int argc,const char* argv[]) {
        orchid::scheduler sche;
        orchid::worker_pool pool(1);
        boost::thread t([&pool](){ pool.run(); });

        sche.spawn(test);
        sche.spawn([&pool](orchid::coroutine_handle co){
            int a = 1,b = 2;
            int c;
            pool.post([a,b,&c](){
                boost::this_thread::sleep(boost::posix_time::seconds(3));
                c = a+b;
            },co);
            ORCHID_DEBUG("c:%d",c);
        });

        sche.run();
    }

##第五个栗子:chat server
这次我们来一个复杂一些的例子：chat client和一个多线程的chat server 。从这个例子中我们将看到一些有用的技巧，比如如何使用boost::shared_from_this来管理协程间共享对象的生命周期；如何利用boost.variant在一个chan中接收多种类型消息。


先从较为简单的chat clent开始：在chat client中我们将创建两个协程，一个不断从本机的标准输入读取输入，然后发送到chat server；另一个则不断从chat server接受消息并发送到本机的标准输出上。

    const static std::size_t STACK_SIZE = 64*1024;

    //客户端类
    class chat_client {
    public:
        chat_client(const string& ip,const string& port)
            :sche_(),
            stdin_(sche_.get_io_service(),STDIN_FILENO),//green化的标准输入
            stdout_(sche_.get_io_service(),STDOUT_FILENO),//green化的标准输出
            sock_(sche_.get_io_service()),
            is_logined_(false),ip_(ip),port_(port) {

        }
        ~chat_client() {

        }
    public:
        //创建协程并启动调度器
        void run() {
            sche_.spawn(boost::bind(&chat_client::handle_console,this,_1),STACK_SIZE);
            sche_.run();
        }
        //停止调度器
        void stop() {
            sche_.stop();
        }

    private:
        // 不断从chat server接收消息并打印到标准输出上。
        void handle_msg(orchid::coroutine_handle co) {
            try {
                sock_.connect(ip_,port_,co);
            } catch (const orchid::io_error& e) {
                ORCHID_ERROR("err msg:%s",e.what());
                return;
            }

            orchid::writer<orchid::descriptor> out(stdout_,co);
            orchid::buffered_reader<orchid::socket> in(sock_,co);

            string line;
            try {
                for (;;) {
                    in.read_until(line,'\n');
                    out.write_full(line.c_str(),line.size());
                }
            } catch (const orchid::io_error& e) {
                ORCHID_ERROR("err msg:%s",e.what());
            }
            
        }

        //不断从标准输入接收用户输入，并处理用户输入，发送消息chat server。
        //登陆用 /l username
        //发送消息用 /s xxxxxxx
        //退出用 /q
        void handle_console(orchid::coroutine_handle co) {
            orchid::buffered_reader<orchid::descriptor> in(stdin_,co);
            orchid::buffered_writer<orchid::socket> out(sock_,co);

            string str;
            char buf[1024] = {0};
            try {
                for(;;) {
                    in.read_until(str,'\n');
                    ORCHID_DEBUG("read from stdin:%s",str.c_str());

                    if(str.empty()) continue;
                    // 退出 /q
                    if(str.size() >= 2 && str[0] == '/' && str[1] == 'q') { 
                        sock_.close();
                        user_.clear();
                        is_logined_ = false;
                        ORCHID_ERROR("closed!");
                        stop();
                    }
                    // 发送消息 /s message
                    else if(str.size() >= 4 && str[0] == '/' && str[1] =='s') {
                        if(!is_logined_) {
                            ORCHID_ERROR("please login first!");
                        } else {
                            int n = sprintf(buf,"%s:%s\r\n",user_.c_str(),str.c_str()+3);
                            out.write(buf,n);
                            out.flush();
                            ORCHID_DEBUG("send:%s n:%d",buf,n);
                        }
                    }
                    // 登陆 /l username 
                    else if(str.size() >= 4 && str[0] == '/' && str[1] == 'l') {
                        if (!is_logined_) {
                            user_.assign(str.begin()+3,str.end()-1);
                            is_logined_ = true;
                            ORCHID_DEBUG("user:%s",user_.c_str());
                        } else {
                            ORCHID_ERROR("already logined!");
                        }
                    } else {
                        print_err();
                    }
                }
            } catch (const orchid::io_error& e) {
                ORCHID_ERROR("err:%s",e.what());
            }

        }

        void print_err() {
            ORCHID_ERROR("usage:\r\nlogin: /l name\r\nexit: /q\r\nsend: /s ooxx");
        }

    private:
        orchid::scheduler sche_;
        orchid::descriptor stdin_;
        orchid::descriptor stdout_;
        orchid::socket sock_;
        string user_;
        bool is_logined_;
        string ip_;
        string port_;

    };

    int main(int argc,char* argv[]) {
        string ip = argv[1];
        string port = argv[2];
        chat_client client(ip,port);
        client.run();
    }

在上面的代码中，我们利用了boost.bind的强大能力，使用类的成员函数创建了协程。


然后是chat server:

chat server中有2个类，server和client。类server实现了chat server的主要逻辑，类client_agent则是客户端代理类，负责从客户端处接收消息以及向客户端发送数据。

server类的职责包括：维护客户端列表，广播某个客户端的发来的消息。
因此server处理的消息有2类，第一类是控制消息（下面代码中的ctrl_t类型），代表了客户端的到来和离开事件；
另外一类是文本消息，代表客户端发送的消息。不管是处理第一种类型的消息还是处理第二种类型的消息，都需要访问到其内部维护的客户端列表。为了同步这些访问，我们需要在同一个chan中接收这两种消息。

    const static std::size_t STACK_SIZE = 64*1024;
    
    template <typename Client>
    struct server {
        ///////////////typedef/////////////
        typedef server<Client> self_type;
        typedef boost::shared_ptr<Client> client_sp_type; 
        typedef std::list<client_sp_type> client_list_type;// 客户端列表类型，
                                                            // 保存的是智能指针。
        enum {REGISTER,UNREGISTER};
        struct ctrl_t {
            int cmd_;
            client_sp_type client_;
        };
        typedef boost::variant<string,ctrl_t> msg_type;//消息类型

        //////////////////变量///////////
        orchid::scheduler_group schedulers_;
        orchid::acceptor acceptor_;
        std::string port_;
        orchid::chan<msg_type> msg_ch_;//server的消息队列
        client_list_type clients_;//客户端列表
        ///////////////////////////

        server(std::size_t size,const string& port)
            :schedulers_(size),acceptor_(schedulers_[0].get_io_service()),port_(port),msg_ch_(512) {
        }
        ~server() {
        }

        //process_msg是消息处理协程。不断的从消息队列中读取消息，然后判断消息类型，并处理。
        void process_msg(orchid::coroutine_handle co) {
            for (;;) {
                msg_type msg;
                if (!msg_ch_.recv(msg,co)) {
                    ORCHID_DEBUG("chan closed");
                    break;
                }

                switch (msg.which()) {
                    case 0:
                        ORCHID_DEBUG("process msg:%s",boost::get<string>(msg).c_str());
                        for(typename client_list_type::iterator it = clients_.begin(); it != clients_.end(); ++it) {
                            (*it) -> ch_.send(boost::get<string>(msg),co);
                        }
                        break;
                    case 1:
                        if(boost::get<ctrl_t>(msg).cmd_ == REGISTER) {
                            ORCHID_DEBUG("on register");
                            boost::get<ctrl_t>(msg).client_->start();
                            clients_.push_back(boost::get<ctrl_t>(msg).client_);
                        } else if(boost::get<ctrl_t>(msg).cmd_ == UNREGISTER) {
                            ORCHID_DEBUG("on unregister");
                            boost::get<ctrl_t>(msg).client_->close();
                            clients_.remove(boost::get<ctrl_t>(msg).client_);
                        } else {
                            throw std::runtime_error("unkonw cmd! should never hanppened!");
                        }
                        break;
                    default:
                        throw std::runtime_error("unkonw msg! should never hanppened!");
                }
            }
        }

        //处理连接到来事件。当连接到来时，发送表示注册的控制消息到消息队列中。
        void accept(orchid::coroutine_handle co) {
            try {
                int index = 0;
                acceptor_.bind_and_listen(port_,true);
                for (;;) {
                    if(index >= schedulers_.size()) index = 0;
                    boost::shared_ptr<Client> c(new Client(schedulers_[index++],*this));
                    acceptor_.accept(c->sock_,co);
                    ORCHID_DEBUG("on accept");
                    ctrl_t msg;
                    msg.cmd_ = REGISTER;
                    msg.client_ = c;
                    msg_ch_.send(msg,co);
                }
            } catch (boost::system::system_error& e) {
                ORCHID_ERROR("err:%s",e.what());
            }
        }

        //启动一个主调度器和一个调度器组。主调度器负责处理消息以及接受连接，调度器组用来处理io请求。
        void run() {
            main_sche_.spawn(boost::bind(&self_type::accept,this,_1));
            main_sche_.spawn(boost::bind(&self_type::process_msg,this,_1));
            boost::thread t(boost::bind(&orchid::scheduler_group::run,&schedulers_));
            main_sche_.run();
            t.join();
        }

    };

客户端代理类的主要职责是：接收客户端socket上的数据，并发送到server类中；接收server发来的消息，并发送到客户端socket中；当客户端断开连接时，通知server。
不难看出，一个客户端代理类对象会同时被3个协程访问到：server的消息处理协程，client的sender和receiver协程。为了安全的释放客户端代理类的对象，我们使用引用计数的方案。引用计数通过boost::enable_shared_from_this来实现。

    //客户端代理类。
    struct client:public boost::enable_shared_from_this<client> {
        orchid::scheduler& sche_;
        server<client>& server_;
        orchid::socket sock_;
        orchid::chan<string> ch_;

        client(orchid::scheduler& sche,server<client>& s)
            :sche_(sche),server_(s),
            sock_(sche_.get_io_service()),ch_(32) {

        }
        ~client() {
            
        }

        // 启动发送和接收协程。注意这个地方的 this -> shared_from_this()，
        // 启动的协程中保存了当前对象的智能指针。
        // 当协程结束后，智能指针会释放，并在引用计数为0时，释放当前对象。
        void start() {
             sche_.spawn(boost::bind(&client::sender,this -> shared_from_this(),_1),STACK_SIZE);
             sche_.spawn(boost::bind(&client::receiver,this -> shared_from_this(),_1),STACK_SIZE);
        }

        //不断从client的消息队列中接收消息，并通过socket发送。
        //chan被关闭后退出循环，协程结束。
        void sender(orchid::coroutine_handle co) {
            string str;
            orchid::buffered_writer<orchid::socket> out(sock_,co);
            try {
                while(ch_.recv(str,co)) {
                    out.write(str.c_str(),str.size());
                    out.flush();
                }
            } catch(const orchid::io_error& e) {
                ORCHID_ERROR("err:%s",e.what());
            }
            ORCHID_DEBUG("sender: exit!");
        }

        // 不断从客户端接收消息，直到客户端断开连接。
        // 当连接断开后，发送反注册信息到server的消息队列中。
        void receiver(orchid::coroutine_handle co) {
            orchid::buffered_reader<orchid::socket> in(sock_,co);
            string str;
            try {
                for (;;) {
                    in.read_until(str,'\n');
                    ORCHID_DEBUG("recv:%s",str.c_str());
                    server_.msg_ch_.send(str,co);
                }
            } catch(const orchid::io_error& e) {
                ORCHID_ERROR("err:%s",e.what());
            }

            server<client_agent>::ctrl_t ctrl_msg;
            ctrl_msg.cmd_ = server<client_agent>::UNREGISTER;
            ctrl_msg.client_ = this -> shared_from_this();
            server_.msg_ch_.send(ctrl_msg, co);
            ORCHID_DEBUG("receiver: exit!");
        }
    };

    int main(int argc,char* argv[]) {
        string port = argv[1];
        server<client> s(4,port);
        s.run();
    }

需要注意的是，对同一个套接字进行读写的协程应该建立在同一个调度器上。因为实际上从green化的IO对象的构造函数即可以看出:green化的IO对象是与调度器中io_service对象绑定的。当不同调度器上的协程访问同一个IO对象的时，orchid不能保证其行为的正确性。

