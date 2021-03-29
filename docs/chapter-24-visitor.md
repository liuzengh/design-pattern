### 第24章：访问者模式

如果你要处理层次结构的类型，除非你能够访问源代码，否则不可能向层次结构的每个成员添加函数。这是一个需要提前规划的问题，并产生了访问者模式。

下面是一个简单的例子:假设你解析了一个由双精度值和加法操作符组成的数学表达式(当然是使用解释器模式)

```c++
( 1.0 + (2.0 + 3.0) )
```

这个表达式可以用如下的层次结构来表示：


```c++
struct Expression
{
    // 目前这里什么也没有
};

struct DoubleExpression : Expression
{
    double value;
    explicit DoubleExpression(const double value) : value{value} {}
};

struct AdditionExpression : Expression
{
    Expression* left;
    Expression* right;
    AdditionExpression(Exprssion* const left, Exprssion* const right) 
        : left{left}, 
        right{right}
     {};

    ~AdditionExpression()
    {
        delete left;
        delete right;
    }
};
```

因此，给定这个对象的层次结构，假设你想给`Expression`的各种继承类添加一些行为(好吧，目前我们只有两个，但这个数字可能会增加)。你会怎么做?

### 入侵式的访问者

我们将从最直接的方法开始，这将会打破开闭原则(Open-Closed Principle, OCP)实际上，我们将跳转到已经编写好的代码中，并修改`Expression`的接口(以及通过关联，对派生类)：

```c++
struct Expression
{
    virtual void print(ostringstrem& oss) = 0;
};
```

这种方法除了破坏`OCP`之外，还依赖于一个假设，即你实际上可以访问层次结构的源代码，而这并不总是得到保证。


但是我们总得从某处开始吧?因此，有了这个改变，我们需要在`DoubleExpression`(这很简单，所以我在这里省略它)和`AdditionExpression`中实现`print()`:

```c++
struct AdditionExpression : Expression
{
    Expression* left;
    Expression* right;
    ...
    void print(ostringstream& oss) override
    {
        oss << "(";
        left->print(oss);
        oss << "+";
        right->print(oss);
        oss << ")";
    }
};
```

哦，这真有趣!我们在子表达式上多态地和递归地调用`print()`。妙啊！让我们来测试一下：

```c++
auto e = new AdditionExpression
{
    new DoubleExpresion{1},
    new AdditionExpression
    {
        new DoubleExpression{2},
        new DoubleExpression{3}
    }
};
ostringstream oss;
e->print(oss);
cout << oss.str() << endl; // print (1 + (2 + 3) )
```

这很简单。但是现在，假设你在层次结构中有10个继承者(顺便说一下，这在现实场景中并不少见)，你需要添加一些新的`eval()`操作。这十个修改需要在十个不同的类里完成。但`OCP`并不是真正的问题。

真正的问题是接口隔离(`Interface Segregation Princile, ISP`)。你看，像打印这样的问题是一个特别关注的问题。与其声明每个表达式都应该打印自己，为什么不引入一个知道如何打印表达式的表达式打印机`ExpressionPrinter`呢?稍后，你可以引入一个表达式求值器`ExpressionEvaluator`，它知道如何执行实际的计算，所有这些都不会以任何方式影响表达式层次结构。

### 反射式的Printer

既然我们已经决定创建一个单独的打印机组件，那么让我们去掉`print()`成员函数(但当然保留基类)。这里有一个警告:不能让表达式类为空。为什么?因为只有当你有一些虚拟的东西在里面的时候你才会得到多态行为。所以，现在，我们在这里插入一个虚拟析构函数:

```c++
struct Expression
{
    virtual ~Expression() = default;
};
```

现在让我们尝试实现一个`ExpressionPrinter`。我的第一反应是写这样的东西:

```c++
struct ExpressionPrinter
{
    void print(DoubleExpression* de, ostringstream& oss) const
    {
        oss << de->value;
    }
    void print(AdditionExpression* ae, ostringstream& oss) const
    {
        oss << "(";
        print(ae->left, oss);
        oss << "+";
        print(ae->right, oss);
        oss << ")";
    }
};
```

前面的代码几乎不能通过编译。C++知道`ae->left`是`Expression`类型，但是它在运行时不能检查类型(不像各种动态类型的语言)， 它不知道应该调用哪个重载函数。这也太糟糕了！

我们能做些什么呢？我们只能做一件事-移除重载并且在运行时进行类型检查。

```c++
struct ExpressionPrinter
{
    void print(Expressoin* e)
    {
        if(auto de dynamic_cast<DoubleExprssion*>(e))
        {
            oss << de->value;
        }
        else if(auto ae = dynamic_cast<AdditionExpression*>(e))
        {
            oss << "(";
            print(ae->left, oss);
            oss << "+";
            print(ae->right, oss);
            oss << ")";
        }
        string str() const { return oss.str(); }
        private:
            ostringstream oss;
    }
};
```

前面的方法实际上是一个可行的解决方案:

```c++
auto e = new AdditionExpression
{
    new DoubleExpresion{1},
    new AdditionExpression
    {
        new DoubleExpression{2},
        new DoubleExpression{3}
    }
};
ExpressionPrinter ep;
ep.print(e);
cout << ep.str() << endl; // print (1 + (2 + 3) )
```

这种方法有一个相当显著的缺点:实际上，您没有为层次结构中的每个元素实现打印的编译器检查。

添加新元素时，可以继续使用`ExpressionPrinter`而不需要修改，它会跳过新类型的任何元素。

但这是一个可行的解决方案。认真地说，很有可能在这里就停止了，而不再进一步使用访问者模式:`dynamic_cast`开销并不那么昂贵，而且我认为许多开发人员会记得在`if`语句中涵盖每一种类型的对象。

#### 什么是分派？

每当人们谈论访问者模式时，就会提到分派(`dispatch`)这个词。它是什么?简单地说，分派就是确定具体要调用哪个函数，为了进行分派需要多少条信息。

下面是一个简单的例子：

```c++
struct Stuff {};
struct Foo : Stuff {};
struct Bar : Stuff {} ;

void func(Foo* foo) {}
void func(Bar* bar) {}
```

现在，如果我创建一个普通的Foo对象，那么调用它的func()就没有问题了

```c++
Foo* foo = new Foo;
func(foo); // ok
```

但如果我决定将它转换为基类指针，那么编译器将不知道要调用哪个重载函数：

```c++
Stuff* stuff = new Foo;
func(stuff);
```

现在，让我们从多态的角度来考虑这个问题:有没有办法强制系统调用正确的重载函数，而不需要任何运行时检查(比如`dynamic_cast`和类似的方法)。的确存在这种方法。

看，当你在一个`Stuff`上调用某个方法时，这个调用可以是多态的(多亏了虚函数表)，它可以被分派到必要的组件。这样就可以调用必要的重载函数。这被称为双分派(`double dispatch`)，因为：

1. 首先，你对实际对象做一个多态调用
2. 在多态调用中，调用重载。因为在对象内部，`this`有一个明确的类型(例如，`Foo*`或`Bar*`)，所以会触发正确的重载。


```c++
struct Stuff 
{
    virtual void call() = 0;
};
struct Foo : Stuff
{
    void call() override 
    {
        func(this);
    }
};
struct Bar : Stuff
{
    void call() override
    {
        func(this);
    }
}
void func(Foo* foo) { }
void func(Bar* bar) { }
```

你能看到这里发生了什么吗?我们不能只是将一个通用的`call()`实现插入到某个东西中:不同的实现必须在各自的类中，这样`this`指针才具有合适的类型。

这个实现允许你写以下代码:

```c++
Stuff* stuff = new Foo;
stuff->call(); // effectively calls func(stuff)
```

#### 经典的访问者

访问者设计模式的经典实现使用了双重分派。访问者成员函数的调用是有约定的:

- 访问者的成员函数通常叫做`visit()`
- 在整个层次结构中实现的成员函数通常称为`accept()`

我们现在可以从我们的`Expression`基类中扔掉虚析构函数，因为我们实际上有一些东西要放进去: `accept()`函数：

```c++
struct Expression
{
    virtual void accept(ExpressionVisitor* visitor) = 0;
};
```

如你所见，上述代码引用了一个名为`ExpressionVisitor`的(抽象)类，它可以作为各种访问者(如`ExpressionPrinter`、`ExpressionEvaluator`等)的基类。我在这里选择了一个指针，但是你也可以选择使用一个引用。

现在，现在从`Expression`继承的每一个派生类都需要以相同的方式实现accept()，即:

```c++
void accept(ExpressionVisitor* visitor) override
{
    visitor->visit(this);
}
```

另一方面，我们可以像下面这样定义`ExpressionVisitor`:

```c++
struct ExpressionVisitor
{
    virtual void visit(DoubleExpression* de) = 0;
    virtual void visit(AdditionExpression* ae) = 0;
};
```

注意，我们必须为所有对象定义重载;否则，在实现相应的`accept()`时将会出现编译错误。现在我们可以继承这个类来定义我们的`ExpressionPrinter`:

```c++
struct ExpressionPrinter : ExpressionVisitor
{
    ostringstream oss;
    string str() const { return oss.str(); }
    void visit(DoubleExpression* de) override;
    void visit(AdditionExpression* ae) override;
};
```

`visit()`函数的实现应该是相当明显的，因为我们已经不止一次地看到了它，但我将再次展示它:

```c++
void visit(AdditionExpression* ae) 
{
    oss << "(";
    ae->left->accept(this);
    oss << "+";
    ae->right->accept(this);
    oss << ")";
}
```


请注意，调用现在是如何在子表达式本身上发生的，再次利用了双重分派。至于使用新的双分派访问者，它是这样的:

```c++
void main()
{
    auto e = new AdditionExpression
    {
        // as before
    };
    ostringstream oss;
    ExpressionPrinter ep;
    ep.visit(e);
    cout << ep.str() << endl; //(1+(2+3))
}
```

#### 实现一个额外的访问者


那么，这种方法的优点是什么呢?这样做的好处是你只需要通过层次结构实现一次`accept()`成员。你再也不用碰层级中的成员了。例如:假设你现在想要一种方法来计算表达式的结果?这是很容易的：

```c++
struct ExpressionEvaluator : ExpressionVisitor
{
    double result;
    void visit(DoubleExpression* de) override;
    void visit(AdditionExpression* ae) override;
};
```

但需要记住的是，visit()目前声明为void方法，因此实现可能看起来有点奇怪:

```c++
void ExpressionEvaluator::visitor(DoubleExpression* de)
{
    result = de->value;
}

void ExpressionEvaluator::visitor(AdditionExpression* ae)
{
    ae->left->accept(this);
    auto temp = result;
    ae->right->accept(this);
    result += temp;
}
```

前面的情况是无法从`accept()`返回结果的，有点棘手。本质上，我们计算左边的部分并缓存值。然后对右边的部分求值(因此设置了`result`)，然后将其增加缓存的值，从而生成求和。不是很直观的代码。

尽管如此，它仍然工作良好:

```c++
auto e = new AdditionExpression{ /* as before */ };
ExpressionPrinter printer;
ExpressionEvaluator evaluator;
printer.visit(e);
evaluator.visit(e);
cout << printer.str() << " = " << evaluator.result << endl;
// prints "(1+(2+3)) = 6
```

同样，你也可以添加许多其他不同的访问者，遵循OCP，并在这个过程中玩得快乐。


### 无环访问者

现在是时候提一下，实际上，访问者设计模式有两种类型。他们是

- 循环访问者，这是基于函数重载。由于层次结构(必须知道访问器类型)和访问器(必须知道层次结构中的每个类)之间的循环依赖关系，该方法的使用仅限于不经常更新的稳定层次结构。

-无环访问者，基于`RTTI`。这样做的优点是对已访问的层次结构没有限制，但是，正如你可能已经猜到的那样，这对性能有影响。

实现无循环访问器的第一步是实际的访问器接口。我们没有为层次结构中的每一个类型定义一个`visit()`重载，而是尽可能地使其泛型化:

```c++
template <typename Visitable>
struct Visitor
{
    virtual void visit(Visitable& obj) = 0;
}
```

我们需要域模型中的每个元素都能够接受这样的访问者，但是，由于每个专门化都是惟一的，所以我们要做的是引入一个标记接口(`marker interface`)，一个空类，只有一个虚析构函数:

```c++
struct VisitorBase // 标记接口
{
    virtual ~VisitorBase() = default;
};
```

前面的类没有行为，但我们将在实际访问的任何对象中将其用作`accept()`方法的参数。现在，我们可以像下面这样重新定义表达式类:

```c++
struct Expression
{
    virtual ~Expression() = default;
    virtual void accept(VisitorBase& obj)
    {
        using EV = Visitor<Expression>;
        if(auto ev = dynamic_cast<EV*>(&obj))
            ev->visit(*this);
    }
}
```

所以新的`accept()`方法是这样工作的:我们取一个`VisitorBase`，然后试着把它转换为一个`Visitor<T>`，其中`T`是我们当前使用的类型。如果转换成功，访问者就知道如何访问我们的类型，为此它相应的`visit()`方法。如果失败了，那就没办法了。理解为什么`obj`本身没有`visit()`方法是非常关键的。如果它这样做了，它将需要为每一个有想要调用它的类型进行重载，这将会引入循环依赖。

在模型的其他部分实现`accept()`之后，我们可以通过再次定义`ExpressionPrinter`将所有东西放在一起，但这一次，它看起来如下所示:

```c++
struct ExpressionPrinter : VisitorBase, 
                           Visitor<DoubelExpression>
                           Visitor<AdditionExpression>
{
    void visit(DoubleExpression &obj) override;
    void visit(AdditionExpression &obj) override;
    string str() const { return oss.str(); }
    private:
        ostringstream oss;
};
```

如你所见，我们为每个想要访问的地方实现了`VisitorBase`标记接口以及`Visitor<T>` 。如果我们省略了一个特定类型的T(例如，假设我注释掉了`Visitor<DoubleExpression>`;)，程序仍然会编译，而相应的`accept()`调用，如果它来了，将作为一个无操作执行。在前面，`visit()`方法的实现与我们在经典访问者实现中的实现实际上是相同的，结果也是一样的。

#### `Variants` 和 `std:visit`

虽然与经典访问者模式没有直接关系，但是`std::visit`还是值得一提的，因为它的名字暗示它与访问者模式有关。本质上，`std::visit`是访问`variant`类型的`correct part`的一种方法。

下面是一个例子:假设你有一个地址，该地址的部分字段是一个房屋字段。现在，一座房子可以只是一个号码(如伦敦路123号)，也可以有一个名字，如Montefiore Castle。因此，你可以如下定义该`variant`

```c++
variant<string, int> house;
// house = "Montefiore Castle"
house = 221;
```

这两个赋值都是有效的。现在，假设你决定打印房子的名字或号码。为此，你可以首先定义一个类型，在该类型里实现对`variant`的函数调用符重载。

```c++
struct AddressPrinter
{
    void operator()(const string& house_name) const
    {
        cout << "A house called" << house_name << "\n";
    }
    void operator()(const int house_number) const
    {
        cout << "House number" << house_number << "\n";
    }
} ;
```

现在，该类型可以与`std::visit()`一起使用，这是一个库函数，它将这个访问者应用到`variant`类型：

```c++
AddressPrinter ap;
std::visit(ap, house); // House number 221
```
可以在适当的地方定义一组访问者函数，这要感谢一些现代c++的魔力。我们需要做的是构造一个类型为`auto&`的`lambda`表达式，获取底层类型，使用`if constexpr`比较它，并相应地处理:

```c++
std::visit([](auto& arg){
    using T = decay_t<decltype(arg)>;
    if consyexpr(is_same_v<T, string>)
    {
        cout << "A house called " << arg.c_str() << "\n";
    }
    else
    {
        cout << "House number " << arg << "\n";
    }
}, house);
```

#### 总结

访问者设计模式允许我们向对象层次结构中的每个元素添加一些行为。我们已经看到的方法包括:

- 入侵式：给层次结构里面的每个对象增加一个虚函数。可行（但是你必须获取到源代码）但打破了开闭原则。
- 反射式：增加一个单独的访问者，这样就不需要改变被访问的对象；当需要动分派时使用`dynamic_cast`
- 经典（双分派）：被访问者整个的层次结构只会以一种通用的方式被修改一次。层次结构里面的每个元素实现`accpet()`方法来接收访问者。然后我们将`visitor`子类化，以在各个方向增强层次结构的功能。

访问者模式经常与解释器模式一起出现:在解析了一些文本输入并将其转换为面向对象的结构之后，我们需要，例如，以特定的方式呈现抽象语法树。`Visitor`帮助在整个层次结构中传播`ostringstream`(或类似的对象)，并将数据整理在一起,


