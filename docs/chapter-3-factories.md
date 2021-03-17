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

注意构造函数的前两个参数的名称被为了a和b， 我们无法告诉用户a和b的值应该来自哪个坐标系。与使用x、y、rho和theta这种清晰的名称来传达构造意图相比，这显然是一种表现力的丧失。

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

就和[:FIXME]Builder模式一样，我们可以把Point类中所有的构建方法都从Point类中提取出来放置到单独一个类中，我么称之为该类为**工厂**。首先我么重新定义Point类：

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
