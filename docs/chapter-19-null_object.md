### 第19章 空对象

我们并不能总能选择自己想使用的接口。例如，我宁愿让我的车自己开车送我去目的地，而不必把100%的注意力放在道路和开车在我旁边的危险疯子身上。软件也是如此:有时你并不是真的想要某一项功能，但它是内置在接口里的。那么你会怎么做呢?创建一个空对象。

#### 场景

假设继承了使用下列接口的库：

```c++
struct Logger
{
    virtual ~Logger() = default;
    virtual void info(const string& s) = 0;
    virtual void warn(const string& s) = 0;
}
```

这个库使用下面的接口来操作银行账户：

```c++
struct BankAccount
{
    std::shared_ptr<Logger> log;
    string name;
    int balance = 0;
    BankAccount(const std::share_ptr<Logger>& logger, const string& name, int balance):
        log{ logger },
        name{ name },
        balance {balance}
    {
        // more members here
    }
};
```

事实上，`BankAccount`可以拥有如下的成员函数:

```c++
void BankAccount::deposit(int amount)
{
    balance += amount;
    log->info(("Deposited $" + lexical_cast<string>(amount)
                + " to " + name + ", balance is now $" 
                + lexical_cast<string>(balance));
}
```

好了，这个实现有什么吗?如果你确实需要日志记录，也没有问题，你只需实现自己的日志记录类...

```c++
struct ConsoleLogger : Logger
{
    void info(const string& s) override
    {
        cout << "INFO: " << s << endl;
    }
    void warn(const string& s) override
    {
        cout << "WARNNING!!!" << s << endl;
    }
};
```

你可以直接使用它。但是，如果你根本不想要日志记录呢?

### 空对象

我们再来仔细看下`BankAccount`的构造函数

```c++
BankAccount(const shared_ptr<Logger>& logger, const string& name, int balance)
```

由于构造函数接受一个日志记录器，因此传递一个未初始化的`shared_ptr<BankAccount>`是不安全的。`BankAccout`可以使用指针之前，在内部检查指针是否为空，但你不知道它是否这样做了，因为没有额外的文档是不可能知道的。

因此，唯一可以传入`BankAccount`的是一个空对象，一个符合接口但不包含功能的类:

```c++
struct NullLoggor : Logger
{
    void info(const string& s) override { }
    void warn(const string& s) override { }
};
```

### 共享指针不是空对象

值得注意的是，`shared_ptr`和其他智能指针类都不是空对象。空对象是保留正确操作(执行无操作)的对象。但是，使用对未初始化的智能指针会崩溃会导致程序崩溃：

```c++
shared_ptr<int> n;
int x = *n + 1; // yikes!
```

值得注意的是，从调用的角度来看，没有办法使智能指针是安全的。换句话说，如果`foo`没有初始化，那么`foo->bar()`会神奇地变成一个空操作，那么你不能编写这样的智能指针。原因是前缀*和后缀->操作符只是代理了底层(原始)指针。没有办法对指针做无操作。

#### 改进设计

停下来想一想:如果`BankAccount`在你的控制之下，你能改进接口使它更容易使用吗?这里有一些想法：

- 在所有地方都进行指针检查。这就理清了`BankAccount`的正确性，但并没有消除库使用者的困惑。请记住，你仍然没有说明指针可以是空的。
- 添加一个默认实参值，类似于`const shared_ptr<Logger>& logger = no_logging`其中`no_logging`是`BankAccount`类的某个成员。即使是这样，你仍然必须在想要使用对象的每个位置对指针值执行检查
- 使用可选(`optional`)类型。它的习惯用法是正确的，并且可以传达意图，但是会导致传入一个`optional<shared_ptr<T>>`以及随后检查可选项是否为空。

#### 隐式空对象

这里有一个激进的想法，需要进行两步操纵。它把涉及到把日志记录过程细分为调用(我们想要一个好的日志记录器接口)和操作(日志记录器实际做的事情)。因此，请考虑以下几点：

```c++
struct OptionalLogger : Logger 
{
    shared_ptr<Logger> impl;
    static shared_ptr<Logger> no_logging;
    Logger(const shared_ptr<Logger>& logger) : impl { logger } { }
    virtual void info(const string& s) override
    {
        if(impl) impl->info(s); // null check here
    }
    // and similar checks for other members
};

// a static instance of a null object
shared_ptr<Logger> BankAccount::no_logging{};
```

现在我们已经从实现中抽象出了调用。我们现在要做的是像下面这样重新定义`BankAccount`构造函数:

```c++
shared_ptr<OptionalLogger> logger;
BankAccount(const string& name, int balance, const shared_ptr<Logger>& logger = no_logging) : 
    log{ make_shared<OptionalLogger>(logger) },
    name{ name },
    balance{ balance } { }
```

如您所见，这里有一个巧妙的诡计:我们使用一个`Logger`，但存储一个`OptionalLogger`(这是代理设计模式)。然后，对这个可选记录器的所有调用都是安全的-它们只有在底层对象可用时才“发生”:

```c++
BankAccount account{ "primary account", 1000 };
account.deposit(2000); // no crash
```

上例中实现的代理对象本质上是`Pimpl`编程技法的自定义版本。

#### 总结

空对象模式提出了一个API设计的问题:我们可以对我们所依赖的对象做什么样的假设?如果我们取一个指针(裸指针或智能指针)，那么是否有义务在每次使用时检查该指针？


如果你觉得没有这种义务，那么用户实现空对象的唯一方法是构造所需接口的无操作实现，并将该实例传递进来。也就是说，这只适用于函数:例如，如果对象的字段也被使用，那么你就遇到了真正的麻烦。

如果你想主动支持空对象作为参数传递的想法,你需要明确:要么指定参数类型为`std::optional`,给参数一个默认值，暗示它是一个内置的空对象(例如，= no_logging),或只写文档说明什么样的值应当出现在这个位置。