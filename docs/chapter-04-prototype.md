### 原型模式

原型模式的作用在于能够高效地创建重复的对象，本质上是一种深拷贝的实现。

想想你每天都在使用的东西，比如汽车、手机。很有可能，它不是从零开始设计的; 恰恰相反，制造商选择了一个现有的设计，做了一些改进，使它在视觉上有别于旧的设计(以便人们可以炫耀)，并开始销售它，淘汰了旧产品。这是事情的自然状态，在软件世界中，我们也会遇到类似的情况:有时候，不是从头创建一个完整的对象，而是您想获取一个预构造的对象，并使用它的一个副本（比如你的简历）。

这让我们想到了建立原型的想法:一个模型对象，我们可以复制这些副本，定制这些副本，然后使用它们。原型模式的挑战实际上是复制部分，剩下的事情则不是什么大问题。

#### 对象的构建

大多数对象构造都是使用构造函数来完成的。但是，如果已经配置了一个对象，为什么不简单地复制该对象，而不是创建一个相同的对象呢？

来看一个例子：
```
1 Contact john{ "John Doe", Address{"123 East Dr", 
"London", 10 } };

2 Contact jane{ "Jane Doe", Address{"123 East Dr",
"London", 11 } };
```

来看看我们要做的事情。john和jane在同一栋楼工作，但在不同的办公室。而其他的则人可能在
伦敦东德街123号工作，所以如果我们想避免重复初始化地址怎么办，我们该怎么做呢?

事实上，原型模式都是关于对象复制的。当然，我们没有一个统一的方法来复制一个对象，我们会介绍其中的一些方法。

##### 普通直接的复制方式

如果你要复制的是一个值，而你要复制的对象通过值来存储所有东西，那么问题将变得十分简单。以上述例子来说，如果你将Contact类和Address类作如下定义：

```
1  struct Address
2  {
3  string street, city;
4  int suite;
5  }
6  struct Contact
7  {
8  string name;
9  Address address;
10 }
```

那么直接进行如下的拷贝是毫无问题的：
```
1 // here is the prototype:
2 Contact worker{"", Address{"123 East Dr", "London", 0}};
3
4 // make a copy of prototype and customize it
5 Contact john = worker;
6 john.name = "John Doe";
7 john.address.suite = 10;
```
令人遗憾的是，这种情况在实际中很少见。例如，Address对象可以是一个指针:

```
1 struct Contact
2 {
3 string name;
4 Address *address; // pointer (or e.g., shared_ptr)
5 }
```
这下就有些麻烦了，因为Contact john = prototype这个赋值语句直接复制了指针，使得现在john和prototype以及原型的所有其他副本都共享相同的地址。

##### 通过拷贝构造复制对象

避免重复的最简单方法是确保在组成对象的所有组成部分(在本例中是Contact和Address)上定义拷贝构造函数。例如，如果我们通过指针来存储address，即：

```
1 struct Contact
2 {
3 string name;
4 Address* address;
5 }
```
然后，我们需要创建一个复制构造函数，实际上有两种方法可以做到这一点。

第一种方法是这样的:
```
1 Contact(const Contact& other)
2 : name{other.name}
3 //, address{ new Address{*other.address} }
4 {
5 address = new Address(
6 other.address->street,
7 other.address->city,
8 other.address->suite
9 );
10 }
```

不幸的是，上述方法不够通用。在这种情况下，它当然可以工作(假设Address有一个初始化其所有成员的构造函数)，但如果Address决定将其街道部分分割为一个由街道名称、门牌号和其他信息组成的对象，该怎么办?(那么我们又会有同样的复制问题)。

明智的做法是在Address上也定义一个复制构造函数。在我们的例子中可以作如下定义:
```
1 Address(const string& street, const string& city, const int
suite)
2 : street{street},
3 city{city},
4 suite{suite} {}
```

现在我们可以重写Contact的构造函数来解决这个问题了：
```
1 Contact(const Contact& other)
2 : name{other.name}
3 , address{ new Address{*other.address} }
4 {}
```

这里还有另一个问题:假设你开始使用类似双指针的东西(例如，void**或unique_ptr)即使有ReSharper和CLion这样的功能强大的代码生成工具，此时也不太可能生成正确的代码，所以在这些类型上快速生成代码可能并不总是最优解。

通过坚持使用复制构造函数而不生成复制赋值操作符，可以在一定程度上减少混乱。另一种选择是抛弃复制构造函数，而采用下面的方法：
```
1 template <typename T> struct Cloneable
2 {
3   virtual T clone() const = 0;
4 }
```

然后继续实现这个接口，并在需要实际副本时调用prototype.clone()。这实际上比复制构造函数/赋值更好地达到目的。

