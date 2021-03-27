### 桥接模式

如果你一直关注c++编译器(特别是GCC、Clang和MSVC)的最新进展，你可能已经注意到编译速度正在提高。特别是，编译器变得越来越增量化，因此编译器实际上只能重新构建已更改的定义，并重用其余的定义，而不是重新构建整个翻译单元。

我之所以提到c++编译，是因为过去开发人员一直在使用一个奇怪的技巧(又是这个短语!)来优化编译速度。当然，我说的是...

#### Pimpl编程技法

让我先解释一下在指向实现的指针(`Pimpl(Pointer to implement)`)编程技法,。假设你决定创建一个`Person`类来存储一个人的姓名并允许他们打印问候。与通常定义Person的成员不同，你继续这样定义类

```c++
struct Person
{
    std::string name;
    void greet();
    Person();
    ~Person();

    class PersonImpl;
    PersonImpl* impl // good place for gsl::owner<T>
}
```

这太奇怪了。对于一个简单的类来说似乎有很多工作要做。让我们看看，我们有`name`和`greet()`函数，但为什么要费心使用构造函数和析构函数呢?这个类`PersonImpl`是什么?


你现在看到的是`Person`类，选择将其实现隐藏在另一个类(`PersonImpl`)。需要注意的是，`PersonImpl`这个类不是在头文件中定义的，而是驻留在.cpp文件(`Person. cpp`, `Person`和`PersonImpl`耦合在一起)。它的定义很简单:

```c++
struct Person::PersonImpl
{
    void greet(Person* p);
};
```
原始的`Person`类向前声明`PersonImpl`，并继续保留指向它的指针。在`Person`的构造函数中初始化并在析构函数中销毁的正是这个指针; 如果智能指针能让你感觉更好，请随意使用。

```c++
Person::Person() :
 impl(new PersonImpl)
{ }

Person::~Person( ) 
{
    delete impl;
}
```

现在，我们要实现`Person::greet()`，正如你可能已经猜到的，它只是将控制权传递给`PersonImpl::greet()`

```c++
void Person::greet()
{
    impl->greet(this);
}

Person::PersonImpl::greet(Person* p)
{
    printf("hello %s", p->name.c_str());
}
```

这就是Pimpl编程技法，唯一的问题是为什么?!? 为什么要这么费劲地委托`greet()`并传递`this`指针呢?这种方法有三个优点:

- 更大比例的类的实现被隐藏起来。如果`Person`类的实现需要提供许多私有/受保护成员，那么你将向客户端公开所有这些细节，即使客户端由于私有/受保护访问修饰符永远无法访问这些成员。使用`Pimpl`编程技法，可以只提供公共接口。
- 修改隐藏`Impl`类的数据成员不会影响二进制兼容性。
- 头文件只需要包含声明所需的头文件，而不需要包含实现。例如，如果`Person`需要`vector<string>`类型的私有成员，您将被迫在头文件`Person.h`种`#include` `<vector>`和`<string>` (这是传递性的，所以任何使用Person.h的人也会包括他们)。利用Pimpl编程技法，可以在`.cpp`文件中`#include` `<vector>`和`<string>` 。

你将注意到，上述几点允许我们保留一个干净的、不变的头文件。这样做的一个减少编译时间，但对于我们来说, `Pimpl`很好得揭示了桥接模式: 在我们的例子中,`Pimpl`不透明的指针(不透明的相对透明的,也就是说,你不知道它背后是什么)作为一个桥梁, 将公共接口的成员与隐藏在`.cpp`文件中的底层实现连接了起来。

#### 桥接模式

Pimpl编程技法是桥梁设计模式的一个非常具体的说明，现在让我们来看看一些更普遍的东西。假设我们有两个对象类(在数学意义上):几何形状和可以在屏幕上绘制它们的渲染器。

就像我们对适配器模式的演示一样，我们假设渲染可以以矢量和栅格形式进行(尽管我们在这里不会编写任何实际的绘图代码)，并且，就形状而言，我们将限制为圆形。

首先，我们给出基类`Renderer`：

```c++
struct Renderer
{
    virtual void render_circle(float x, float y, float radius) = 0;
};
```

我们可以很容易地构造矢量和栅格实现;下面我将使用一些代码模拟实际的呈现，以便向控制台编写内容

```c++
struct VectorRenderer : Renderer
{
    void render_circle(float x, float y, float radius) override
    {
        cout << "Rasterizing circle of radius " << radius << endl;
    }
};

struct RasterRenderer : Renderer
{
    void render_circle(float x, float y, float radius) override
    {
        cout << "Drawing a vector circle of radius " << radius << endl;
    }
};
```

基类`Shape`持有渲染器的引用; 该形状将支持`draw()`成员函数的自渲染，也将支持`resize()`操作。

```c++
struct Shape
{
    protected:
        Renderer& renderer;
        Shape(Renderer& renderer) : renderer { renderer } { }
    public:
        virtual void draw() = 0;
        virtual void resize(float factor) = 0;
};
```

您会注意到Shape类引用了一个渲染器。这恰好是我们建造的桥梁。现在我们可以创建`Shape`类的实现，提供额外的信息，比如圆心的位置和半径。

```c++
struct Circle : Shape
{
    float x, y, radius;
    void draw() override
    {
        render.render_circle(x, y, radius);
    }
    void resize(float factor) override
    {
        radius *= factor;
    }
    Circle(Renderer& renderer, float x, float y, float radius):
        Shape{renderer}, 
        x{x}, 
        y{y}, 
        radius{radius} 
        {}
    };

}
```

好的，所以这个模式很快就写好了，当然，有趣的部分是在`draw()`中:在这里我们使用桥梁连接圆(它有关于它的位置和大小的信息)和渲染过程。这里的桥就是一个`Renderer`, 例如

```c++
RasterRenderer rr;
Circle raster_circle{ rr, 5, 5, 5 };
raster_circle.draw();
raster_circle.resize(2);
raster_circle.draw();
```

在前面的例子中，桥是`RasterRenderer`: 你创建它的对象`rr`，把`rr`的一个引用传递给`Circle`，然后调用`draw()`将把`RasterRenderer`作为桥，绘制圆圈。如果你需要微调圆，你可以调用`resize()`调整它的大小，渲染仍然会工作得很好，因为渲染器不知道或关心`Circle`。


#### 总结

桥是一个相当简单的概念，作为一个连接器或胶水，连接两个部分在一起。抽象(接口)的使用允许组件在不真正了解具体实现的情况下相互交互。

也就是说，桥接模式的参与者确实需要知道彼此的存在。具体来说，一个`Circle`需要一个对`Renderer`引用，相反，渲染器知道如何具体地绘制圆(`render_circle()`成员函数的名称)。这可以与中介模式形成对比，中介模式允许对象在不直接感知对方的情况下进行通信。

