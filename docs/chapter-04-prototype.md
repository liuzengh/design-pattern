### 原型模式

想想你每天都在使用的东西，比如汽车、手机。很有可能，它不是从零开始设计的; 恰恰相反，制造商选择了一个现有的设计，做了一些改进，使它在视觉上有别于旧的设计(以便人们可以炫耀)，并开始销售它，淘汰了旧产品。这是事情的自然状态，在软件世界中，我们也会遇到类似的情况:有时候，不是从头创建一个完整的对象，而是您想获取一个预构造的对象，并使用它的一个副本（比如你的简历）。

这让我们想到了建立原型的想法:一个模型对象，我们可以复制这些副本，定制这些副本，然后使用它们。原型模式的挑战实际上是复制部分，剩下的事情则不是什么大问题。

#### 对象的构建

大多数对象构造都是使用构造函数来完成的。但是，如果已经配置了一个对象，为什么不简单地复制该对象，而不是创建一个相同的对象呢？

来看一个例子：
```c++
Contact john{"John Doe", Address{"123 East Dr", "London", 10}};
Contact jane{"Jane Doe", Address{"123 East Dr", "London", 11}};
```

来看看我们要做的事情。john和jane在同一栋楼工作，但在不同的办公室。而其他的则人可能在
伦敦东德街123号工作，所以如果我们想避免重复初始化地址怎么办，我们该怎么做呢?

事实上，原型模式都是关于对象复制的。当然，我们没有一个统一的方法来复制一个对象，我们会介绍其中的一些方法。

##### 普通直接的复制方式

如果你要复制的是一个值，而你要复制的对象通过值来存储所有东西，那么问题将变得十分简单。以上述例子来说，如果你将Contact类和Address类作如下定义：

```c++
struct Address {
  string street, city;
  int suite;
};
struct Contact {
  string name;
  Address address;
};
```

那么直接进行如下的拷贝是毫无问题的：

```c++
// here is the prototype:
Contact worker{"", Address{"123 East Dr", "London", 0}};
// make a copy of prototype and customize it
Contact john = worker;
john.name = "John Doe";
john.address.suite = 10;
```
令人遗憾的是，这种情况在实际中很少见。例如，Address对象可以是一个指针:

```c++
struct Contact {
  string name;
  Address *address;  // pointer (or e.g., shared_ptr)
};
```
这下就有些麻烦了，因为Contact john = prototype这个赋值语句直接复制了指针，使得现在john和prototype以及原型的所有其他副本都共享相同的地址。

##### 通过拷贝构造复制对象

避免重复的最简单方法是确保在组成对象的所有组成部分(在本例中是Contact和Address)上定义拷贝构造函数。例如，如果我们通过指针来存储address，即：

```c++
struct Contact {
  string name;
  Address* address;
};
```
然后，我们需要创建一个拷贝构造函数，实际上有两种方法可以做到这一点。

第一种方法是这样的:

```c++
Contact(const Contact& other)
    : name{other.name}  //, address{ new Address{*other.address} }
{
  address = new Address(other.address->street, other.address->city,
                        other.address->suite);
}
```

不幸的是，上述方法不够通用。在这种情况下，它当然可以工作(假设Address有一个初始化其所有成员的构造函数)，但如果Address决定将其街道部分分割为一个由街道名称、门牌号和其他信息组成的对象，该怎么办?(那么我们又会有同样的复制问题)。

明智的做法是在Address上也定义一个拷贝构造函数。在我们的例子中可以作如下定义:

```c++
Address(const string& street, const string& city, const int suite)
    : street{street}, city{city}, suite{suite} {}
```

现在我们可以重写Contact的构造函数来解决这个问题了：
```c++
Contact(const Contact& other)
    : name{other.name}, address{new Address{*other.address}} {}
```

这里还有另一个问题:假设你开始使用类似双指针的东西(例如，void**或unique_ptr)即使有ReSharper和CLion这样的功能强大的代码生成工具，此时也不太可能生成正确的代码，所以在这些类型上快速生成代码可能并不总是最优解。

通过坚持使用拷贝构造函数而不生成拷贝赋值操作符，可以在一定程度上减少混乱。另一种选择是抛弃拷贝构造函数，而采用下面的方法：

```c++
template <typename T>
struct Cloneable {
  virtual T clone() const = 0;
};
```

然后继续实现这个接口，并在需要实际副本时调用prototype.clone()。这实际上比拷贝构造函数/赋值能更好地达到目的。

#### 序列化

其他编程语言的设计者也遇到过同样的问题，必须在整个对象图上显式定义复制操作，并很快意识到一个类需要是“平凡可序列化的”——默认情况下，你应该能够获取一个类并例如，将其写入文件，而不必为类添加任何特征（好吧，最多可能是一两个属性）。

为什么这与手头的问题有关？因为如果您可以将某些内容序列化到文件或内存中，那么您就可以反序列化它，保留所有信息，包括所有依赖对象。这不是很方便吗？好...

不幸的是，与其他编程语言不同，c++在序列化方面并没有为我们提供现成的工具。例如，我们不能将一个复杂的对象图序列化到一个文件中。为什么不呢?在其他编程语言中，编译后的二进制文件不仅包括可执行代码，还包括大量的元数据，而且序列化可以通过一种称为反射的特性实现——到目前为止在c++中还没有这种特性。

如果我们想要序列化，那么就像显式复制操作一样，我们需要自己实现它。幸运的是，我们可以使用一个现成的名为 `Boost` 库，而不是胡乱摆弄和思考序列化 `std::string` 的方法。序列化可以帮我们解决一些问题。下面是一个如何向  `Address` 类型添加序列化支持的示例:

```c++
struct Address {
  string street;
  string city;
  int suite;

 private:
  friend class boost::serialization::access;
  template <class Ar>
  void serialize(Ar& ar, const unsigned int version) {
    ar& street;
    ar& city;
    ar& suite;
  }
};
```

这可能看起来有点落后，说实话，但最终的结果是，我们使用&操作符指定了 `Address` 的所有部分，我们将需要写入到保存对象的任何地方。注意，前面的代码是用于保存和加载数据的成员函数。可以告诉`Boost`在保存和加载时执行不同的操作，但这与我们的原型需求不是特别相关。

现在我们也需要对 `Contact` 类进行相同的操作:

```c++
struct Contact {
  string name;
  Address* address = nullptr;

 private:
  friend class boost::serialization::access;
  template <class Ar>
  void serialize(Ar& ar, const unsigned int version) {
    ar& name;
    ar& address;  // no *
  }
};
```

前面的`serialize()`函数的结构或多或少是相同的，但请注意一件有趣的事情:我们仍然将其序列化为 `ar & *` 地址，而没有对指针进行解引用。`Boost`足够智能，可以发现发生了什么，即使 `address` 被设置为 `nullptr`，它也可以很好地序列化/反序列化。

因此，如果你想以这种方式实现原型模式，你需要对可能出现在对象图中的每个可能类型都实现 `serialize()`。但是如果你这样做了，你现在可以做的是定义一种通过序列化/反序列化复制对象的方法:

```c++
auto clone = [](const Contact& c) {
  // 1. Serialize the contact
  ostringstream oss;
  boost::archive::text_oarchive oa(oss);
  oa << c;
  string s = oss.str();
  // 2. Deserialize the contact
  istringstream iss(oss.str());
  boost::archive::text_iarchive ia(iss);
  Contact result;
  ia >> result;
  return result;
};
```

现在，有一个叫 `john`的联系人，你可以简单地写成：

```c++
Contact jane = clone(john);
jane.name = "Jane";  // and so on
```
然后根据你自己的想法来定制 `jane`。


#### 原型工厂

如果你有要复制的预定义对象，你打算在哪里存储它们？全局变量？也许。现在假设我们的公司设有主办公室（main offices）和辅办公室(auxiliary offices)。我们可以像这样声明全局变量：

```c++
Contact main{"", new Address{"123 East Dr", "London", 0}};
Contact aux{"", new Address{"123B East Dr", "London", 0}};
```

例如，我们可以将这些定义放到 `Contact.h` 中，以便任何使用 `Contact` 类的人都可以使用这些全局中的一种
变量并复制它们。但更明智的做法是要有某种专门的类来存储原型和根据需要产生上述原型的定制副本。这将为我们提供额外的灵活性：例如，我们可以创建实用程序函数并生成正确初始化的 `unique_ptrs`:

```c++
struct EmployeeFactory {
  static Contact main;
  static Contact aux;
  static unique_ptr<Contact> NewMainOfficeEmployee(string name, int suite) {
    return NewEmployee(name, suite, main);
  }

  static unique_ptr<Contact> NewAuxOfficeEmployee(string name, int suite) {
    return NewEmployee(name, suite, aux);
  }

 private:
  static unique_ptr<Contact> NewEmployee(string name, int suite,
                                         Contact& proto) {
    auto result = make_unique<Contact>(proto);
    result->name = name;
    result->address->suite = suite;
    return result;
  }
};
```

可以像下面这样使用上面的代码：

```c++
auto john = EmployeeFactory::NewAuxOfficeEmployee("John Doe", 123);
auto jane = EmployeeFactory::NewMainOfficeEmployee("Jane Doe", 125);
```

为什么要使用工厂？好吧，考虑一下我们复制原型然后*忘记*定制它的情况。它将在实际数据所在的位置有一些空白字符串和零。使用我们对工厂的讨论中的方法，例如，我们可以将所有非完全初始化构造函数设为私有，将 `EmployeeFactory` 声明为友元类，然后就可以了——现在 `client` 无法获得部分构造的 `Contact` 对象。

#### 总结

原型设计模式阐释了对象*深度*拷贝的概念，并不需要每次都通过构造函数完整初始化来创建一个对象，可以对创建好的对象进行复制，复制产生的对象和原来的对象互不依赖，稍加修改后就能得到想要的新对象。

在C++中实现原型模式实际上只存在两种方法，且都需要手动实现：

- 在代码中正确的复制对象，即进行深拷贝。可以在拷贝构造函数/拷贝赋值运算符或单独的成员函数中实现。
- 在代码中支持序列化/反序列化，序列化后再进行反序列化实现拷贝。该方法需要额外的计算开销，拷贝频率越高，开销越大。该方法相对于拷贝构造函数的唯一优点是能复用已有的序列化代码。

无论选择哪种方法，都需要做一些额外工作，使用代码生成工具（例如 ReSharper、CLion）是可以减轻这部分的工作量的。如果按值来存储数据，不需要担心什么，因为一般而言并不会出现问题。