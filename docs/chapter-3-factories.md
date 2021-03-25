### 第三章 工厂

> I had a problem and tried to use Java, now I have a
ProblemFactory.  –Old Java joke.

本章同时覆盖了GoF中的两个设计模式：工厂方法和抽象工厂。因为这两个设计模式是紧密相关的，所以接下来我们讲它们合在一起讨论。

#### 场景

让我们从一个实际的例子开始吧！假设你想要存储笛卡尔空间(Cartesian space)下某个点的信息，你可以直接了当像下面这样实现：

```c++
struct Point
{
    Point(const float x, const float y):
        x{x}, y{y} {}
    float x, y; // 严格的直角坐标表示
};
```

目前为止，一切都表现良好。但是现在你也想通过极坐标(Polar coordinates )的方式来初始化这个点。你需要提供另外版本的构造函数：

```c++
Point(const float r, const float theta)
{
    x = r * cos(theta);
    y = r * sin(theta);
}
```

但是不幸的是，上面的两个构造函数的函数签名是相同的[^1]，在c++语言里面是不允许这样的函数重载出现的。那么你打算怎么做呢？一种方法是引入一个枚举类型：

注1：一些编程语言，尤其是Objective-C和Swift，确实允许重载仅因参数名不同的函数。不幸的是，这种想法导致了所有调用中参数名的病毒式传播。大多数时候，我仍然更喜欢位置参数。

```c++
enum class PointType
{
    cartesian
    polar
};
```

在原来的构造函数上增加一个额外的参数。

```c++
Point(float a, float b, PointType type = PointType::cartesian)
{
    if(type == PointType::cartesian)
    {
        x = a;
        y = b;
    }
    else
    {
        x = a * cos(b);
        y = a * sin(b);
    }
}
```

注意构造函数的前两个参数的名称被为了a和b， 我们无法告诉用户a和b的值应该来自哪个坐标系。与使用x、y、rho和theta这种清晰的名称来传达构造意图相比，这显然缺乏表现力。

总之，我们的构造函数的设计在功能上是可用的，但是在形式上是丑陋的。接下来让我们看看能否在此基础上改进。


#### 工厂方法

上述构造函数的问题在于它的名称总是与类型匹配。这与普通函数不同，意味着我们不能用它来传递任何额外的信息。同时，给定名字总是相同的，我们不能有两个重载一个取x, y；而另一个取r, theta。

那么我们该怎么做呢？ 把构造函数放入保护字段，然后暴露一些静态方法用于创建新的坐标点怎么样?

```c++
class Point
{
    protected:
        Point(const float x, const float y):
            x{x}, y{y} {}
    public:
        static Point NewCartesian(float x, float y) 
        {
            return {x, y};
        }
        static Point NewPloar(float r, float theta) 
        {
            return {r * cos(theta), r * sin(theta)};
        }
        // 其他的成员
    private:
        float    x;
        float    y;
}
```

上面每一个静态方法都被称为**工厂方法**。它所做的只是创建一个点并返回它，其优点是方法的名称和参数的名称清楚地传达了需要哪种坐标。现在创建一个点，我们可以简单的这样写：

```c++
auto = Point::NewPloar(5, M_PI_4)
```

从上面的代码看出，我们清楚的意识到我们正在用极坐标下$r=5, theta =\pi/4$的方式创建一个新的点。

#### 工厂

就和建造者模式(Builder Pattern)一样，我们可以把Point类中所有的构建方法都从Point类中提取出来放置到单独一个类中，我么称之为该类为**工厂**。首先我么重新定义Point类：

```c++
struct Point
{
    float x, y;
    friend class PointFactory;
    private:
        Point(float x, float y):
            x(x), y(y) {};
};
```

以上代码中有两点需要注意：

- Point类的构造函数被放置在私有段，因为我么不希望其他类能直接调用该方法。这并不是一个严格的要求，但是将其声明为公有的，会造成一些歧义，因为它向用户提供了另外一种构造对象的方法，在这里的设计中，我们希望用户只能调用PointFactory中的方法构造对象。
- Point中声明了PointFactory为其友元类。这样是有意为之的，以便Point的私有构造函数对PointFactory可用，否则，工厂将无法实例化对象！这里暗示这以上两种类型都是同时被创建的，而不在Point创建之后很久才创建工厂。
现在我们只需在另外一个名为PointFactory的类中定义我么的NewXxx()函数:

```c++
struct PointFactory
{
    static Point NewCartesian(float x, float y)
    {
        return Point{x, y};
    }
    static Point NewPolar(float r, float theta)
    {
        return Point{ r*cos(theta), r*sin(theta); };
    }
};
```
这样而来, 我们现在有了一个专门为创建点的实例而设计的类，可以像下面这样使用它：

```c++
auto PointFactory::NewCatesian(3, 4);
```

#### 内部工厂

内部工厂也是一个工厂，只不过它是它创建的类型的内部类。公平地说，内部工厂是c#、Java和其他缺乏friend关键字的语言的典型组件，但没有理由不能在c++中使用它。

内部工厂存在的原因是，内部类可以访问外部类的私有成员，反过来，外部类也可以访问内部类的私有成员。这意味着我们的Point类也可以这样定义:

```c++
struct Point
{
    private:
        Point(float x, float y):
            x(x), y(y) {}
    
    struct PointFactory
    {
        static Point NewCartesian(float x, float y)
        {
            return Point{x, y};
        }
        static Point NewPolar(float r, float theta)
        {
            return Point{ r*cos(theta), r*sin(theta); };
        }
    };

    public:
        float x, y;
        static PointFactory Factory;      
};
```
好的，让我们来看发生了什么？我们把工厂类放到了工厂创建的类中。如果一个工厂只工作于一个单一类型，那么这很方便，而如果一个工厂依赖于多个类型，那么就不那么方便了（如果还需要其他类的的私有成员，那就几乎不可能了）。

您会注意到我在这里有点狡猾:整个工厂都在一个私有块中，而且它的构造函数也被标记为私有。本质上，即使我们可以将这个工厂暴露为Point::PointFactory，这也相当拗口。相反，我定义了一个名为Factory的静态成员。这允许我们使用工厂作为

```c++
auto pp = Point::Factory.NewCartesian(2, 3);
```

当然，如果出于某种原因，您不喜欢混合使用::和.，您可以向下面这样更改代码，以便在任何地方使用::。

- 将工厂公开，这允许你编写如下形式的代码：
   
```c++
Point::PointFactory::NewXxx(...)
```

- 如果你不希望Point这个词在上面代码中出现两次，你可以使用 `typedef PointFactory Factory`，这样就能写成`Point::Factory::NewXxx(...)`的形式。这可能是人们能想到的最合理的语法了。或者干脆把内部工厂PointFactory改名为Factory，这样就一劳永逸地解决了问题，除非你决定以后把它提出来。



是否使用内部工厂很大程度上取决于个人组织代码风格。 然而，从原始对象公开工厂极大地提高了API的可用性。[:FIXME]如果我发现一个名为Point的类和一个私有构造函数，我如何能够知道这个类是要被使用的?除非Person::在代码完成列表中给我一些有意义的东西，否则我不会这么做。


#### 抽象工厂

目前为止，我们一直在研究单个对象的构造。有时，你可能会涉及到一系列对象的构造。在实际中这种情况是非常少见的，同工厂方法和简单陈旧的工厂模式相比，抽象工厂模式只会出现在复杂的系统中。不管怎样，主要是由于历史的原因，我们都需要讨论它。

下面是一个简单的场景：假设你在一家只提供咖啡和茶的咖啡店里面工作。这两种热饮是通过完全不同的设备制成，我们可以把它们都建模成一个工厂。茶和饮料事实上可以分为热(hot)饮和冷(cold)饮，但是这里我们只关注热饮。首先我们把HotDrink定义成：

```c++
struct HotDrink
{
    virtual void prepare(int volume) = 0;
}
```

prepare函数的功能就是提供一定量(volume)的热饮。例如，对于茶这种类型的热饮，可以实现成这样：

```c++
struct Tea: HotDrink
{
    void prepare(int volume) override
    {
        cout << "Take tea bag, boil water, pour " 
             << volume 
             << "ml, add some lemon" << endl;
    }
};
```

对于咖啡类型的是实现也是相似的。现在，我们可以编写一个假想的make_drink函数，该函数通过饮料的名字来制作对应的饮料。给出一系列不同的类型，这看起来相当乏味：

```c++
unique_ptr<HotDrink> make_drink(string type)
{
    unique_ptr<HotDrink> drink;
    if(type == "tea")
    {
        drink = make_unique<Tea>();
        drink->prepare(200);
    }
    else
    {
        drink = make_unique<Coffee>();
        drink->prepare(50);
    }
    return drink;
}
```

现在请记住，不同的饮料是由不同的机器制造的。在我们的例子中，我们只对热饮感兴趣，我们可以构造一个HotDrinkfactory类来建模:

```c++
struct HotDrinkFactory
{
    virtual unique_ptr<HotDrink> make() const = 0;
};
```

这种类型恰好是一个抽象工厂（Abstract Factory）:它是有特定接口的一个工厂，但是它是抽象的，[FIXME]这意味着它只能作为函数参数， 例如，我们需要具体的实现来实际制作饮料。例如想要制作咖啡，我们可以写成：

```c++
struct CoffeeFactory:HotDrinkFactory
{
    unique_ptr<HotDrink> make() const override
    {
        return make_unique<Coffee>();
    }
}
```

TeaFactory的定义也与上面的CoffeeFactory相似。现在，假设我们想要定义更高层次的接口，用来制作不同类型的饮料，可以是热饮也可以是冷饮。我们可以创建一个名为DrinkFactory的工厂，它本身包含各种可用工厂的引用：

```c++
class DrinkFactory
{
    map<string, unique_ptr<HotDrinkFactory>> hot_factories;
    DrinkFactory()
    {
        hot_factories["coffee"] = make_unique<CoffeeFactory>();
        hot_factories["tea"] = make_unique<TeaFactory>();
    }
    unique_ptr<HotDrink> make_drink(const string &name)
    {
        auto drink = hot_factories[name]->make();
        drink->prepare(200); // oops!
        return drink;
    }
};
```

这里，我假设我们希望根据饮料的名称，不是某个整数或enum成员来制作饮料。我们只需创建字符串和相关工厂的映射(map):实际的工厂类型是HotDrinkFactory(我们的抽象工厂)，并通过智能指针而不是直接存储它们(这很有意义，因为我们希望防止对象切片)

现在,如果你想喝一杯饮料时，你可以找到相关的工厂(想象一下咖啡店的店员走到正确的机器前)，生产饮料，准备好需要的量(我在前面设置了一个常数;你可以随意将其提升为一个参数)，就拿到了想要的饮料。嗯，就是这样。


#### 函数工厂

我想提的最后一件事情是：当我们使用工厂(factory)这个词的时候，我们通常指下面两种情况中的一种：

- 一个知道如何创建对象的类
- 一个被调用时会创建对象的函数

第二种情况不仅仅是传统意义上的工厂方法。如果传递返回类型为T的std::function到某个函数中，这个被传递的函数通常被称为工厂，而不是工厂方法。这可能看起来有点奇怪，但如果考虑到方法与成员函数的含义是相同的，这样说会显得有意义点。

幸运的是，函数可以存储在变量中，这意味着不只是存储指向工厂的指针(就像我们之前在DrinkFactory中做的那样)，我们可以把准备200毫升液体的过程内化到函数中。这是通过从工厂切换到简单的使用函数块来实现的，例如:

```c++
class DrinkWithVolumeFactory
{
    map<string, function<unique_ptr<HotDrink>()>> factories;
    public:
        DrinkWithVolumeFactory()
        {
            factories["tea"] = []{
                auto tea = make_unique<Tea>();
                tea->prepare(200);
                return tea;
            };
             // 对应Coffee类也是类似的。
        }
}
```

当然，在采用了这种方法之后，我们现在只需要直接调用存储的工厂, 而不必在对象被构造出来之后再调用prepare方法。

```c++
inline unique_ptr<HotDrink>
DrinkWithVolumeFactory::make_drink(const string &name)
{
    return factories[name];
}
```

在使用方法上和以前是一样的。

#### 总结

让我们来回顾下这章涉及到的术语：

- 工厂方法（factory method）是类的成员函数，可以作为创建对象的一种方式，通常用来替代构造函数。
- 工厂（factory）通常是知道如何创建对象的独立的类，尽管如果你传递构造对象的函数(std::function，函数指针或者函数对象)到某个函数里面，这个参数通常也被称为工厂。
- 抽象工厂（abstract factory），顾名思义，是一个抽象类，可以被生产一系列对象的具体类所继承。抽象工厂在实际中很少见。


工厂相对于构造函数调用有下面几个关键的优势：

- 工厂可以说“不”，这意味着除了选择返回一个对象外，它可以返回一个空指针(nullptr)。
- 命名更有直观意义，且不受限，不像构造函数的函数名必须和类名相同。
- 一个工厂能够生产出许多不同类型的对象。
- 工厂能够表现出多态行为，实例化一个类并通过基类的引用或指针返回实例化后的对象。
- 工厂能够实现缓存(caching)和其他存储优化，他也是其他方法，例如池或单例模式（更多参见第5章内容）实现的自然的选择。

工厂与建造者模式的不同之处在于，对于工厂，您通常一次性创建一个对象，而对于建造者，您通过部分地提供信息来分段地构造对象。