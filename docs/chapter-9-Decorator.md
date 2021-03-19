### 第九章 装饰器

假设你正在使用同事编写的类，并且希望扩展该类的功能。如果不修改原始代码，你会怎么做呢？好的，一种方法是继承:你创建一个派生类，添加你需要的功能，甚至可能重写(override)一些东西，然后就可以了。

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

TransparentShape myCircle
{
 ColoredShape{ Circle{23}, "green"}, 64 };
 cout << myCircle.str();
 // A circle of radius 23 has the color green has 25.098% transparency

```

### 静态装饰器
