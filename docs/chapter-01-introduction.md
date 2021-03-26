### 第一章 介绍

“设计模式” 这个话题听起来很枯燥，从学术上来说，又略显呆滞；老实说，任何能够想象到的编程语言都把这个讲烂了——包括一些编程语言，例如 JavaScript，甚至没有正确的面向对象编程特征！那为什么还要写另一本书呢？

我想这本书存在的主要原因，那就是 C++ 又再次焕发生机。在经历了很长一段时间的停滞之后，它正在进化、成长，尽管它又不得不向后与 C 语言兼容做斗争，但是，好的事情正在发生，尽管不是我们所预期的速度（例如 modules，还有其它东西）。

现在，关于设计模式，我们不应该忘记最初出版的设计模式书籍[^1]，其中的示例是使用 C++ 与 Smalltalk写就的。从那时起，许多编程语言都将设计模式直接融入到语言当中：例如，C# 直接将观察者模式与其对事件的内置支持结合在一起（对应于 event 关键字）。C++ 没有这样实现，至少在语法级别上没有这样做。尽管如此，像诸如 std::function 这样的特性的引入确实使许多编程场景变得更加简单。

注1：Erich Gamma et al., *Design Patterns: Elements of Reusable Object-Oriented Software* (Boston, MA: Addison Wesley, 1994).

同时，设计模式也是一项有趣的研究，比如，如何通过不同复杂程度的技术，不同类型的权衡，来解决一个问题。有些设计模式或多或少是必要的、不可避免的，而其它设计模式更多的是科学上的求知欲（尽管如此，这本书还是会讨论的，因为我是一个完美主义者）。

读者应该意识到，对于某些问题的综合解决方案（例如，观察者模式）通常会导致过度设计；也就是说，创建比大多数典型场景所需的复杂得多的结构。虽然，过度设计具有很多乐趣（嘿嘿，你能真正解决问题，并给同事留下深刻印象），但这往往是不可行的。

#### 预备知识

##### 这本书是为谁写的

这本书被设计的更现代，是对经典 GoF 书的更新的，尤其针对 C++ 编程语言。我的意识是，你们中还有多少人在写 Smalltalk？不是很多，这是我的猜测。

这本书的目的是研究如何将现代 C++（目前，可用的 C++ 最新版本）应用于经典设计模式的实现。与此同时，它也是尝试实现任何新模式、新方法，只要是有利于 C++ 开发人员的。

最后，在一些地方，这本书只是现代 C++ 的一个十分简单的技术演示，展示了它的一些最新特性（例如，coroutines）是如何使难题变得更容易解决。

##### 代码示例

本书中的示例都适合于投入到生产环境中，但是，为了便于阅读，我们做了一些简化：

- 经常的，你会发现我使用 struct 去替代 class，仅仅为了避免在太多的地方书写 public 关键字。
- 我将避免使用 std:: 前缀，因为它会损害可读性，特别是在代码密度很高的地方。如果我使用了 string，你可以打赌我指的是 std::string。
- 我将避免添加虚析构函数，而在现实生活中，添加它们可能是有意义的。
- 在极少数情况下，我将按值创建、传递参数，以避免 shared_ptr、 make_shared 等等的扩散。智能指针增加了另外一个层次的复杂度，将它们集成到本书中介绍的设计模式中，作为一个练习留给读者。
- 我有时会省略一些代码元素，它们对于完成一个类型的功能是必要的（例如，移动构造函数），因为这些元素占用了太多的空间。
- 在很多情况下，我会忽略 const 关键字；在正常情况下，这实际上是有意义的。const 正确性通常会导致 api 表面上的分裂、加倍，这在书的格式中不能很好地工作。

你应该意识到，大多数示例都使用了现代 C++（C++11、14、17和更高版本），并且，开发人员通常可以使用最新的 C++ 语言特性。例如，当 C++14 允许我们自动推断返回值类型时，你将不会发现许多函数签名以 -> decltype(...) 为结尾。这些示例中没有一个是针对特定编译器的，但是如果你选择的编译器[^2]不能正常工作，你需要找到解决办法。

注2：Intel, I'm looking at you!

在某些情况下，我将引用其它编程语言，比如 C# 或者 Kotlin。有时值得注意的是，其它语言设计者是如何实现特定功能的？对于 C++ 来说，从其它语言借鉴一般可用的想法并不陌生：例如，在许多其它语言中，引入了 auto 关键字用于变量声明和返回类型的自动推断。

##### 开发工具

本书中编写的代码示例是用于现代 C++ 编译器的，像 Clang，GCC 或者 MSVC。我一般假设你使用的是可用的最新编译器版本，因此，将使用我可以使用的最新、最优秀的语言特性。在某些情况下，高级语言示例对于早期编译器需要降级使用；而在其它情况下，则可能无法实现。

就开发人员的工具而言，这本书没有具体涉及到它们，因此，如果你有一个最新的编译器，你应该很好地遵循这些示例：它们中的大多数都是自包含的 .cpp 文件。尽管如此，我还是想借此机会提醒你，诸如 CLion 或 ReSharper C++ 之类的质量开发人员工具极大地提高了开发体验。只要你投资一小笔钱，你就可以获得大量的额外功能，这些功能可以直接转化为编码速度和代码质量的提高。

##### 盗版

数字盗版是一个不可逃避的事实。一个崭新的一代正在成长，从来没有购买过一部电影或一本书籍，甚至这本书。这也没什么可做的。我唯一能说的是，如果你翻版这本书，你可能不会读最新的版本。

在线数字出版的乐趣在于，我可以把这本书更新为 C++ 的最新版本，我也可以做更多的研究。因此，如果你为这本书付费，当 C++ 语言和标准库的新版本发布时，你将在将来获得免费的更新。如果不付费，哦，好吧...

#### 重要的概念

 在我们开始之前，我想简单地提及在这本书中将要引用的 C++ 世界的一些关键概念。

##### 奇异递归模板模式（CRTP）

嗨，很显然，这是一个模式！我不知道它是否有资格被列为一个独立的设计模式，但是，它肯定是 C++ 世界中的一个模式。从本质上说，这个想法很简单：继承者将自己作为模板参数传递给它的基类：

```c++
struct Foo : SomeBase<Foo>
{
	...
}
```

现在，您可能想知道为什么有人会这么做？原因之一是，以便于能够访问基类实现中的类型化 this 指针。

例如，假设 SomeBase 的每个继承者都实现了迭代所需的 begin()/end() 对。那么，您将如何在 SomeBase 的成员中迭代该对象？直觉表明，您不能这样做，因为 SomeBase 本身没有提供 begin()/end() 接口。但是，如果您使用 CRTP，实际上是可以将 this 转换为派生类类型：

```c++
template <typename Derived>
struct SomeBase
{
    void foo()
    {
        for (auto& item : *static_cast<Derived*>(this))
        {
            ...
        }
    }
}
```

有关此方法的具体示例，请参阅第 9 章。

##### 混合继承

在 C++ 中，类可以定义为继承自它自己的模板参数，例如：

```c++
template <typename T> struct Mixin : T
{
	...
}
```

这种方法被称为混合继承（*mixin inheritance*），并允许类型的分层组合。例如，您可以允许 Foo\<Bar\<Baz>> x; 声明一个实现所有三个类的特征的类型的变量，而不必实际构造一个全新的 FooBarBaz 类型。

有关此方法的具体示例，请参阅第 9 章。

##### 属性

一个属性（*property*，通常是私有的）仅仅是字段以及 getter 和 setter 的组合。在标准 C++ 中，一个属性如下所示：

```c++
class Person
{
    int age;
public:
    int get_age() const { return age; }
    void set_age(int value) { age = value; }
};
```

大量的编程语言（例如，C#、Kotlin）通过直接将其添加到编程语言中，将属性的概念内化。虽然 C++ 没有这样做（而且将来也不太可能这样做），但是有一个名为 property 的非标准声明说明符，您可以在大多数编译器（MSVC、Clang、Intel）中使用：

```c++
class Person
{
    int age_;
public:
    int get_age() const { return age_; }
    void set_age(int value) { age_ = value; }
    __declspec(property(get=get_age, put=set_age)) int age;
};
```

这可以按如下所示使用：

```c++
Person person;
p.age = 20; // calls p.set_age(20)
```

#### SOLID 设计原则

SOLID 是一个首字母缩写，代表以下设计原则（及其缩写）：

- 单一责任原则（SRP）
- 开闭原则（OCP）
- 里氏替换原则（LSP）
- 接口隔离原则（ISP）
- 依赖注入原则（DIP）

这些原则是由 Robert C. Martin 在 2000 年代初期引入的；事实上，它们只是从 Robert 的书和他的博客中表述的几十项原则中选出的五项原则。这五个特定主题一般都渗透了对模式和软件设计的讨论，所以在我们深入到设计模式之前（我知道你都非常渴望），我们将做一个简短的回顾关于 SOLID 的原则是什么。

##### 单一职责原则

假设您决定把您最私密的想法记在日记里。日记具有一个标题和多个条目。您可以按如下方式对其进行建模：

```c++
struct Journal
{
    string title;
    vector<string> entries;
    
    explicit Journal(const string& title) : title{title} {}
};
```

现在，您可以添加用于将添加到日志中的功能，并以日记中的条目序号为前缀。这很容易：

```c++
void Journal::add(const string& entry)
{
    static int count = 1;
    entries.push_back(boost::lexical_cast<string>(count++)
        + ": " + entry);
}
```

现在，该日记可用于：

```c++
Journal j{"Dear Diary"};
j.add("I cried today");
j.add("I ate a bug");
```

因为添加一条日记条目是日记实际上需要做的事情，所以将此函数作为 Journal 类的一部分是有意义的。这是日记的责任来保持条目，所以，与这相关的任何事情都是公平的游戏。

现在，假设您决定通过将日记保存在文件中而保留该日记。您需要将此代码添加到 Journal 类：

```c++
void Journal::save(const string& filename)
{
    ofstream ofs(filename);
    for (auto& s : entries)
        ofs << s << endl;
}
```

这种方法是有问题的。日志的责任是保存日志条目，而不是把它们写道磁盘上。如果您将磁盘写入功能添加到 Journal 和类似类中，持久化方法中的任何更改（例如，您决定向云写入而不是磁盘），都将在每个受影响的类中需要进行大量的微小的更改。

我想在这里停顿一下，并指出：一个架构，使您不得不在大量的类中做很多微小的更改，无论是否相关（如在层次结构中），通常都是一种代码气味（*code smell*）——一个不太对劲的迹象。现在，这完全取决于情况：如果你要重命名一个在 100 个地方使用的符号，我认为这通常是可以的，因为 ReSharper、CLion 或任何你使用的 IDE 实际上将允许你执行重构并且将更改到处传播。但是当你需要完全修改接口时...嗯，那可能是一个非常痛苦的过程！

因此，我指出，持久化是一个单独的问题，最好在一个单独的类别中表达，例如：

```c++
struct PersistenceManager
{
    static void save(const Journal& j, const string& filename)
    {
        ofstream ofs(filename);
        for (auto& s: j.entries)
            ofs << s << endl;
    }
};
```

这正是单一责任（*Single Responsibility*）的含义：每个类只有一个责任，因此，只有一个改变的理由。只有在需要对条目的存储做更多工作的情况下，Journal 才需要更改。例如，你可能希望每个条目都以时间戳为前缀，因此，你将更改 add() 函数来实现这一点。从另一方面来说，如果你要更改持久化机制，这将在 PersistenceManager 中进行更改。

一个违反 SRP 的反模式的极端例子被称为上帝对象（God Object）。上帝对象是一个巨大的类，它试图处理尽可能多的问题，称为一个难以处理的巨大怪物。

幸运的是，对于我们来说，上帝对象很容易识别出来，并且由于有了源代码管理系统（只需要计算成员函数的数量），负责的开发人员可以迅速确定并受到适当的惩罚。

##### 开闭原则

假设在数据库中，我们拥有一个（完全假设的）范围的产品。每种产品具有颜色和尺寸，并定义为：

```c++
enum class Color { Red, Green, Blue };
enum class Size { Small, Medium, Large };

struct Product
{
    string name;
    Color color;
    Size size;
};
```

现在，我们希望为给定的一组产品提供特定的过滤功能。我们制作了一个类似于以下内容的过滤器：

```c++
struct ProductFilter
{
    typedef vector<Product*> Items;
};
```

现在，为了支持通过颜色过滤产品，我们定义了一个成员函数，以精确地执行以下操作：

```c++
ProductFilter::Items ProductFilter::by_color(Items items, Color color)
{
    Items result;
    for (auto& i : items)
        if (i->color == color)
            result.push_back(i);
    return result;
}
```

我们目前按颜色过滤项目的方法都很好，而且很好。我们的代码开始进入生产环节，但不幸的是，一段时间之后，老板进来并要求我们实现按尺寸大小进行过滤。因此，我们跳回 ProductFilter.cpp 添加以下代码并重新编译：

```c++
ProductFilter::Items ProductFilter::by_size(Items items, Size size)
{
    Items result;
    for (auto& i : items)
        if (i->size == size)
            result.push_back(i);
    return result;
}
```

这感觉像是彻底的复制，不是吗？为什么我们不直接编写一个接受谓词（一些函数）的通用方法呢？嗯，一个原因可能是不同形式的过滤可以以不同的方式进行：例如，某些记录类型可能被编入索引，需要以特定的方式进行搜索；某些数据类型可以在 GPU 上搜索，而其它数据类型则不适用。

我们的代码进入生成环节，但是，再次的，老板回来告诉我们，现在有一个需求需要按颜色和尺寸进行搜索。那么，我们要做什么呢，还是增加另一个函数？

```c++
ProductFilter::Items ProductFilter::by_color_and_size(Items items, Size size, Color color)
{
    Items result;
    for (auto& i : items)
        if (i->size == size && i->color == color)
            result.push_back(i);
    return result;
}
```

从前面的场景中，我们想要的是实现“开放-关闭原则”（*Open-Closed Principle*），该原则声明类型是为了扩展而开放的，但为修改而关闭的。换句话说，我们希望过滤是可扩展的（可能在另一个编译单元中），而不必修改它（并且重新编译已经工作并可能已经发送给客户的内容）。

我们如何做到这一点？首先，我们从概念上（SRP!）将我们的过滤过程分为两部分：筛选器（接受所有项并且只返回某些项的过程）和规范（应用于数据元素的谓词的定义）。

我们可以对规范接口做一个非常简单地定义：

```c++
template <typename T> struct Specification
{
    virtual bool is_satisfied(T* item) = 0;
};
```

在前面的示例中，类型 T 是我们选择的任何类型：它当然可以是一个 Product，但也可以是其它东西。这使得整个方法可重复使用。

接下来，我们需要一种基于 Specification\<T> 的过滤方法：你猜到的，这是通过定义完成，一个 Filter\<T>：

```c++
template <typename T> struct Filter
{
    virtual vector<T*> filter(
    	vector<T*> items,
    	Specification<T>& spec) = 0;
};
```

同样的，我们所做的就是为一个名为 filter 的函数指定签名，该函数接受所有项目和一个规范，并返回符合规范的所有项目。假设这些项目被存储为 vector<T*>，但实际上，你可以向 filter() 传递，或者是一对迭代器，或者是一些专门为遍历集合而设计的定制接口。遗憾的是，C++ 语言未能标准化枚举或集合的概念，这是存在于其它编程语言（例如，.NET 的 IEnumerable）中的东西。

基于前述，改进的过滤器的实现非常的简单：

```c++
struct BetterFilter : Filter<Product>
{
	vector<Product*> filter(
		vector<Product*> items,
		Specification<Product>& spec) override
	{
		vector<Product*> result;
		for (auto& p : items)
			if (spec.is_satisfied(p))
				result.push_back(p);
		return result;
	}
};
```

再次，你可以想到 Specification\<T>，该规范被传入作为 std::function 的强类型化等效项，该函数仅约束到一定数量的可能的筛选规格。

现在，这是最简单的部分。为了制作一个颜色过滤器，你可以制作一个 ColorSpecification：

```c++
struct ColorSpecification : Specification<Product>
{
	Color color;
	
	explicit ColorSpecification(const Color color) :
    color{color} {}
	
	bool is_satisfied(Product* item) override {
		return item->color == color;
	}
};
```

根据本规范，以及给定的产品清单，我们现在可以按如下方式过滤这些产品：

```c++
Product apple{ "Apple", Color::Green, Size::Small };
Product tree{ "Tree", Color::Green, Size::Large };
Product house{ "House", Color::Blue, Size::Large };

vector<Product*> all{ &apple, &tree, &house };

BetterFilter bf;
ColorSpecification green(Color::Green);

auto green_things = bf.filter(all, green);
for (auto& x : green_things)
    cout << x->name << " is green" << endl;
```

前面给我们的是 “Apple” 和 “Tree”，因为它们都是绿色的。现在，我们迄今为止尚未实现的唯一目标是搜索尺寸和颜色（或者，实际上，解释了如何搜索尺寸或颜色，或混合不同的标准）。答案是你简单地做了一个复合规范。例如，对于逻辑 AND，你可以使其如下所示：

```c++
template <typename T> struct AndSpecification :
Specification<T>
{
	Specification<T>& first;
	Specification<T>& second;
	
	AndSpecification(Specification<T>& first,
                     Specification<T>& second)
		: first{first}, second{second} {}
		
	bool is_satisfied(T* item) override
	{
		return first.is_satisfied(item) && second.is_satisfied(item);
	}
};
```

现在，你可以在更简单的规范基础上创建复合条件。复用我们早期制作的绿色规范，找到一些绿色和大的东西现在就像这样简单：

```c++
SizeSpecification large(Size::Large);
ColorSpecification green(Color::Green);
AndSpecification<Product> green_and_large{ large, green };

auto big_green_things = bf.filter(all, green_and_big);
for (auto& x : big_green_things)
    cout << x->name << " is large and green" << endl;

// Tree is large and green
```

这里有很多代码！但是请记住，由于 C++ 的强大功能，你可以简单地引入一个 operator && 用于两个 Specification\<T> 对象，从而使得过滤过程由两个（或更多！）标准，极为简单：

```c++
template <typename T> struct Specification
{
    virtual bool is_satisfied(T* item) = 0;
    
    AndSpecification<T> operator &&(Specification&& other)
    {
        return AndSpecification<T>(*this, other);
    }
};
```

如果你现在避免为尺寸/颜色规范设置额外的变量，则可以将复合规范简化为一行：

```c++
auto green_and_big =
    ColorSpecification(Color::Green)
    && SizeSpecification(Size::Large);
```

因此，让我们回顾以下 OCP 原则是声明，以及前面的示例是如何执行它的。基本上，OCP 声明你不需要返回你已经编写和测试过的代码，并对其进行更改。这正是这里发生的！我们制定了 Specification\<T> 和 Filter\<T>，从那时起，我们所要做的就是实现任何一个接口（不需要修改接口本身）来实现新的过滤机制。这就是“开放供扩展，封闭供修改”的意思。

##### 里氏替换原则

里氏替换原则（以 Barbara Liskov 命名）指出，如果一个接口可以接受类型为 Parent 的对象，那么它应该同样地可以接受类型为 Child 的对象，而不会有任何破坏。让我们来看看 LSP 被破坏的情况。

下面是一个矩形；它有宽度（width）和高度（height），以及一组计算面积的 getters 和 setters：

```c++
class Rectangle
{
protected:
    int width, height;
public:
    Rectangle(const int width, const int height)
        : width{width}, height{height} { }
    
    int get_width() const { return width; }
    virtual void set_width(const int width) { this->width = width; }
    int get_height() const { return height; }
    virtual void set_height(const int height) { this->height = height; }
    
    int area() const { return width * height; }
};
```

现在，假设我们有一种特殊的矩形，称为正方形。此对象将重写 setters，以设置宽度和高度：

```c++
class Square : public Rectangle
{
public:
	Square(int size) : Rectangle(size, size) {}
	void set_width(const int width) override {
		this->width = height = width;
	}
	void set_height(const int height) override {
		this->height = width = height;
	}
};
```

这种做法是邪恶的。你还看不到它，因为它确实是无辜的：setters 简单地设置了两个维度，可能会发生什么错误呢？好吧，如果我们采用前面的方法，我们可以很容易地构建一个函数，该函数以 Rectangle 类型变量为参数，当传入 Square 类型变量时，它会爆炸： 

```c++
void process(Rectangle& r)
{
    int w = r.get_width();
    r.set_height(10);
    
    cout << "expected area = " << (w * 10)
        << ", got " << r.area() << endl;
}
```

前面的函数以公式 Area = Width * Height 作为不变量。它得到宽度，设置高度，并正确地期望乘积等于计算的面积。但是使用 Square 调用前面的函数会产生不匹配：

```c++
Square s{5};
process(s); // expected area = 50, got 25
```

从这个例子（我承认有点人为的）得到的启示是，process() 完全不能接受派生类型 Square 而不是基类型 Rectangle，从而破坏了 LSP 原则。如果你给它一个 Rectangle，一切都很好，所以它可能需要一些时间才能出现在你的测试（或者生产，希望不是！）。

解决办法是什么呢？嗯，有很多。就我个人而言，我认为类型 Square 甚至不应该存在：相反，我们可以创建一个工厂（参见第3章）来创建矩形和正方形：

```c++
struct RectangleFactory
{
    static Rectangle create_rectangle(int w, int h);
    static Rectangle create_square(int size);
};
```

你也可能需要一种检测一个 Rectangle 是否是一个 Square 的方法：

```c++
bool Rectangle::is_square() const
{
    return width == height;
}
```

在这种情况下，核心选项是在 Square 的 set_width() / set_height() 中抛出一个异常，说明这些操作不受支持，你应该使用 set_size() 代替。但是，这违反了最小覆盖的原则（ *principle of least surpise*），因为你希望调用 set_width() 来进行有意义的更改...我说的对吗？

##### 接口分离原则

好吧，这是另一个人为的例子，尽管如此，它仍然适合于说明这个问题。假设你决定定义一个多功能打印机：该设备可以打印、扫描和传真文档。因此，你可以定义如下：

```c++
struct MyFavouritePrinter /* : IMachine */
{
    void print(vector<Document*> docs) override;
    void fax(vector<Document*> docs) override;
    void scan(vector<Document*> docs) override;
};
```

这很好。现在，假设你决定定义一个需要由所有计划制造多功能打印机的人实现的接口。因此，你可以在你最喜欢的 IDE 中使用提取接口函数功能，你可以得到如下内容：

```c++
struct IMachine
{
    virtual void print(vector<Document*> docs) = 0;
    virtual void fax(vector<Document*> docs) = 0;
    virtual void scan(vector<Document*> docs) = 0;
};
```

这里有一个问题。原因是这个接口的一些实现者可能不需要扫描或传真，只需要打印。然而，你强迫他们实现这些额外的功能：当然，它们可以都是无操作的，但为什么要这么做呢？

因此，ISP 的建议是将接口分开，以便于实现者可以根据他们的需求进行挑选和选择。由于打印和扫描是不同的操作（例如，扫描仪不能打印），我们为这些操作定义了不同的接口：

```c++
struct IPrinter
{
    virtual void print(vector<Document*> docs) = 0;
};

struct IScanner
{
    virtual void scan(vector<Document*> docs) = 0;
};
```

然后，打印机或扫描仪可以实现所需的功能：

```c++
struct Printer : IPrinter
{
	void print(vector<Document*> docs) override;
};

struct Scanner : IScanner
{
	void scan(vector<Document*> docs) override;
};
```

现在，如果我们真的想要一个 IMachine 接口，我们可以将它定义为上述接口的组合：

```c++
struct IMachine : IPrinter, IScanner /* IFax and so on */
{  
};
```

当你在具体的多功能设备中实现这个接口时，这就是要使用的接口。例如，你可以使用简单的委托来确保 Machine 重用特定 IPrinter 和 IScanner 提供的功能：

```c++
struct Machine : IMachine
{
	IPrinter& printer;
	IScanner& scanner;
	
	Machine(IPrinter& printer, IScanner& scanner)
		: printer{printer},
		  scanner{scanner}
	{
	}
	
	void print(vector<Document*> docs) override
	{
		printer.print(docs);
	}
	
	void scan(vector<Document*> docs) override
	{
		scanner.scan(docs);
	}
};
```

因此，简单地说，这里的想法是将复杂接口的部分分隔成单独的接口，以避免迫使实现者实现他们并不真正需要的功能。当为某些复杂的应用程序编写插件时，如果你得到一个具有 20 个令人困惑的函数的接口，用于实现各种 no-ops 和 return nullptr 时，说不定是 API 作者违反了 ISP 原则。

##### 依赖反转原则

DIP 的原始定义如下所示[^3] ：

*A. High-level modules should not depend on low-level modules. Both should depend on abstractions*. 

注3：Martin, Robert C., *Agile Software Development, Principles, Patterns, and Practices* (New York: Prentice Hall, 2003), pp. 127-131.

这句话的主要意思是，如果你对日志记录感兴趣，你的报告组件不应该依赖于具体的 ConsoleLogger，而是可以依赖于 ILogger 接口。在这种情况下，我们认为报告组件是高级别的（更接近业务领域），而日志记录则是一个基本的关注点（类似于文件 I/O 或线程，但不是），被认为是一个低级别的模块。

*B. Abstractions should not depend on details. Details should depend on abstractions.*

这再次重申了接口或基类上的依赖比依赖于具体的类型更好。希望这个语句的真实性是显而易见的，因为这种方法支持更好的可配置性和可测试性——前提是你使用了一个良好的框架来处理这些依赖关系。

所以，现在的主要问题是：你是如何真正实现上述所有的？这确实需要更多的工作，因为现在你需要明确说明，例如，Reporting 依赖于 ILogger。你表达它的方式也许如下所示：

```c++
class Reporting
{
    ILogger& logger;
public:
    Reporting(const ILogger& logger) : logger{logger} {}
    void prepare_report()
    {
        logger.log_info("Preparing the report");
        ...
    }
};
```

现在的问题是，要初始化前面的类，你需要显式地调用 Reporting{ConsoleLogger{}} 或类似地东西。如果 Reporting 依赖于五个不同的接口呢？如果 ConsoleLogger 有自己的依赖项，又怎么办？你可以通过编写大量的代码来管理这个问题，但是这里有一个更好的方法。

针对前面的现代、流行、时尚的做法是使用依赖注入（*Dependency Injection*）：这基本上意味着你要使用诸如 Boost.DI[^4]之类的库自动满足特定组件的依赖关系的要求。

注4：At the moment, Boost.DI is not yet part of Boost proper, it is part of the boost-experimental Github repository.

让我们考虑一个具有引擎但还需要写入日志的汽车的例子。从目前的情况来看，我们可以说一辆车取决于这两件情况。首先，我们可以将引擎定义为：

```c++
struct Engine
{
    float volume = 5;
    int horse_power = 400;
    
    friend ostream& operator<< (ostream& os, const Engine& obj)
    {
        return os
            << "volume: " << obj.volume
            << "horse_power: " << obj.horse_power;
    } // thanks, ReSharper!
};
```

现在，由我们决定是否要提取一个 IEngine 接口并将其馈送到汽车。也许我们有，也许我们没有，这通常是一个设计决定。如果你设想有一个引擎层次结构，或者你预见到为了测试目的需要一个 NullEngine（参见第19章），那么是的，你确实需要抽象出接口。

无论如何，我们也需要日志记录，因为这可以通过多种方式完成（控制台、电子邮件、短信、鸽子邮件...），我们可能希望有一个 ILogger 接口：

```c++
struct ILogger
{
    virtual ~ILogger() {}
    virtual void Log(const string& s) = 0;
};
```

以及某种具体的实现：

```c++
struct ConsoleLogger : ILogger
{
	ConsoleLogger() {}
	
	void Log(const string& s) override
	{
		cout << "LOG: " << s.c_str() << endl;
	}
};
```

现在，我们将要定义的汽车取决于引擎和日志组件。我们两者都需要，但这取决于我们如何存储它们：我们可以使用指针，引用，unique_ptr/shared_ptr 或其它。我们将这两个依赖组件定义为构造函数的参数：

```c++
struct Car
{
    unique_ptr<Engine> engine;
    shared_ptr<ILogger> logger;
    
    Car(unique_ptr<Engine> engine,
        const shared_ptr<ILogger>& logger)
      : engine{move(engine)},
        logger{logger}
    {
        logger->Log("making a car");
    }
    
    friend ostream& operator<<(ostream& os, const Car& obj)
    {
        return os << "car with engine: " << *obj.engine;
    }
};
```

现在，你可能希望在初始化 Car 时看到对 make_unique/make_shared 的调用。但我们不会这么做的。相反，我们将使用 Boost.DI。首先，我们将定义一个绑定，将 ILogger 绑定到 ConsoleLogger；这意味着，只要有人要求一个 ILogger，就给他们一个 ConsoleLogger：

```c++
auto injector = di::make_injector(
	di::bind<ILogger>().to<ConsoleLogger>()
);
```

现在，我们已经配置了注射器，我们可以使用它来创建一辆汽车：

```c++
auto car = injector.create<shared_ptr<Car>>();
```

前面的内容创建了一个 shared_ptr\<Car>，它指向了一个完全初始化的 Car 对象，这正是我们想要的。这种方法的伟大之处在于，如果需要更改正在使用的记录器的类型，我们可以在一个地方（绑定调用）更改它，而 ILogger 出现的每个地方现在都可以使用我们提供的其它日志记录组件。这种方法还可以帮助我们进行单元测试，并允许我们使用桩（或 Null 对象模式）代替模拟。

##### 模式时间!

通过对 SOLID 设计原则的理解，我们将深入到设计模式本身。请系好安全带，这将是一段漫长的旅程（希望不会很无聊）。

