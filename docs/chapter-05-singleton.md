### 单例模式

单例模式可以说是面试中经常碰到的设计模式了(因为它简单且好实现)，它的基本思想非常简单：应用程序中应该只有一个特定组件的实例。例如，当我们将数据库加载到内存并提供只读接口时，应当考虑使用单例模式，因为浪费内存来存储几个相同的数据集是没有意义的。实际上，你的应用程序可能存在这样的约束，即两个或多个数据库实例不能放入内存(或者导致内存不足而引起程序故障)。


#### 全局单例对象


一个很简单的实现单例的方法是使用静态全局对象：

```c++
static Database database{};
```

全局静态对象的问题在于它们在不同编译单元中的初始化顺序是不确定的。这可能会导致糟糕的后果，比如当一个全局对象去引用另一个全局对象，而后者还未被初始化，则会出现问题。

一个解决此问题的方法是使用一个全部函数来返回这个静态对象：
```c++
 Database& get_database() {
  static Database database;
  return database;
};
```
可以调用这个函数来获得对数据库的引用。但是，我们需要知道，上述代码的线程安全只有在c++11之后才得到保证，我们应该检查编译器是否确实打算加入锁以防止静态对象初始化时的并发访问。

当然，这个场景很容易出现一些麻烦的问题:如果Database决定在其析构函数中使用其他类似的单例，程序可能会崩溃。这就引出了更多的哲学问题:单例对象可以引用其他的单例对象吗?

#### 经典实现

前面的代码并不能保证只有一个对象被创建。我们可以使用一个计数器来防止多于一个的对象被创建：
```c++
struct Database {
  Database() {
    static int instance_count{0};
    if (++instance_count > 1) throw std::exception("Cannot make >1 database!");
  }
};
```
对于这个问题，这是一种不太友好的方法:尽管它通过抛出异常来阻止创建多个实例，但它无法传达我们不希望任何人多次调用构造函数这个目的。防止显式构造数据库的唯一方法是再次将其构造函数设为私有，并引入上述函数作为成员函数来返回唯一的实例。在前c++11时代，你只需将复制构造函数设为私有，就可以达到大致相同的目的。

```c++
struct Database {
 protected:
  Database() { /* do what you need to do */
  }

 public:
  static Database& get() {
    // thread-safe in C++11
    static Database database;
    return database;
  }
  Database(Database const&) = delete;
  Database(Database&&) = delete;
  Database& operator=(Database const&) = delete;
  Database& operator=(Database&&) = delete;
};
```

最后，你可以将`get()`实现为一个堆分配。

```c++
static Database& get() {
  static Database* database = new Database();
  return *database;
}
```
前面的实现依赖于假设数据库一直存在到程序结束，并且使用指针而不是引用确保了析构函数(即使创建了析构函数，也必须是公共的)永远不会被调用。前面的代码不会导致内存泄漏。

#### 线程安全

正如已经提到的，从c++11开始，前面列出的单例初始化方式是线程安全的。即如果两个线程同时调用get()，我们不会遇到数据库被创建两次的情况。在c++ 11之前，我们将使用一种称为双重检查锁定的方法来构造单例。典型的实现如下所示：

```c++
struct Database {
  // same members as before, but then...
  static Database& instance();

 private:
  static boost::atomic<Database*> instance;
  static boost::mutex mtx;
};

Database& Database::instance() {
  Database* db = instance.load(boost::memory_order_consume);
  if (!db) {
    boost::mutex::scoped_lock lock(mtx);
    db = instance.load(boost::memory_order_consume);
    if (!db) {
      db = new Database();
      instance.store(db, boost::memory_order_release);
    }
  }
}    
```

当然本书讲述是以Modern c++为主，故不会在这方面再做过多讨论。


#### 单例模式遇到的麻烦

假设我们的数据库包含首都城市及其人口列表。我们的单例数据库要遵循的接口是：

```
class Database {
 public:
  virtual int get_population(const std::string& name) = 0;
};
```

我们有一个成员函数来获得给定城市的人口。现在，让我们假设这个接口被一个叫做`SingletonDatabase` 的具体实现所采用，这个实现singleton的方法和我们之前做的一样:

```c++
class SingletonDatabase : public Database {
  SingletonDatabase() { /* read data from database */
  }
  std::map<std::string, int> capitals;

 public:
  SingletonDatabase(SingletonDatabase const&) = delete;
  void operator=(SingletonDatabase const&) = delete;
  static SingletonDatabase& get() {
    static SingletonDatabase db;
    return db;
  }

  int get_population(const std::string& name) override {
    return capitals[name];
  }
};
```

正如我们所提到的那样，单例对象的真正问题在于它们在其他组件中的使用。比如，基于上面的例子，我们构建了一个来计算几个不同城市的总人口的函数：

```c++
struct SingletonRecordFinder {
  int total_population(std::vector<std::string> names) {
    int result = 0;
    for (auto& name : names)
      result += SingletonDatabase::get().get_population(name);
    return result;
  }
};
```

这个实现的问题在于，`SingletonRecordFinder` 现在完全依赖于`SingletonDatabase`。这为测试带来了一个问题:如果我们想要检查`SingletonRecordFinder` 是否正确工作，我们需要使用实际数据库中的数据，即:

```c++
TEST(RecordFinderTests, SingletonTotalPopulationTest) {
  SingletonRecordFinder rf;
  std::vector<std::string> names{"Seoul", "Mexico City"};
  int tp = rf.total_population(names);
  EXPECT_EQ(17500000 + 17400000, tp);
}
```

但是，如果我们不想使用实际的数据库，而是想用一些虚拟的数据进行测试，在我们目前的设计中，这是不可能的，也正是这种不灵活导致了单例模式的衰败。那么，我们应该怎么做呢?首先，我们不能再依赖显示地依赖`Singleton-Database`。因为我们所需要的只是实现数据库接口，所以我们可以创建一个新的`ConfigurableRecordFinder`来配置数据的来源。

```c++
struct ConfigurableRecordFinder {
  explicit ConfigurableRecordFinder(Database& db) : db{db} {}

  int total_population(std::vector<std::string> names) {
    int result = 0;
    for (auto& name : names) result += db.get_population(name);
    return result;
  }

  Database& db;
};
```

我们现在使用`db`引用，而不是显式地使用单例。这让我们可以创建一个专门用于测试`RecordFinder`的自行生成的虚拟数据库：

```c++
class DummyDatabase : public Database {
  std::map<std::string, int> capitals;

 public:
  DummyDatabase() {
    capitals["alpha"] = 1;
    capitals["beta"] = 2;
    capitals["gamma"] = 3;
  }

  int get_population(const std::string& name) override {
    return capitals[name];
  }
};
```

现在，我们可以重写我们的单元测试：

```c++
TEST(RecordFinderTests, DummyTotalPopulationTest) {
  DummyDatabase db{};
  ConfigurableRecordFinder rf{db};
  EXPECT_EQ(4, rf.total_population(std::vector<std::string>{"alpha", "gamma"}));
}
```

#### 单例和控制反转

明确地使组件成为单例的方法明显是侵入性的，并且决定停止将类视为单例最终的代价会特别昂贵。另一种解决方案是采用约定，而不是直接强制执行类的生命周期，而是将此功能外包给 `IoC` 容器。以下是在使用 `Boost.DI` 依赖注入框架时定义单例组件的样子：

```c++
auto injector = di::make_injector(di::bind<IFoo>.to<Foo>.in(di::singleton),
                                  // other configuration steps here
);
```

在前面，我使用类型名称中的第一个字母 `I` 来表示接口类型。本质上，`di :: bind` 行的意思是，每当我们需要一个具有 `IFoo` 类型成员的组件时，我们就用 `Foo` 的单例实例初始化该组件。

许多人认为，在 `DI` 容器中使用单例是工程实践上唯一可接受的单例用法。至少使用这种方法，如果你需要用其他东西替换单例对象，你可以在一个中心位置进行：容器配置代码。一个额外的好处是你不必自己实现任何单例逻辑，这可以防止可能出现的错误。哦，我有没有提到 `Boost.DI` 是线程安全的？

#### 单态

单态（Monostate）是单例模式的变体。它是一个行为像单例，同时看起来和普通类相同的类。

```c++
class Printer {
  static int id;

 public:
  int get_id() const { return id; }
  void set_id(int value) { id = value; }
};
```

你能看到这里发生了什么吗？该类看起来像一个带有 getter 和 setter 的普通类，但它们实际上处理静态数据！

这似乎是一个非常巧妙的技巧：你让人们实例化`Printer`，但他们都引用相同的数据。但是，用户应该如何知道这一点？用户会很高兴地实例化两台打印机，为它们分配不同的 ID，并且当它们完全相同时会感到非常惊讶！


单态方法在某种程度上有效，并且有几个的优势。例如，它很容易继承，它可以利用多态性，并且它的生命周期被合理定义（但话说回来，你可能并不总是希望如此）。它最大的优点是你可以使用一个已经在整个系统中使用的现有对象，修补它以使其以单态方式运行，并且如果你的系统能够很好地处理非多个对象实例，你就会得到一个类似单例的对象无需重写额外代码即可实现。

这样做的缺点也很明显：它是一种侵入性方法（将普通对象转换为单态并不容易），并且它使用静态成员意味着它总是占用空间，即使不需要它。最终，单态最大的缺点是它做出了一个非常乐观的假设，即类字段总是通过 `getter` 和 `setter` 公开。如果直接访问它们，你的重构几乎注定要失败。


#### 总结

单例并非完全邪恶，但如果使用不慎，它们会破坏应用程序的可测试性和可重构性。如果你确实必须使用单例，请尝试避免直接使用它（例如，编写 `SomeComponent.getInstance().foo()`），而是继续将其指定为依赖项（例如，构造函数参数），从应用程序中的单个位置满足所有依赖项（例如，控制容器的反转）

