#简介

##什么是orchid?
orchid是一个构建于boost库基础上的C++库，类似于python下的gevent/eventlet，为用户提供基于协程的并发模型。

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


##预备知识
orchid的实现严重依赖于boost，依赖的主要子库包括：boost.context boost.asio boost.iostreams shared_ptr boost.bind 等等。如果用户对这些子库，尤其是boost.asio和boost.bind、shared_ptr具有一定的了解的话，会更加有利于了解和使用orchid。当然如果不了解也没有关系，本文会在后面的例子中对涉及的相关知识进行简单的介绍。

##编译与安装

orchid 本身是个只包含头文件的模板库，拷贝到指定的目录即可，但是orchid依赖的boost库需要编译。而且在使用orchid的时需要链接 boost_context boost_iostreams boost_system boost_thread 等子库（参见unit_test里的CMakeLists.txt）。

boost需要采用最新的svn里的版本，因为1.52及以下版本缺少boost.atomic等子库。

####MAC OS

    git clone https://github.com/ryppl/boost-svn
    cd boost-svn
    ./bootstrap.sh
    ./b2 toolset=clang cxxflags="-arch x86_64" linkflags="-arch x86_64" install
    cd ..
    git clone https://github.com/ioriiod0/orchid.git
    cd orchid
    cp -r orchid <安装路径>


####LINUX
    
    git clone https://github.com/ryppl/boost-svn
    cd boost-svn
    ./bootstrap.sh
    ./b2 install
    cd ..
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
第二个栗子，让我们从网络编程届的hello world：echo server开始。echo server首先必须要处理连接事件，在orchid中，我们创建一个协程来专门处理连接事件：

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

在上面的代码中，我们创建了一个green化的acceptor，并让它监听5678端口，然后在"阻塞"等待连接到来，当连接事件发生时，创建一个新的协程来服务新得到的socket。green化的socket被包裹在智能指针中以参数形式传递给处理socket io事件的协程。处理套接字IO的协程如下：

    //处理SOCKET IO事件的协程
    void handle_io(orchid::coroutine_handle co,socket_ptr sock) {
        orchid::tcp_ostream out(*sock,co);
        orchid::tcp_istream in(*sock,co);
        for(std::string str;std::getline(in, str) && out;)
        {
            out<<str<<endl;
        }
      
    }

协程首先在传入的套接字上创建了一个输入流和一个输出流，分别代表了TCP的输入和输出。然后不断地从输入流中读取一行，并输出到输出流当中。当socket上的TCP连接断开时，输入流和输出流的eof标志为会被置位，因此循环结束，协程退出。

orchid可以使用户以流的形式来操作套接字。输入流和输出流分别提供了std::istream和std::ostream的接口；输入流和输出流是带缓冲的，如果用户需要无缓冲的读写socket或者自建缓冲，可以直接调用orchid::socket的read和write函数。但是需要注意这两个函数会抛出boost::system_error异常来表示错误（参见benchmark_orchid_client和benchmar_orchid_server）。

最后是main函数：

    int main() {
        orchid::scheduler sche;
        sche.spawn(handle_accept,orchid::coroutine::minimum_stack_size());//创建协程
        sche.run();
    }

然后我们来看客户端的代码，在客户端中，我们创建100个并发的TCP连接不断的向echo server发送hello world。

首先是处理socket io的协程：

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

为了能够从外部打断client的执行，我们还需要一个协程来处理中断信号，这样我们就可以用ctrl+c来正确的中断程序的执行：

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

在客户端的main函数中，我们创建100个协程，同时向服务器发送hello world。

在上面这个echo server的例子中，我们采用了一种 coroutine per connection 的编程模型，与传统的 thread per connection 模型一样的简洁清晰，但是整个程序实际上运行在同一线程当中。
由于协程的切换和内存开销远远小于线程，因此我们可以轻易的同时启动上千协程来同时服务上千连接，这是 thread per connection的模型很难做到的；
在性能方面，整个green化的IO系统实际上是使用boost.asio这种高性能的异步io库实现的,与原始的asio相比，orchid的性能损耗非常小，性能基本持平。
因此通过orchid，我们可以在保持同步IO模型简洁性的同时，获得近似于异步IO模型的高性能。


##第三个栗子:生产者-消费者
在这个例子中，我们将主要介绍orchid提供的协程间的通信机制：chan。chan这个概念引用自golang的chan。chan表现为一个阻塞消息队列。
orchid提供的chan只支持 单生产者-单消费者 和 多生产者-单消费者 这两种模型（在其他模型，如多生产者-多消费者中，也可以工作，单可能会出现某些消费者饿死的现象）。

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

    // 关闭一个chan，当chan关闭后，再调用send和recv都会直接返回false；并且所有阻塞在chan中的协程都会被唤醒
    // 被唤醒的协程中 send/recv 返回false。
    void close(); 

下面是一个简单的生产者和消费者的例子：

    //生产者，不断发送自己的ID给消费者
    void sender(orchid::coroutine_handle co,int id,orchid::chan<int>& ch) {
        for (;;) {
            ch.send(id,co);
        }
    }
    //消费者，不断接收生产者发送的ID并打印ID.
    void receiver(orchid::coroutine_handle co,orchid::chan<int>& ch) {
        orchid::descriptor stdout(co -> get_scheduler().get_io_service(),STDOUT_FILENO);
        orchid::descriptor_ostream console(stdout,co);
        int id;
        for (;;) {
            ch.recv(id,co);
            console<<"receiver receive: "<<id<<std::endl;
        }
    }

    //生产者和消费者运行在同一个调度器中。
    void test_one_scheduler() {
        orchid::scheduler sche;
        orchid::chan<int> ch(10);
        for (int i=0;i<100;++i) {
            sche.spawn(boost::bind(sender,_1,i,boost::ref(ch)));
        }
        sche.spawn(boost::bind(receiver,_1,boost::ref(ch)));
        sche.run();
    }

    //生产者和消费者运行再不同的调度器中。
    void test_scheduler_group() {
        orchid::scheduler_group group(2);
        orchid::chan<int> ch(10);
        for (int i=0;i<100;++i) {
            group[i%2].spawn(boost::bind(sender,_1,i,boost::ref(ch)));
        }
        group[0].spawn(boost::bind(receiver,_1,boost::ref(ch)));
        group.run();
    }

通过scheduler_group类我们可以方便的创建一组调度器，每个调度器运行在一个单独的线程中。可以通过下标来访问某个调度器；通过调用其run方法同时启动多个调度器；通过调用其stop方法，同时停止多个调度器。


##第四个栗子:chat server
这次我们来一个复杂一些的例子：chat server 和 chat client。从这个例子中我们将看到一些有用的技巧，比如如何使用boost::shared_from_this来管理协程间共享对象的生命周期；如何利用boost.variant在一个chan中接收多种类型消息。


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
        void receive_msg(orchid::coroutine_handle co) {
            string str;
            orchid::descriptor_ostream out(stdout_,co);
            orchid::tcp_istream in(sock_,co);
            for (string str;std::getline(in, str);) {
                out<<str<<endl;
            }
        }

        //不断从标准输入接收用户输入，并处理用户输入，发送消息chat server。
        //登陆用 /l username
        //发送消息用 /s xxxxxxx
        //退出用 /q
        void handle_console(orchid::coroutine_handle co) {
            orchid::descriptor_istream in(stdin_,co);
            orchid::tcp_ostream out(sock_,co);
            //首先连接chat server
            try {
                sock_.connect(ip_,port_,co);
            } catch (boost::system::system_error& e) {
                cerr<<e.code()<<" "<<e.what()<<endl;
                return;
            }
            //连接成功则启动接受消息的协程。
            sche_.spawn(boost::bind(&chat_client::receive_msg,this,_1), STACK_SIZE);
            //不断读取标准输入并进行处理。
            for(string str;std::getline(in,str);) {
                if(str.empty()) continue;
                // 退出 “/q”
                if(str.size() >= 2 && str[0] == '/' && str[1] == 'q') { 
                    sock_.close();
                    user_.clear();
                    is_logined_ = false;
                    cerr<<"closed"<<endl;
                    stop();
                }
                // 发送消息 /s message
                else if(str.size() >= 4 && str[0] == '/' && str[1] =='s') {
                    if(!is_logined_) {
                        cerr<<"login first"<<endl;
                    } else {
                        out<<user_<<" : "<<str.substr(3)<<endl;
                    }
                }
                // 登陆 “/l username”
                else if(str.size() >= 4 && str[0] == '/' && str[1] == 'l') {
                    if (!is_logined_) {
                        user_ = str.substr(3);
                        is_logined_ = true;
                    } else {
                        cerr<<"err: already logined!"<<endl;
                    }
                } else {
                    print_err();
                }
            }

        }

        void print_err() {
               cerr<<"err: bad cmd!"<<endl
                <<"usage:"<<endl
                <<"login: /l username"<<endl
                <<"exit: /q"<<endl
                <<"send: /s xxxxxxxxxxxx"<<endl;
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

chat server中有2个类，server和client。类server实现了chat server的主要逻辑，类client则是客户端代理类，负责从客户端处接收和发送数据。

server类的职责包括：维护客户端列表，广播某个客户端的发来的消息。
因此server处理的消息有2类，第一类是控制消息（下面代码中的ctrl_t类型），代表了客户端的到来和离开事件；
另外一类是文本消息，server类需要向所有的客户端转发、广播该类消息。不管是处理第一种类型的消息还是处理第二种类型的消息，都需要访问到其内部维护的客户端列表。为了同步这些访问，我们需要在同一个chan中接收这两种消息。

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

        //handle_msg是消息处理协程。不断的从消息队列中读取消息，然后判断消息类型，并处理。
        void handle_msg(orchid::coroutine_handle co) {
            msg_type msg;
            for (;;) {
                msg_ch_.recv(msg,co);
                if(msg.which() == 0) {// 如果是string类型，即文本消息，则向所有的客户端代理广播。
                    for(typename client_list_type::iterator it = clients_.begin(); it != clients_.end(); ++it) {
                        //向客户端代理的chan中发送消息。
                        (*it) -> ch_.send(boost::get<string>(msg),co);
                    }
                } else if(msg.which() == 1) {//如果是ctrl_t类型，即控制消息，则修改客户端列表。
                    if(boost::get<ctrl_t>(msg).cmd_ == REGISTER) {//注册消息
                        clients_.push_back(boost::get<ctrl_t>(msg).client_);
                    } else if(boost::get<ctrl_t>(msg).cmd_ == UNREGISTER) {//反注册消息
                        clients_.remove(boost::get<ctrl_t>(msg).client_);
                    } else {
                        throw std::runtime_error("unkonw cmd! should never hanppened!");
                    }
                } else {
                    throw std::runtime_error("unkonw msg! should never hanppened!");
                }
            }
        }

        //处理连接到来事件。当连接到来时，发送表示注册的控制消息到消息队列中。
        void handle_accept(orchid::coroutine_handle co) {
        try {
            int index = 1;
            acceptor_.bind_and_listen(port_);
            for (;;) {
                if(index >= schedulers_.size()) index = 0;
                boost::shared_ptr<Client> c(new Client(schedulers_[index++],*this));
                acceptor_.accept(c->sock_,co);
                c -> start();
                ctrl_t msg;
                msg.cmd_ = REGISTER;
                msg.client_ = c;
                msg_ch_.send(msg,co);
            }
        } catch (boost::system::system_error& e) {
            cout<<e.code()<<" "<<e.what()<<endl;
        }
    }

        void run() {
            schedulers_[0].spawn(boost::bind(&self_type::handle_accept,this,_1),STACK_SIZE);
            schedulers_[0].spawn(boost::bind(&self_type::handle_msg,this,_1),STACK_SIZE);
            schedulers_.run();
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
        void sender(orchid::coroutine_handle& co) {
            string str;
            orchid::tcp_ostream out(sock_,co);
            while(ch_.recv(str,co)) {
                out<<str<<endl;
            }
        }

        // 不断从客户端接收消息，直到客户端断开连接。
        // 当连接断开后，关闭自身的chan并发送反注册信息到server的消息队列中。
        void receiver(orchid::coroutine_handle& co) {
            orchid::tcp_istream in(sock_,co);
            //客户端断开连接后会退出循环。
            for (string str;std::getline(in,str);) {
                //向server的chan中发送消息。
                server_.msg_ch_.send(str,co);
            }
            ch_.close();
            server<client>::ctrl_t ctrl_msg;
            ctrl_msg.cmd_ = server<client>::UNREGISTER;
            ctrl_msg.client_ = this -> shared_from_this();
            server_.msg_ch_.send(ctrl_msg, co);

        }
    };

    int main(int argc,char* argv[]) {
        string port = argv[1];
        server<client> s(4,port);
        s.run();
    }

需要注意的是，对同一个套接字进行读写的协程应该建立在同一个调度器上。因为实际上从green化的IO对象的构造函数即可以看出:green化的IO对象是与调度器中io_service对象绑定的。当不同调度器上的协程访问同一个IO对象的时，orchid不能保证其行为的正确性。

