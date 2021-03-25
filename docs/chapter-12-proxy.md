### 代理

#### 智能指针

代理模式最简单、最直接的例子是智能指针。智能指针对普通指针进行了包装，同时保留引用计数，重写了某些操作符，但总的来说，它提供和普通指针一样的使用接口。

```c++
struct BankAccount
{
    void deposit(int amount) { ... }
};

BankAccount *ba = new BankAccount;
ba->deposit(123);
auto ba2 = make_shared<BankAccount>();
ba2->deposit(123); // 相同的 API!
```

因此，智能指针在某些地方可以用来替代普通指针。例如，`if (ba){...}`无论`ba`是普通指针还是智能指针，`*ba`在这两种情况下可以获得底层对象。

当然，差异是存在的。最明显的一个是你不需要在智能指针上调用delete。但除此之外，它真的试图尽可能地接近一个普通的指针。



#### 属性代理

在其他编程语言中，属性用于指示字段和该字段的一组`getter/setter`方法。在c++[^1]中没有属性，但是如果我们想继续使用一个字段，同时给它特定的访问/修改(`accessor/mutator`)行为，我们可以构建一个*属性代理*

本质上，属性代理是一个可以伪装成属性的类，所以我们可以这样定义它:

```c++
template<typename T>
struct Property 
{
    T value;
    Property(const T initial_value)
    {
        *this = initial_value;
    }
    operator T()
    {
        return value;
        // 执行一些getter操作
    }
    T operator=(T new_value) 
    {
        return value = new_value;
        // 执行一些setter操作
    } 
};
```

在前面的实现中，我已经向通常要自定义(或直接替换)的位置添加了注释，如果要采用这种方法，注释大致对应于`getter /setter`的位置。

因此，我们的类`Property<T>`本质上是`T`的替换，不管它是什么。它的工作原理是简单地允许与T的转换，并允许两者都使用value字段。现在，你可以把它用作一个字段。

```c++
struct Creature
{
    Property<int> strength{ 10 };
    Property<int> agility{ 5 };
};
```

一个字段上的典型操作也将在属性代理类型的字段上工作

```c++
Creature creature;
creature.agility = 20;
auto x = creature.strength
```

#### 虚代理

如果试图对nullptr或未初始化的指针进行解引用，就会自找麻烦。然而，在某些情况下，您只希望在访问对象时构造该对象，而不希望过早地分配它。

这种方法称为延迟实例化。如果你确切地知道哪些地方需要懒惰行为，你就可以提前计划并为之做特别的准备。如果您不这样做，那么您可以构建一个接受现有对象并使其懒惰的代理。我们称之为虚拟代理，因为底层对象可能根本不存在，所以我们访问的不是具体的东西，而是虚拟的东西。

想象一个典型的`Image`接口：

```c++
struct Image
{
    virtual void draw() = 0;
};
```

`Bitmap`的即时(与惰性相反)实现将在构建时从文件中加载图像，即使该图像实际上并不需要。(是的，下面的代码是模拟的。)

```c++
struct Bitmap : Image
{

    Bitmap(const string &filename)
    {
        cout << "Loading image from " << filename << endl;
    }
    void draw() override
    {
        cout << "Drawing image " << filename << endl;
    }
}
```

构造这个位图的行为将触发图像的加载:

```c++
Bitmap img{ "pokemon.png" };  // 从pokemon.png加载图像
```

那不是我们想要的。我们想要的是那种只在使用draw()方法时加载自身的位图。现在，我想我们可以跳回到位图，让它变得懒惰，但假设它是固定的，不能修改(或继承，就此而言)。

因此，我们可以构建一个虚拟代理，它将聚合原来的位图，提供一个相同的接口，并重用原来的位图功能：

```c++
struct LazyBitmap : Image
{
    LazyBitmap(const string filename) : filename(filename) {}
    ~LazyBitmap() {delete bmp; } // 如果*bmp没有先被new出来，这里就会出错

    void draw() override
    {
        if(!bmp) 
            bmp = new Bitmap(filename);
        bmp->draw();
    }

    private:
        Bitmap *bmp{nullptr};
        string filename;
}
```

我们到了。正如您所看到的，这个`LazyBitmap`的构造函数要轻得多:它所做的只是存储用于加载图像的文件名，这样图像实际上就不会被加载。

所有神奇的事情都发生在`draw()`中:这是我们检查`bmp`指针的地方，以查看底层(`eager`!)位图是否已经构造好了。如果没有，我们就构造它，然后调用它的`draw()`函数来实际绘制图像。

现在假设您有一些使用`Image`类型的API:

```c++
void draw_image(Image& img)
{
    cout << "About to draw the image" << endl;
    img.draw();
    cout << "Done drawing the image" << endl;
}
```

我们可以将该API与一个`LazyBitmap`实例一起使用，而不是使用`Bitmap`(万岁，多态!)来渲染图像，以一种惰性的方式加载它:

```c++
LazyBitmap img{ "pokemon.png" };
draw_image(img); // image loaded here
// About to draw the image
// Loading image from pokemon.png
// Drawing image pokemon.png
// Done drawing the image
```

#### 通讯代理