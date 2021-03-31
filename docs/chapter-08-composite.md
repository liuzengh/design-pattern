### 组合

#### 多个属性

组合设计模式通常适用于整个类，一个对象通常由多个对象构成。举个例子，方便理解。在一个游戏中，每个生物都有不同的强度值、敏捷值、智力值等，这就很容易定义：
```c++
class Creature{
    int strength, agility, intelligence;
public:
    int get_strength() const
    {
        return strength;
    }

    void set_strength(int strength){
        Creature::strength = strength;
    }

    int get_agility() const
    {
        return agility;
    }

    void set_agility(int agility){
        Creature::agility = agility;
    }

    int get_intelligence() const
    {
        return intelligence;
    }

    void set_intelligence(int intelligence){
        Creature::intelligence = intelligence;
    }
};
```
接下来我们想要对这些属性进行操作，例如求多个属性的最大值、平均值、总和,如下:


```c++
class Creature{
    //其他的数据成员
    int sum() const{
        return strength + agility + intelligence;
    }

    double average const{
        return sum() / 3.0;
    }
    int max() const{
        return ::max(::max(strength, agility),intelligence);
    }
}
```

然而这样并不理想，原因如下：

        1）在计算所有统计数据总和时候，我们容易犯错并且忘记其中一个
    
        2）3.0是代表属性的数目，在这里被设计成一个固定值
    
        3）计算最大值时，我们必须构建一对std::max()

想象一下如果再增加一个新的属性，这个时候我们需要对sum()，average()，max()重构，这是十分糟糕的。

如何避免？如下：

```c++
class Creature{
    enum Abilities {str, agl, intl, count};
    array<int,count> abilities;
}
```

上面的枚举定义了一个名为count的额外值，标记着有多少个属性。现在我们这样定义属性的get和set方法：
```c++
int get_strength() const { return abilities[str];}

void set_strength(int value){
    abilities[str]=value;
}
//对于其他属性同样
```
现在再让我们看看对sum()，average()，max()的计算，看看有什么改进:

```c++
int sum() const{
    return accumulate(abilities.begin(), abilities.end(),0);
}

double average() const{
    return sum() / (double)count;
}

int max() const{
    return *max_element(abilities.begin(), abilities.end());
}
```

是不是更棒了，不仅使代码更容易编写和维护，而且添加新属性时候，十分简单，总量根本不需要去改变，并不会影响sum()，average()，max()。


#### 组合图形对象

想想诸如PowerPoint等应用程序，在哪里您可以选择多个不同的对象并将其作为一个拖动。然而如果要选一个一个对象，您也可以抓取该对象。渲染也是相同的：您可以呈现单个图形对象，或者您可以将多个形状组合在一起，并将其绘制为一个组。这种方法的实现相当容易，因为它只是依赖于单个接口，如下所示：

```c++
struct GraphicObject{
    virtual void draw() = 0;
};
```

现在从名字来看，你可能认为它总是代表一个单独的项目。然而，想想看：几个矩形和圆形组合在一起代表一个组合图形对象。正如我可以定义的，比如说，一个圆：

```c++
struct Circle : GraphicObject
{
    void draw() override
    {
        std::cout << "Circle" << std::endl;

    }
};
```
同样，我们可以定义一个由几个其他图形对象组成的图形对象。是的，关系可以无限递归：

```c++
struct Group : GraphicObject
{
    std::string name;
    explicit Group(const std::string& name) : name(name){}

    void draw() override
    {
        std::cout << "Group" << name.c_str() << " contains:" << std::endl;
        for(auto&& o:obejct)
            o->draw();
    }

    std::vector<GraphicObject*> objects;
}
```
单个圆和任意组都可以绘制，只要他们实现了draw()函数。组中有一个指向其他图形对象的指针数组，通过其访问多个对象的draw()方法，来渲染自身。
以下是编程接口的使用方法：
```c++
Group root("root");
Circle c1, c2;
root.obejects.push_back(&c1);

Group subgroup("sub");
subgroup.objects.push_back(&c2);

root.obejcts.push_back(&subgroup);

root.draw();
```
前面的代码生成以下输出：
```c++
Group root contains:
Circle 
Group sub contains:
Circle
```
这是组合设计模式最简单的实现，尽管我们自己已经定义了一个定制接口。现在，如果我们尝试采用其他一些更标准化的迭代对象的方法，这个模式会是什么样子呢？

#### 神经网络
机器学习是热门的新事物。机器学习中的一部分是使用人工神经网络:试图模仿神经元在我们大脑中工作方式的软件结构。
神经网络的核心概念当然是神经元。神经元可以根据其输入产生(通常是数字)输出，我们可以将该值反馈给网络中的其他连接。我们将只关注连接，所以我们将这样对神经元建模:

```c++
1   struct Neuron
2   {
3     vector<Neuron*> in, out;
4     unsigned int id;
5
6     Neuron()
7     {
8       static int id = 1;
9       this->id = id++;
10     }
11   };
```
我在id字段输入了身份。现在，你可能想做的是把一个神经元连接到另一个神经元上，这可以用
```c++
1   template<> void connect_to<Neuron>(Neuron& other)
2   {
3     out.push_back(&other);
4     other.in.push_back(this);
5   }
```
这个函数造当前神经元和另一个神经元之间建立了联系。目前为止，一切顺利。现在，假设我们也想创建神经元层。一个层很简单，就是特定数量的神经元组合再一起。
```c++
1   struct NeuronLayer : vector<Neuron>
2   {
3     NeuronLayer(int count)
4     {
5       while (count --> 0)
6         emplace_back(Neuron{});
7     }
8   };
```
看起来不错。但是现在有一个小问题。问题是这样的：我们希望神经元能够连接到神经元层。总的来说，我们希望像这样能够奏效：
```c++
1   Neuron n1, n2;
2   NeuronLayer layer1, layer2;
3   n1.connect_to(n2);
4   n1.connect_to(layer1);
5   layer1.connect_to(n1);
6   layer1.connect_to(layer2);
```
如您所见，我们有四个不同的案例需要处理:
1、神经元连接到另一个神经元
2、神经元连接到神经元层
3、神经元层连接到神经元
4、神经元层连接到另一个神经元层

正如你所猜到的，我们不可能对connect_to()函数进行四次重载。如果有三个不同的类，你会考虑创建九个函数吗？我不这么认为。相反，我们要做的是在基类中插入槽。由于多重继承，我们完全可以做到这一点。那么，下面呢？

```c++
 1   template <typename Self>
 2   struct SomeNeurons
 3   {
 4     template <typename T> void connect_to(T&  other)
 5     {
 6       for (Neuron& from : *static_cast<Self*>(this))
 7       {
 8         for (Neuron& to : other)
 9         {
10           from.out.push_back(&to);
11           to.in.push_back(&from);
12         }
13       }
14     }
15   };
```
connect_to的实现绝对值得探讨。如您所见，它是一个模板成员函数，接受T，然后成对地迭代*this和T&的神经元，互相连接每个。但是有一个警告，我们不能只迭代*this，因为这会给我们一个SomeNeurons&和我们真正要找的类型。
这就是我们为什么被迫让一些神经元成为一个模板类，其中模板参数Self指的是继承类。然后我们在取消引用和迭代内容之前，将this指针转换为Self*。SomeNeurons<Neuron>是为了实现方便而付出的小小代价。
剩下的就是在Neuron和NeuronLayer中实现SomeNeurons::begin()和end()，让基于范围的循环真正工作。
由于NeuronLayer继承自vector，因此不用显示实现begin()/end()，它已经自动存在。但是神经元本身确实需要一种迭代的方法。它需要让自己成为唯一可重复的元素。这可以通过以下方式完成：
```c++
1   Neuron* begin() override { return this; }
2   Neuron* end() override { return this + 1; }
```
正是这个神奇的东西让SomeNeurons::connect_to()成为可能。简单来说，我们使得单个对象的行为像一个可迭代的对象集合。这允许以下所有用途：

```c++
1   Neuron neuron, neuron2;
2   NeuronLayer layer, layer2;
3
4   neuron.connect_to(neuron2);
5   neuron.connect_to(layer);
6   layer.connect_to(neuron);
7   layer.connect_to(layer2);
```
更不用说，如果您要引入一个新的容器（比如NeuronsRing），您所要做的就是从SomeNeurons<NeuronRing>继承，实现begin()/end()，新的类将立即连接到所有的神经元和神经元层。

#### 总结
复合设计模式允许我们为单个对象和对象集合提供相同的接口。这可以通过显式使用接口成员来完成，也可以通过duck typing（在程序设计中是动态类型的一种风格。在这种风格中，一个对象有效的语义，不是由继承自特定的类或实现特定的接口，而是由"当前方法和属性的集合"决定）来完成。例如基于范围的for循环并不需要您继承任何东西，而是通过实现begin()和end()。
正是这些begin()/end()成员允许标量类型伪装成“集合”。同样有趣的是，我们的connect_to()函数的嵌套for循环能够将这两个构造连接在一起，尽管它们具有不同的迭代器类型:Neuron返回Neuron*而NeuronLayer返回vector::iterator——这两者并不完全相同。哈哈，这就是模板的魅力。
最后，我必须承认，只有当你想拥有一个单一成员函数时，所有这些跳跃才有必要。如果您可以调用一个全局函数，或者如果您对有多个connect_to()实现感到满意，那么基类SomeNeurons并不是必要的。
