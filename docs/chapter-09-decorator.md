### 第九章 装饰器

假设你正在使用同事编写的类，并且希望扩展该类的功能。如果不修改原始代码，你会怎么做呢？一种方法使用是继承:你可以创建一个派生类，添加你需要的功能，甚至可能重写(override)一些东西，然后就可以了。

但这并不总是有效，原因有很多。例如，您通常不希望从std::vector继承，因为它没有虚析构函数，或者从int继承(这是不可能的)。继承不起作用的最关键原因是，当你需要添加多个功能的时候，你希望遵循单一职责原则，将这些功能分开。

装饰器模式允许我们在不修改原始类型(开闭原则)或导致派生类型数量激增的情况下增加现有类的功能。

#### 场景

假设我们定义一个名为Shape的抽象类：

```c++
struct Shape
{
    virtual string str() const = 0;
};
```

在Shape类中，`str()`是一个虚函数，我们将使用它来提供表示特定形状的字符串。现在我们可以用该接口实现Circle类或Square类：

```c++
struct Circle : Shape
{
  float radius;
  explict: Circle(const float radius) : radius{radius} {};
  void resize(float factor) { radius *= factor; } 
  string str() const override
  {
    ostringstream oss;
    oss <<  "A circle of radius " << radius;
    return oss.str();
  }
}; 
// 下面省略了Square 类的实现
```

#### 动态装饰器

假设我们想要给形状增加一些颜色。我们可以使用组合的方式替代继承实现ColoredShape类，简单地引用一个已经构造好的Shape对象并增强它:

```c++
struct ColoredShape : Shape
{
    Shape &shape;
    string color;
    ColoredShape(Shape &shape, const string &color) : shape(shape), color(color) {};
    string str() const override 
    {
        ostringstream oss;
        oss << shape.str() << " has the color " << color;
        return oss.str()
    }
};
```

正如你所看到的，ColoredShape它本身是`Shape`的一种。我们可以像这样使用它：

```c++
Circle circle{0.5f};
ColoredShape redCircle{circle, "red"};
cout << redCircle.str();
// prints "A circle of radius 0.5 has the color red"
```

如果我们现在想要增加形状的透明度，这也很简单：

```c++
 struct TransparentShape : Shape
 {
    Shape& shape;
    uint8_t transparency;

    TransparentShape(Shape& shape, const uint8_t transparency)
    : shape{shape}, transparency{transparency} {}

    string str() const override
    {
        ostringstream oss;
        oss << shape.str() << " has "
        << static_cast<float>(transparency) / 255.f*100.f
        << "% transparency";
        return oss.str();
    }
 };
```

但最重要的是我们可以把ColoredShape和TransparentShape组合起来，使得一个形状既有颜色又有透明度:

```c++

TransparentShape myCircle {ColoredShape{ Circle{23}, "green"}, 64 };
 cout << myCircle.str();
 // A circle of radius 23 has the color green has 25.098% transparency

```

#### 静态装饰器

你是否注意到，在之前的讨论的场景中，我们给`Circle`提供了一个名为`resize()`的函数，不过它并不在`Shape`接口中。你可能已经猜到的，因为它不是`Shape`成员函数，所以不能从装饰器中调用它。

```c++
Circle circle{3};
ColoredShape redCircle{circle, "red"};
redCircle.resize(2); // 编译不能通过
```

假设你并不真正关心是否可以在运行时组合对象，你真正关心的是：能否访问修饰对象的所有字段和成员函数。有可能建造这样一个装饰器吗?

的确有办法实现，而且它是通过模板和继承完成的——但不是那种会导致状态空间爆炸的继承。相反，我们使用一种叫做`Mixin`继承的方法，类从它自己的模板参数继承。

为此，我们创建一个新的`ColoredShape`，它继承自一个模板参数。我们没有办法将模板形参限制为任何特定类型，因此我们将使`static_assert`用进行类型检查。

```c++
template<typename T>
struct ColoredShape :  T
{
  static_assert(is_base_of<Shape, T>::value, "Template argument must be a Shape");
  string color;
  string str() const override
  {
    ostringstream oss;
    oss << T::str() << "has the color" << color;
    return oss.str();
  }
};
```

有了`ColorredShape<T>`和`TransparentShape<T`的实现，我们现在可以把它们组合成一个有颜色的透明形状。

```c++
ColoredShape<TransparentShape<Shape>> square{"bule"};
square.size = 2;
square.transparency = 0.5;
cout << square.str();
square.size();
```

这的确很棒，但并不完美:我们似乎失去了对构造函数的充分使用：即使我们能够初始化最外层的类，我们也不能在一行代码中完全构造具有特定大小、颜色和透明度的形状。

为了锦上添（即装饰！）花，我们给出`Colordshape`和`TransparentShape`转发构造函数。这些构造函数将接受两个参数:第一个参数作用于当前模板类，第二个是传递给基类的泛型形参包。

```c++
template<typename T> 
struct TransparentShape : T
{
  uint8_t transparency;
  template<typename ...Args>
  TransparentShape(const uint8_t transparency, Args ...args):
    T(std::forward<Args>(args)...),
    transparency{transparency} {}
};
// ColoredShape也类似
```

只是重申一下，前面的构造函数可以接受任意数量的参数，其中第一个参数用于初始化透明值，其余的只是转发给基类的构造函数。

构造函数的数目必须保证是正确的，如果构造函数的数目或值的类型不正确，程序将无法编译。如果开始向类型中添加默认构造函数，那么整体参数集的使用就会变得灵活得多，但也会引入歧义和混淆。


哦，还要确保永远不要显式地使用这些构造函数，否则在组合这些装饰器时，就会违反c++的复制列表初始化规则。现在，如何真正利用这些好处？

```c++
ColoredShape2<TransparentShape2<Square>> sq = { "red", 51, 5 };
cout << sq.str() << endl;
 // A square with side 5 has 20% transparency has the color red
```

漂亮!这正是我们想要的。这就完成了静态装饰器的实现。同样，你可以对它进行增强，以避免重复类型，如`ColorredShape<ColorredShape<...>>`，或循环 `ColorredShape<TransparentShape<ColorredShape<...>>>`;但在静态环境中，这感觉像是浪费时间。不过，多亏了各种形式的模板魔法，这是完全可行的。

#### 函数装饰器

虽然装饰器模式通常应用于类，但也同样可以应用于函数。假设你想在现有的代码中实现一个额外的功能: 你想记录一个函数被调用的情况，并在Excel中分析统计数据。当然，这可以通过在调用之前和之后添加一些代码来实现。

```c++
cout << "Entering function\n";
// do the work
cout << "Exiting funcion\n";
```

这工作得很好，但就关注点分离而言并不好:我们希望将日志记录功能存储在某个地方，以便我们可以重用它，并在必要时增强它。可以使用不同的方法来实现。一种方法是将整个工作单元作为`lambda`表达式提供给类似下面的日志组件：

```c++
struct Logger{
  function<void()> func;
  string name;
  Logger(const function<void>& func, const string& name):
    func{func},
    name{name}
    {
      
    }
    void operator()()const
    {
      cout << "Entering" << name  << endl;
      func();
      cout << "Exiting" << name  << endl;
    }
};
```

使用这种方法，你可以编写以下内容:

```c++
Logger([]() {cout << "Hello" << endl; }, "HelloFunction")();
\\ output:
\\ Entering HelloFunction
\\ Hello
\\ Exiting HelloFunction
```

我们也可以将函数作为模板参数而不是`std::function`传入，这只需要在前面的代码中稍微改动下即可：

```c++
template <typename Func>
struct Logger2{
  Func func;
  string name;
  Logger2(const Func& func, const string& name):
    func{func},
    name{name}
    {
      
    }
    void operator()() const
    {
      cout << "Entering" << name  << endl;
      func();
      cout << "Exiting" << name  << endl;
    }
};
```
与之前用法完全相同， 我们可以创建一个实用函数来日志对象：

```c++
template <typename Func> auto make_logger2(Func func, const string& name)
{
  return Logger2<Func>{ func, name }; // () = call now
}
```

然后像这样使用它:

```c++
auto call = make_logger2([]() {cout << "Hello!" << endl; }, "HelloFunction");
call();
```

你可能会问这样做有什么意义呢？意义在于，我们现在有能力创建一个装饰器(其中包含被装饰的函数)并在我们选择的时候调用它。

前面定义的`function<void()> func`没有函数参数和返回值，如果现在你想要实现带有返回值和函数参数的`add()`函数的调用(定义如下)，该怎么办:

```c++
double add(double a, double b)
{
  cout << a << "+" << b << "=" << (a + b) << endl;
  return a + b;
}
```

不是那么容易!但当然也不是不可能。让我们再实现一个`Logger`版本吧:

```c++
template <typename R, typename... Args>
struct Logger3{
  function<R(Args...)> func;
  string name;

  Logger3(const function<R(Args...)>& func, const string& name):
    func{func},
    name{name}
    {
      
    }
    R operator()(Args... args) const
    {
      cout << "Entering" << name  << endl;
      R result = func(args...);
      cout << "Exiting" << name  << endl;
      return R;
    }
};
```

在前面，模板参数`R`指的是返回值的类型，而`Args`，你肯定已经猜到了。装饰器保留该函数并在必要时调用它，唯一的区别是`operator()`返回一个`R`，因此不会丢失返回值。我们可以构造另一个实用函数`make_function`

```c++
template <typename R, typename... Args> 
auto make_logger3(R (*func)(Args...), const string& name)
{
  return Logger3<R(Args...)>{function<R(Args...)>(func),  name }; // () = call now
}
```

注意，我没有使用`std::function`，而是将第一个参数定义为普通函数指针。我们现在可以使用这个函数来实例化带有日志记录的函数调用并使用它

```c++
auto logged_add = make_logger3(add, "Add");
auto result = log_add(2, 3);
```

当然，可以用依赖注入（ Dependency Injection）代替`make_logger3`。这种方法的好处是：

- 通过提供空的对象(`Null Object`)来动态打开和关闭日志记录，而不是实际的日志对象
- 禁用被记录的代码的实际调用(同样，通过替换不同的日志对象)

总之，这是开发人员工具箱中的另一个有用的工具。[FIXME:]我把这种方法放入到依赖项注入中留给读者作为练习。

#### 总结

在遵循开闭原则（OCP）的同时，装饰器为类提供了额外的功能。它的特点是可组合性:几个装饰器可以以任何顺序应用到一个对象上。我们已经研究了以下类型的装饰器：

- **动态装饰器** 可以存储修饰对象的引用(甚至存储整个值，如果你想的话!)，并提供动态(运行时)可组合性，但代价是不能访问底层对象自己的成员。
- **静态装饰器** 使用`mixin`继承(从模板参数继承)在编译时组合装饰器。这失去了任何类型的运行时灵活性(您不能重新组合对象)，但允许你访问底层对象的成员。这些对象也可以通过构造函数转发完全初始化。
- **函数装饰器** 可以包装代码块或特定的函数，以允许 行为的组合


值得一提的是，在不允许多重继承的语言中，装饰器也用于模拟多重继承，方法是聚合多个对象，然后提供一个接口，该接口是聚合对象的接口的集合并。