## Design Patterns In Modern C++ 中文版翻译

### 动机

1. 本书的示例是用C++11、14、17和更高版本的现代C++编写的，有助于熟悉现代C++的语法。
2. 设计模式是编程经验的总结，广泛存在于工程实践中，牵扯出非常多的相关内容（比大家熟悉的单例模式为例，可以引出C++11后的多线程内存模型，除了用局部静态变量还可以用Acquire and Release栅栏， Sequentially Consistent 原子操作等无锁方式实现，以及folly中如何在工业实践中实现Singleton来管理多个Singletons），以此为线索梳理所学的知识。
3. 打算在原书的基础上补充大量的相关知识，如STL、Boost和folly中的设计模式，举例Leetcode题目中的设计模式，还有融入多线程并发情况下的一些例子。

### TODO

- [x] Chapter01: Introduction. 我直接使用了[@soyoo的翻译结果](https://github.com/soyoo/design_patterns_in_modern_cpp_zh_sample)
- [x] Chapter02: Builder。
- [x] Chapter03: Factories. 涉及工厂方法、工厂、内部工厂、抽象工厂和函数工厂。
- [x] Chapter04: Prototype. 原型模式，对深拷贝的实现做了一些讨论(拷贝构造函数和拷贝复制运算符；序列化和反序列化)。
- [x] Chapter05: Singleton. 线程安全，单例的问题，控制反转和Monostate.
- [x] Chapter06: Adapter. 额外补充了STL中queue的实现，提供了一个更安全和方法的Queue。需要了解boost库中的hash是怎么做的。
- [x] Chapter07: Bridge. 增加了Pimpl编程技法的说明。
- [x] Chapter08: Composite. 
- [x] Chapter09：Decorator. 涉及动态装饰器、静态装饰器 和 函数装饰器。
- [x] Chapter10: Facade. 外观模式, 缓冲-视窗-控制台。
- [x] Chapter11: Flyweight. 享元模式。Boost库中Flyweight的实现，以及Bimap
- [x] Chapter12: Proxy. 智能指针、属性代理、虚代理和通信代理。 
- [x] Chapter13: Chain of Responsibility. 指针链；代理链涉及中介模式和观察者模式。
- [x] Chapter14: Command.
- [x] Chapter15: Interpreter.涉及编译原理里面的词法分析，语法分析，`Boost.spirit`的使用。后面会补充LeetCode上实现计算器的几道题目和正则表达式的题目，也许会增加`Lex/Yacc`工具的使用介绍，以及tinySQL解释器实现的简单解释。
- [x] Chapter16: Iterator. STL库中的迭代器，涉及二叉树的迭代器，使用协程来简化迭代过程。
- [x] Chapter17: Mediator.
- [x] Chapter18: Memento.
- [x] Chapter19: Nulll Object. 涉及到对代理模式和pimpl编程技法的运用，以及std::optional
- [x] Chapter20: Observer. 属性观察者、模板观察者Observer\<T>、可观察Observable\<T> 、依赖问题, 取消订阅与线程安全 和使用Boost.Signals2 来实现 Observer。
- [x] Chapter21: State.  补充字符串匹配、例子
- [x] Chapter22: Strategy. 动态策略和静态策略
- [x] Chapter23: Template Method. 模版方法模式和策略模式的异同。
- [x] Chapter24: Visitor. 入侵式、反射式、经典式的访问者的设计思路，std::visitor在variant类型上的访问。

## 补充

在原著的基础上补充了很多相关的东西，
- 第5章-单例：补充了无锁的double-check实现。
- 第6章-适配器：探讨了STL中queue的适配器设计，提供了一个更方便和更安全的`Queue`适配器实现。
- 第7章-桥接：对C++中的Pimpl编程技法进行了补充，给出了在编译器方面的应用。
- 第12章-代理：讨论C++中智能指针的实现，给出一个半线程安全的智能指针`shared_ptr`的实现。
- 第20章-观察者：补充了由观察者模式衍生出来的发布-订阅模式，总结了消息队列使用注意事项，提供了2种用redis来实现消息队列的解决方案。

### 第5章-单例：无锁的double-check实现

```c++
class Singleton
{
    protected:
        Singleton();
    private:
        static std::mutex m_mutex;
        static std::atomic<Singleton*> m_instance = nullptr;
    public:
        static Singleton* Singleton::getInstance() 
        {
            Singleton* tmp = m_instance.load(std::memory_order_acquire);
            if (tmp == nullptr) 
            {
                //std::scoped_lock(m_mutex);
                std::lock_guard<std::mutex> lock(m_mutex);
                tmp = m_instance.load(std::memory_order_relaxed);
                if (tmp == nullptr) 
                {
                    tmp = new Singleton;
                    m_instance.store(tmp, std::memory_order_release);
                }
            }
            return tmp;
        }
        Singleton(const Singleton&) = delete;
        Singleton& operator=(const Singleton&) = delete;
        Singleton(Singleton&&) = delete;
        Singleton& operator=(Singleton&&) = delete;

};
```
###  第6章-适配器：设计更安全方便的Queue

STL中`queue`是一个FIFO队列，提供的核心接口函数为

- push( ) : 插入一个元素到队列中
- front( ) : 返回队首元素
- back( ) : 返回队尾元素
- pop( ) : 移除队首元素


我们可以看到在STL头文件`<queue>`中`queue`中定义：

```c++
namespace std
{
    template<typename T, typename Container = deque<T> >
    class queue;
}
```

注意到第二个可选参数，说明在`queue`中默认使用`deuqe`作为实际存储`T`类的容器, 当然也可以使用任何提供成员函数`front()`、`back()`、`push_back()`和`pop_front()`的序列容器类，如`list`。标准库中的`queue`的实现更注重效率而不是方便和安全。这并不是适合于所有场合。我们可以在一个提供`deque`容器做出修改，适配出一个不同于标准库但符合自己风格的`Queue`。

下面实现的Queue提供了抛出异常的处理，以及返回队首元素的新的`pop`方法。

```c++
template <typename T,  Container = deque<T> >
class Queue
{
    protected:
        Container c;
        
        //异常类：在一个空队列中调用 pop() 和 front() 
        class EmptyQueue : public exception
        {
            public:
                virtual const char* what const throw( )
                {
                    return "empty queue!";
                }
            
        }
        
        typename Container::size_type size( ) const
        {
            return c.size( );
        }
        
        bool empty( ) bool 
        {
            return c.empty();
        }

        void push( const T& item )
        {
            c.push( item );
        }
        
        void push( T&& item )
        {
            c.push( item );
        }

        const T& front( ) const
        {
            if( c.empty( ) )
                throw EmptyQueue();
            return c.front();
        }
        
        T& front( )
        {
             if( c.empty( ) )
                throw EmptyQueue( );
            return c.front( );
        }

        const T& back( ) const
        {
            if( c.empty( ) )
                throw EmptyQueue();
            return c.back();
        }
        
        T& front( )
        {
             if( c.empty( ) )
                throw EmptyQueue( );
            return c.back( );
        }

        // 返回队首元素
        T pop( ) const 
        {
            if( c.empty() )
                throw EmptyQueue();
            
            T elem( c.front( ) );
            c.pop();
            return elem;
        }
        
};

```

### 第7章-桥接：Pimpl编程技法-减少编译依赖

PImpl（Pointer to implementation）是一种C++编程技术，其通过将类的实现的详细信息放在另一个单独的类中，并通过不透明的指针来访问。这项技术能够将实现的细节从其对象中去除，还能减少编译依赖。有人将其称为“编译防火墙（Compilation Firewalls）”。

#### Pimpl技法的定义和用处

在C ++中，如果头文件类定义中的任何内容发生更改，则必须重新编译该类的所有用户-即使唯一的更改是该类用户甚至无法访问的私有类成员。这是因为C ++的构建模型基于文本包含（textual inclusion），并且因为C ++假定调用者知道一个类的两个主要方面，而这两个可能会受到私有成员的影响：

- 因为类的私有数据成员参与其对象表示，影响大小和布局，
- 也因为类的私有成员函数参与重载决议（这发生于成员访问检查之前），故对实现细节的任何更改都要求该类的所有用户重编译。

为了减少这些编译依赖性，一种常见的技术是使用不透明的指针来隐藏一些实现细节。这是基本概念：

```c++
// Pimpl idiom - basic idea
class widget {
    // :::
private:
    struct impl;        // things to be hidden go here
    impl* pimpl_;       // opaque pointer to forward-declared class
};
```

类widget使用了handle/body编程技法的变体。handle/body主要用于对一个共享实现的引用计数，但是它也具有更一般的实现隐藏用法。为了方便起见，从现在开始，我将widget称为“可见类”，将impl称为“ Pimpl类”。

编程技法的一大优势是，它打破了编译时的依赖性。首先，系统构建运行得更快，因为使用Pimpl可以消除额外的#include。我从事过一些项目，在这些项目中，仅将几个广为可见的类转换为使用Pimpls即可使系统的构建时间减少一半。其次，它可以本地化代码更改的构建影响，因为可以自由更改驻留在Pimpl中的类的各个部分，也就是可以自由添加或删除成员，而无需重新编译客户端代码。由于它非常擅长消除仅由于现在隐藏的成员的更改而导致的编译级联，因此通常被称为“编译防火墙”。

#### Pimpl技法的实践

避免使用原生指针和显式的`delete`。要仅使用C++标准设施表达`Pimpl`，最合适的选择是通过`unique_ptr`来保存`Pimpl`对象，因为Pimpl对象唯一被可见类拥有。使用`unique_ptr`的代码很简单：

```c++
// in header file
class widget {
public:
    widget();
    ~widget();
private:
    class impl;
    unique_ptr<impl> pimpl;
};
 
// in implementation file
class widget::impl {
    // :::
};
 
widget::widget() : pimpl{ new impl{ /*...*/ } } { }
widget::~widget() { }                   // or =default

```

//TODO: handle/body编程技法

#### 

### 第12章-代理：实现一个半线程安全的智能指针

1. 智能指针(shared_ptr)线程安全吗?
   
- （"half thread-safe"）

- 是: 引用计数控制单元线程安全, 保证对象只被释放一次

- 否: 对于数据的读写没有线程安全

2. 如何将智能指针变成线程安全?

- 使用 mutex 控制智能指针的访问
- 使用全局非成员原子操作函数访问, 诸如: std::atomic_load(), atomic_store(), …
- 缺点: 容易出错, 忘记使用这些操作
- C++20: atomic<shared_ptr<T>>, atomic<weak_ptr<T>> std::atomic_shared_ptrand astd::atomic_weak_ptr.
- 内部原理可能使用了 mutex
- 全局非成员原子操作函数标记为不推荐使用(deprecated)

3. 数据竞争

- 一个shared_ptr对象实体可以被多个线程同时读取
- 两个shared_ptr对象实体可以被两个线程同时写入，"析构"算写操作
- 如果要从多个线程读写同一个shared_ptr,那么需要加锁。

4. 代码实现

```c++ 
#include <atomic>

// 非完全线程安全的。
// 引用计数
template<typename T>
class ReferenceCounter
{
    
    ReferenceCounter( ):count(0) {};
    ReferenceCounter(const ReferenceCounter&) = delete;
    ReferenceCounter& operator=(const ReferenceCounter&) = delete;
    ReferenceCounter(ReferenceCounter&&) = default
    ReferenceCounter& operator=(ReferenceCounter&&) = default;
    // 前置++, 这里不提供后置，因为后置返回一个ReferenceCounter的拷贝，而之前禁止ReferenceCounter拷贝。
    ReferenceCounter& operator++()
    {   
        count.fetch_add(1);
        return *this;
    }

    ReferenceCounter& operator--()
    {
        count.fetch_sub(1);
        return *this;
    }

    size_t getCount() const 
    {
        return count.load();
    }

    private:
        // 原子类型，或上锁
        std::atomic<size_t> count;
        
};

template <typename T>
class SharedPtr{
    
    explicit SharedPtr(T* ptr_ = nullptr) : ptr(ptr_) {
        count = new ReferenceCounter();
        if(ptr)
        {
            ++(*count);
        } 
    }
    ~SharedPtr() {
        --(*count);
        if(count->getCount() == 0)
        {
            delete ptr;
            delete count;
        }
    }

    SharedPtr(const SharedPtr<T>& other) : 
        ptr(other.ptr), 
        count(other.count)
    {
        ++(*count);
    }

    SharedPtr<T>& operator= (const SharedPtr<T>& other)
    {
        if(this != &other)
        {
            --(*count);
            if(count.getCount() == 0)
            {
                delete count;
                delete ptr;
            }
            ptr = other.ptr;
            count = other.count;
            ++(*count);
        }
        return *this;   
    }

    SharedPtr(SharedPtr&& other) = default;
    SharedPtr& operator=(SharedPtr&& other) = default;

    T& operator*() const { return *ptr; }
    T* operator->() const{
        return ptr;
    }
    T* get() const { return ptr; }
    
    int use_count() const { return count->getCount(); }

    bool unique() const { return use_count() == 1; }

    private:
        T* ptr;
        ReferenceCounter* count;
}
```


5.  重新回顾C++线程池中使用的虚调用方法


### 第20章：观察者模式

从观察者模式出发，了解了发布订阅模式，到消息队列，再到用redis实现消息队列以及实现消息队列的注意事项

#### 观察者模式

#### 发布订阅模式
- 发布者和订阅者不直接关联，借助消息队列实现了松耦合，降低了复杂度，同时提高了系统的可伸缩性和可扩展性。
- 异步

#### 消息队列

##### 使用redis实现消息队列

- 如何保证有序：FIFO数据结构
- 如何保证不重复：全局Id(LIST法1：生产者和消费者约定好；STREAMS法二：消息队列自动产生（时间戳）)
- 如何保证可靠性：备份

|功能和适用场景|基于List|基于Streams| 备注
|:-           |:-      |:-        |:-        |
| 适用场景 |Redis5.0版本的部署环境，消息总量小| Redis5.0及以后的版本的部署环境，消息总量大，需要消费组形式读取数据 | |
| 消息保序 |LPUSH/RPOP| XADD/XREAD | |
| 阻塞读取 |BRPOP|XREAD block | 阻塞式读取在客户端没有读到队列数据时，自动阻塞，节省因不断peek的CPU开销 |
| 重复消息处理|生产者自行实现全局唯一ID| STREAMs自行生成全局唯一ID | |
| 消息可靠性|适用BRPOPRPUSH| 使用PENDING List 自动存留消息，使用XPENGDING查看，使用XACK确认消息 | |

##### 消息队列的注意事项


在使用消息队列时，重点需要关注的是如何保证不丢消息？

那么下面就来分析一下，哪些情况下，会丢消息，以及如何解决？

1、生产者在发布消息时异常：

a) 网络故障或其他问题导致发布失败（直接返回错误，消息根本没发出去）
b) 网络抖动导致发布超时（可能发送数据包成功，但读取响应结果超时了，不知道结果如何）

情况a还好，消息根本没发出去，那么重新发一次就好了。但是情况b没办法知道到底有没有发布成功，所以也只能再发一次。所以这两种情况，生产者都需要重新发布消息，直到成功为止（一般设定一个最大重试次数，超过最大次数依旧失败的需要报警处理）。这就会导致消费者可能会收到重复消息的问题，所以消费者需要保证在收到重复消息时，依旧能保证业务的正确性（设计幂等逻辑），一般需要根据具体业务来做，例如使用消息的唯一ID，或者版本号配合业务逻辑来处理。

2、消费者在处理消息时异常：

也就是消费者把消息拿出来了，但是还没处理完，消费者就挂了。这种情况，需要消费者恢复时，依旧能处理之前没有消费成功的消息。使用List当作队列时，也就是利用老师文章所讲的备份队列来保证，代价是增加了维护这个备份队列的成本。而Streams则是采用ack的方式，消费成功后告知中间件，这种方式处理起来更优雅，成熟的队列中间件例如RabbitMQ、Kafka都是采用这种方式来保证消费者不丢消息的。

3、消息队列中间件丢失消息

上面2个层面都比较好处理，只要客户端和服务端配合好，就能保证生产者和消费者都不丢消息。但是，如果消息队列中间件本身就不可靠，也有可能会丢失消息，毕竟生产者和消费这都依赖它，如果它不可靠，那么生产者和消费者无论怎么做，都无法保证数据不丢失。

a) 在用Redis当作队列或存储数据时，是有可能丢失数据的：一个场景是，如果打开AOF并且是每秒写盘，因为这个写盘过程是异步的，Redis宕机时会丢失1秒的数据。而如果AOF改为同步写盘，那么写入性能会下降。另一个场景是，如果采用主从集群，如果写入量比较大，从库同步存在延迟，此时进行主从切换，也存在丢失数据的可能（从库还未同步完成主库发来的数据就被提成主库）。总的来说，Redis不保证严格的数据完整性和主从切换时的一致性。我们在使用Redis时需要注意。

b) 而采用RabbitMQ和Kafka这些专业的队列中间件时，就没有这个问题了。这些组件一般是部署一个集群，生产者在发布消息时，队列中间件一般会采用写多个节点+预写磁盘的方式保证消息的完整性，即便其中一个节点挂了，也能保证集群的数据不丢失。当然，为了做到这些，方案肯定比Redis设计的要复杂（毕竟是专们针对队列场景设计的）。

综上，Redis可以用作队列，而且性能很高，部署维护也很轻量，但缺点是无法严格保数据的完整性（个人认为这就是业界有争议要不要使用Redis当作队列的地方）。而使用专业的队列中间件，可以严格保证数据的完整性，但缺点是，部署维护成本高，用起来比较重。

所以我们需要根据具体情况进行选择，如果对于丢数据不敏感的业务，例如发短信、发通知的场景，可以采用Redis作队列。如果是金融相关的业务场景，例如交易、支付这类，建议还是使用专业的队列中间件。

##### 消息队列实现方式对比

关于Redis是否适合做消息队列，业界一直存在争议。很多人认为使用消息队列就应该采用KafKa，RabbitMQ这些专门没安心消息队列场景的软件，而Redis更适合做缓存。


|功能和适用场景|基于Redis|基于Kafka/RabbitMQ| 备注
|:-           |:-      |:-        |:-        |
| 适用场景 |对丢数据不敏感的业务，如发短信和通知| 严格数据完整的应用，金融相关业务场景，支付和交易 | |
| 属性 |轻量级，部署和维护简单，性能高| 重量级 | |
| 数据完整性 |不保证严格的数据完整性和主从切换的一致性| 部署集群，多结点+预写磁盘保证数据完整性 |  |
| 重复消息处理|生产者自行实现全局唯一ID| STREAMs自行生成全局唯一ID | |
| 消息可靠性|适用BRPOPRPUSH| 使用PENDING List 自动存留消息，使用XPENGDING查看，使用XACK确认消息 | |

### 如何学习设计模式(即如何学习C++的面向对象思想)
[该内容来自@franktea](https://github.com/franktea)
#### 面向对象的三大特征
所有的人都知道，面向对象的三大特征是封装继承多态（这里的多态指的是运行时多态，本文中所有多态均指运行时多态），那么哪个特征才是面向对象最重要的特征呢？

先说封装。其实C语言的struct也是一种封装，所以封装并不是面向对象所独有。再看继承，继承可以非常方便地重用代码，相对面向过程来说是一种非常强大的功能，在面向对象刚被发明出来不久的一段时间里，继承被很多人看成面向对象最强大的特征。

到后来，人们发现面向对象最强大的特征是多态，因为代码不仅仅是需要重用，扩展也很重要。「设计模式」中，几乎每种模式，都是用多态来实现的。

一个问题：只支持多态，不支持继承的编程语言，算是面向对象的编程语言吗？

我的答案：不是。虽然继承不如多态重要，但是它不是多余的。多态往往是配合继承才更强大。

#### 类的设计以及多态
第一点，前面说了，面向对象最重要的是多态，多态就是使用虚函数，在自己设计的类中，将哪些成员函数定义为虚函数，这是一个重要的问题。对于新手，我的建议是：在搞清规则之前，可以将所有的成员函数都定义为虚函数。（其实在java这样的编程语言中，根本不需要程序员自己去指定哪个成员函数是virtual，从语法上来说，任何一个非static非private的都是virtual。）

在虚函数的定义上，先将所有能定义成虚函数的的成员函数全部声明为virtual，然后再在使用中慢慢做减法，根据自己的理解，将多余的virtual去掉。

第二点，在使用面向对象的时候，尽量使用父类指针，而不是子类指针。100分的设计是永远使用父类指针、永远不使用子类指针。父类指针向子类指针转换需要用dynamic_cast，不使用dynamic_cast的设计就是最好的设计。新手可以遵循这个原则。

当然，在一些非常复杂的系统中，无法做到100分，有时候还是需要向下转换成子类指针，这样的设计肯定是扣分的，但是对于复杂系统肯定有一个平衡。我自己做服务器，所有设计都可以做到永远使用父类指针，但是对于复杂的像客户端unnreal代码，向下转换几乎不可避免。

#### 虚函数表和虚函数指针

关于面向对象的语法知识，我唯一觉得需要强调的就是对于vtable的实现，推荐大家用实验的方法，一定要自己写代码亲自操作一遍（按照随便一篇vtable原理的文章动手操作一遍即可），无论是通过单步调试去查看vtable，还是通过编译器的各种命令来查看，都要自己亲自动手操作一下，加深印象。

#### 补充
gdb调试要点： 64位, 设置断点，打印虚表。
```
g++ 多继承有虚函数重写.cpp -o 多继承有虚函数重写 -m64 -g
break 30
set print pretty on
info vtbl d
```



### 一篇引文：从vtable到设计模式——我的C++面向对象学习心得

[该内容来自@franktea](https://github.com/franktea)
#### 前言

按照很多教程的内容安排，学习C++语法以后很快就会进入到面向对象的学习，在初学者的心中，面向对象有非常重要的地位。但是如何才能快速学习面向对象、多久学会面向对象才算正常，这是新手常见的问题。

面向对象的语法书上都有说，vtable的原理也有非常多的文章进行讲述，这些东西再一再重复没有意义，我想写一些我自己在学习过程中的心得体验。

关于面向对象的语法知识，我唯一觉得需要强调的就是对于vtable的实现，推荐大家用实验的方法，一定要自己写代码亲自操作一遍（按照随便一篇vtable原理的文章动手操作一遍即可），无论是通过单步调试去查看vtable，还是通过编译器的各种命令来查看，都要自己亲自动手操作一下，加深印象。

面向对象的语法风格出现以后，无数的程序员基于这些特性写出了很多代码，各显神通，后来被总结提取出一些可复用的方法论，叫做「设计模式」。设计模式是学习和掌握面向对象思想的重要课程。那么问题来了，何时学习设计模式？如何学习？在理解设计模式之前应该做什么、能做什么？

#### 封装、继承、（运行时）多态，哪个才是面向对象最重要的特征

所有的人都知道，面向对象的三大特征是封装继承多态（这里的多态指的是运行时多态，本文中所有多态均指运行时多态），那么哪个特征才是面向对象最重要的特征呢？

先说封装。其实C语言的struct也是一种封装，所以封装并不是面向对象所独有。再看继承，继承可以非常方便地重用代码，相对面向过程来说是一种非常强大的功能，在面向对象刚被发明出来不久的一段时间里，继承被很多人看成面向对象最强大的特征。

到后来，人们发现面向对象最强大的特征是多态，因为代码不仅仅是需要重用，扩展也很重要。「设计模式」中，几乎每种模式，都是用多态来实现的。

一个问题：只支持多态，不支持继承的编程语言，算是面向对象的编程语言吗？

我的答案：不是。虽然继承不如多态重要，但是它不是多余的。多态往往是配合继承才更强大。

#### 设计模式的意义

设计模式对于如何用面向对象的思想解决软件中的设计和实现问题提供了一些可重用的思路，它还有一个重要的意义，就是为每种设计思路都取了名字，便于程序员之间的交流。

有些人在设计类的名字的时候就包含了使用的设计模式，比如一个使用了adapter模式的类名字叫xxxAdapter；xxxFactory一看就知道它使用了factory模式，给其它使用和维护这些代码的人节省了大量的时间。

#### 何时开始学习设计模式

知乎上见过一个问题：『你想对刚毕业的人说些什么』，这个问题就是一个刚踏入社会的小鲜肉，向在社会上摸爬滚打多年的人取经，想获得一些生存闯关的金句宝典，从而让自己少踩坑。

这样的问题的答案有意义吗？有一些是有的，可以直接理解，但是很多是要结合自己过去的经验教训才能有体会的，知道得早也没有什么收获。

如果面向对象的初学者也提问：「你想对刚学习面向对象的人说什么？」，答案就在设计模式这本书中。

所以何时开始学习设计模式呢？我的答案是任何时候都可以。但是唯一要注意的就是，不要强迫自己去理解，设计模式的书可以摆在那里，想看就看一下，能理解多少就理解多少。但是越早看设计模式这本书，共鸣就越少，因为共鸣是要结合自己写面向对象代码的经验的。

学习面向对象几年以后再看设计模式是否可以行？

我觉得可行，结合自己几年之内在学习各种面向对象的库和自己写代码的经验，学习设计模式会很快。

永远不学设计模式行不行？

我觉得不行，我前面提到了，设计模式不仅仅是总结思想，思想可以通过模仿现有的库来学习，但是设计模式还有一个重要的作用是给模式命名，命名可以更好地与其他程序员沟通交流。

#### 如何学习设计模式(即如何学习C++的面向对象思想)

除了学习C++语法，还需要学习一下UML类图，不会的自己去搜，UML有好几种图，其中类图、状态图、序列图最为常用，学这3种即可。

在C++中可以通过学Qt库来学习面向对象。Qt除了可以用来写跨平台的UI，还可以写一些简单的网络程序，在学校里可以用来做各种大作业，无论是学生成绩管理系统、图书管理系统、足球俱乐部，等等，用Qt都可以很好地完成。我学Qt用的是这本书：<<Qt Creator 快速入门 霍亚飞著>>

Qt里面本身就用了很设计模式，从它的类里面继承一个子类，覆盖一个或几个虚函数，就可以将自己的类融入到Qt的体系中。其实这就是学习面向对象的第一步，也是最好的开始，不吃猪肉、先看猪跑，从它的类继承多了，自己也会慢慢理解如何从自己写的类继承。

学习面向对象有什么减少弯路又能加速理解的套路呢？根据我自己的经验总结，对于新手我至少可以说两点。

第一点，前面说了，面向对象最重要的是多态，多态就是使用虚函数，在自己设计的类中，将哪些成员函数定义为虚函数，这是一个重要的问题。对于新手，我的建议是：在搞清规则之前，可以将所有的成员函数都定义为虚函数。（其实在java这样的编程语言中，根本不需要程序员自己去指定哪个成员函数是virtual，从语法上来说，任何一个非static非private的都是virtual。）

在虚函数的定义上，先将所有能定义成虚函数的的成员函数全部声明为virtual，然后再在使用中慢慢做减法，根据自己的理解，将多余的virtual去掉。

第二点，在使用面向对象的时候，尽量使用父类指针，而不是子类指针。100分的设计是永远使用父类指针、永远不使用子类指针。父类指针向子类指针转换需要用dynamic_cast，不使用dynamic_cast的设计就是最好的设计。新手可以遵循这个原则。

当然，在一些非常复杂的系统中，无法做到100分，有时候还是需要向下转换成子类指针，这样的设计肯定是扣分的，但是对于复杂系统肯定有一个平衡。我自己做服务器，所有设计都可以做到永远使用父类指针，但是对于复杂的像客户端unnreal代码，向下转换几乎不可避免。

#### 多久学会设计模式才算优秀

初学者都很急于求成，希望一天就能学会。但是从另一个角度来说，一天都能学会的东西，肯定不是什么有价值的东西。

我大概用了4年左右的时间，理解了面向对象。从大一开始学习C++，到大四毕业工作以后一年内设计出来了一个总共有一千多个类的系统，可以按照需求无限扩展。我现在可以设计任意多个类的系统。

我相信很多人比我更优秀，但是我更相信我自己的方法，我的学习方法其实就是不给自己设置时间期限，盲人摸象，今天摸这里明天摸那里，时间长了总会知道大象的全貌。

我是打算用几十年的时间从事编程的工作，到底是一天理解还是几年理解，对我来说并没有区别。至于做题、考试、工作等等，不用理解一样可以完成，按照现有的系统模仿即可。

我很清楚地记得，我第一次体会到面向对象的意义，是模仿MFC的一个机制。MFC在90年代的时候就做到了可以用字符串来动态创建一个对象（C++没有反射机制这在语法层面是无法做到的），MFC用的方法非常简单，将所有的类的名字和其构造函数放在一个全局的链表中，通过字符串在链表中去查找对应的构造函数，从而调用该构造函数new出对应的对象。

需要添加新的功能的时候，只要新添加一个.h一个.cpp，在两个文件中实现一个子类的代码，并调用宏将该类的构造函数添加到全局链表中。

通过添加新文件（一个.h和一个.cpp）的方法，不用修改之前的任何代码，就扩展了程序的功能，这就是面向对象的意义之一。

后来我在鹅厂做服务器，这个方法我一直使用，只是将链表改成了map或unordered_map。以后如果我找到合适的例子，我想通过例子说明此思想的应用，作为面向对象思想理解的入门级素材，我觉得挺好的，当然，那就是另外一篇文章了。

#### 总结

设计模式是一些方法论，自己通过学习优秀的C++框架（如Qt）慢慢去体会和应用这些方法，最终可以慢慢理解。

不要刻意急于求成，人生很长，每一步都有它的意义，走过的路哪怕是弯路，都有它的意义。

在理解之前，注重于模仿，即使不理解，靠模仿已经能解决很多问题。

如果硬要问捷径是什么，我的答案就是抓紧时间多写代码，写了几万行代码就慢慢理解了。如果你不能改变几万行这个数字，那就去改变积累几万行代码的时间。比如说从3年缩短到2年，这完全是可能的。

我非常讨厌写长文，这篇文章在没有任何代码凑字数的情况下还是超过了3000字，也是源于我对面向对象思想的热爱，它帮我解决了很多问题，我现在用面向对象的思想来写代码，已经成了一件很自然的事情。

其实面向对象的思想在C++中并不是主流，自从90年代STL被作为标准库纳入C++那一刻起，泛型编程在C++里面就占据了上风，并且后来一直在迅速发展。同样的设计模式在C++中不仅仅可以用面向对象的思想实现，也可以用泛型编程的思想实现，不少时候后者可能更神奇更优雅更高效。

面向对象注重的是代码的扩展和维护，而不是高性能，在一些需要高性能的场合，像我所在的游戏领域需要优化性能的地方，不能用面向对象，以后如果我找到合适的例子作为素材，我会再写一篇「面向对象的缺点」的文章。

### 声明

译者纯粹出于学习目的与个人兴趣翻译本书，不追求任何经济利益。

本译文只供学习研究参考之用，不得公开传播发行或用于商业用途。有能力阅读英文书籍者请购买正版支持。

### 许可

[CC-BY 4.0](LICENSE)


### 参考

 1. [《Design Patterns In Modern C++》](https://book.douban.com/subject/30200080/)
 2. [《The C++ Standard Library - A Tutorial and Reference, 2nd Edition》](http://cppstdlib.com/)
   