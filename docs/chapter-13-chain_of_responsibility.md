### 第13章 责任链

考虑一个典型的公司违规行为:内幕交易。
假设一个特定的交易员被当场抓住利用内幕消息进行交易。
这事该怪谁?如果管理层不知道，那就是交易员了。
但或许交易员的同行也参与其中，在这种情况下，集团经理可能要对此负责。
或者，这种做法是一种制度上的做法，在这种情况下，应该受到指责的是首席执行官。

这是责任链(Chain of Responsibility, CoR)的一个示例:系统中有几个不同的元素，它们都可以一个接一个地处理消息。
作为一个概念，它很容易实现，因为这意味着使用某种类型的列表。

#### 场景

想象一个电脑游戏，每个生物(`creature`)都有一个名字和两个属性值攻击(`attack`)和防御(`defense`):

```c++
struct Creature
{
    string name;
    int attack, defense;
    // 构造函数和<<运算符
};
```

现在，随着生物在游戏中的进展，它可能会遇到一个道具(例如，一把魔法剑)，或者它最终会被施魔法。
在任何一种情况下，它的攻击和防御值都将被我们称为`CreatureModifier`的东西修改。
此外，应用多个修改器的情况并不少见，所以我们需要能够在生物上堆叠修改器，允许它们按照被附加的顺序应用。
让我们看下是如何实现的。


#### 指针链

在经典的责任链(CoR)方式中，我们将实现`CreatureModifier`如下:

```c++
class CreatureModifier
{
    CreatureModifier* next {nullptr};
    protected:
        Creature& creature; // 可选：指针或共享指针
    public:
        explicit CreatureModifier(Creature& creature) : creature(creature) {}
    void add(CreatureModifier* cm)
    {
        if(next)
            next->add(cm);
        else
            next = cm;
    }
    virtual void handle()
    {
        if(next) next->handle(); // 关键之处！
    }
};
```

这里发生了很多事情，所以让我们依次讨论一下:

- 该类获取并存储它计划修改的生物的引用。
- 该类实际上做的不多，但它不是抽象的:它的所有成员都有实现。
- `next`成员指向一个可选的`CreatureModifer`。当然，这意味着它所指向的修改器是`CreatureModifer`的继承者。
- 添加另一个生物修改器到修改器链中。这是递归执行的:如果当前修饰符为`nullptr`，则将其设置为该值，否则遍历整个链并将其放到末端。
- 函数`handle()`只是处理链中的下一项，如果它存在的话;它没有自己的行为。它是虚函数，这意味着它需要被覆盖。

到目前为止，我们得到的只是一个可怜的只追加的单链表的实现。但是当我们开始继承它的时候，事情会变得更清晰。例如，以下是你如何制作一个修饰器，使生物的攻击值加倍。

```c++
class DoubleAttackModifier : public CreatureModifier
{
    public:
    explicit DoubleAttackModifier(Creature& creature) : CreatureModifier(creature) {}

    void handle() override
    {
        creature.attack *= 2;
        CreatureModifier::handle();
    }
};
```

好了，我们终于有进展了。因此，这个修饰器继承自`CreatureModifier`，并且在它的`handle()`方法中做两件事:使攻击值加倍，并从基类调用`handle()`。第二部分至关重要:应用修改器链的唯一方法是每个继承类都不要忘记在其自己的`handle()`实现的末尾调用基类的方法。

这是另一个更复杂的修饰器。该修饰器将攻击值为2或2以下的生物的防御能力增加1：


```c++
class DoubleAttackModifier : public CreatureModifier
{
    public:
    explicit DoubleAttackModifier(Creature& creature) : CreatureModifier(creature) {}

    void handle() override
    {
        if(creature.attack <= 2)
            creature.attack += 1;
        CreatureModifier::handle();
    }
};
```

同样，我们在最后调用基类。把所有这些放在一起，我们现在可以创建一个生物并对它应用一个修改器的组合：

```c++
Creature goblin{ "Goblin", 1, 1 };
CreatureModifier root{ goblin };
DoubleAttackModifier r1{ goblin };
DoubleAttackModifier r1_2{ goblin };
IncreaseDefenseModifier r2{ goblin };
root.add(&r1);
root.add(&r1_2);
root.add(&r2);
root.handle();
cout << goblin << endl;
// name: Goblin attack: 4 defense: 1
```

正如你所看到的，地精是4/1，因为它的攻击加倍了两次，并且防御调整值，虽然添加了，但并不影响它的防御值。

这里还有一个有趣的问题。假设你决定对一个生物施放一个不能加值的法术。这容易做吗?其实很简单，因为你要做的就是避免调用基类的`handle()`:这避免了执行整个链。

```c++
class NoBonusesModifier : public CreatureModifier
{
    public:
    explicit NoBonusesModifier(Creature& creature) : CreatureModifier(creature) {}

    void handle() override
    {
        // 在这里什么也不需要做！
    }
};
```

现在，如果将`NoBonusesModifier`插入链的开头，则不会再应用其他元素。

#### 代理链

指针链的例子是非常人工的。在现实世界中，你会希望生物能够任意承担和失去加成，这是一个仅追加链表所不支持的。此外，你不想永久地修改底层生物的属性(就像我们做的那样)，你想要保持临时修改。

实现CoR的一种方法是通过一个集中的组件。这个组件可以保存游戏中所有可用的修改器列表，并且可以通过确保所有相关的加值被应用来帮助查询特定生物的攻击或防御。

我们将要构建的组件称为*事件代理(`event broker`)*。由于它连接到每个参与组件，因此它表示中介者设计模式，而且，由于它通过事件响应查询，因此它利用了观察者设计模式。

让我们构建一个。首先，我们将定义一个名为Game的结构，它将代表正在玩的游戏:

```c++
struct Game // 中介者
{
    signal<void (Query&)> queries;
};
```

我们正在使用Boost.Signals2库，用于保存称为`queries`的信号。本质上，这让我们做的是发射这个信号(`signal`)，并由每个插槽`solt`(监听组件)处理它。但是事件与查询生物的攻击值或防御值有什么关系呢？

好吧，假设你想要查询一个生物的统计信息。你当然可以尝试读取一个字段，但请记住:我们需要在知道最终值之前应用所有修饰器。因此，我们将把查询封装在一个单独的对象中(这是命令模式)，定义如下：

```c++
struct Query
{
    string creature_name;
    enum Argument { attack, defense} argument;
    int result;
};
```

我们在前面提到的类中所做的一切都包含了从生物中查询特定值的概念。我们需要提供的只是生物的名称和我们感兴趣的统计信息。`Game::Query`将构建并使用这个值来应用修饰器并返回最终值。

现在，让我们来看看生物的定义。这和我们之前的很相似。在字段方面唯一的区别是`Game`的引用：

```c++
class Creature
{
    Game& game;
    int attack, defense;
    public:
        string name;
    Creature(Game& game, ...) : game { game }, ... { ... }
    // 其他成员函数
};
```

现在，注意`attack`和`defense`是私有的。这意味着，为了获得最终的(后修饰符)攻击值，你需要调用一个单独的getter函数：

```c++
int Creature::get_attack() const
{
    Query q{name, Query::Argument::attack, attack};
    game.queries(q);
    return q.result;
}
```

这就是奇迹发生的地方!我们不只是返回一个值或静态地应用一些基于指针的链，而是使用正确的参数创建一个`Query`，然后将查询发送给订阅`Game::queries`的任何人处理。每个订阅组件都有机会修改基线攻击值。

现在让我们来实现修改器。同样，我们将创建一个基类，但这一次它没有`handle()`方法:

```c++
class CreatureModifier:
{
    Game& game;
    Creature& creature;
    public:
        CreatureModifier(Game& game, Creature& creature) :
            game(game), creature(creature) 
        {}
};
```

因此修饰器基类并不是特别有趣。实际上，你完全可以不使用它，因为它所做的只是确保使用正确的参数调用构造函数。但是由于我们已经使用了这种方法，现在让我们继承`CreatureModifier`，看看如何执行实际的修改:

```c++
class DoubleAttackModifier : public CreatureModifier
{
    connection conn;
    public:
    DoubleAttackModifier(Game& game, Creature& creature)
    : CreatureModifier(game, creature)
    {
        conn = game.queries.connect([&](Query& q){
            if (q.creature_name == creature.name &&
                q.argument == Query::Argument::attack)
                q.result *= 2;
        });
    }
    ~DoubleAttackModifier() { conn.disconnect(); }
 };
```

如您所见，所有的乐趣都发生在构造函数和析构函数中;不需要其他方法。在构造函数中，我们使用`Game`引用获取`Game::queries`信号并连接到它，指定一个`lambda`表达式使攻击加倍。当然，`lambda`表达式必须做一些检查:我们需要确保我们增加了正确的生物(我们通过名称进行比较)，并且我们所追求的统计数据实际上是`attack`。这两条信息都保存在查询引用中，就像我们修改的初始结果值一样。


我们还注意存储信号连接，以便在对象被销毁时断开它。这样，我们可以暂时应用修改器，让它在修改器超出作用域时失效。

把它们放在一起，我们得到以下结果：

```c++
Game game;
Creature goblin{ game, "Strong Goblin", 2, 2 };
cout << goblin << endl;
// name: Strong Goblin attack: 2 defense: 2
{
DoubleAttackModifier dam{ game, goblin };
cout << goblin << endl;
// name: Strong Goblin attack: 4 defense: 2
}
cout << goblin << endl;
 // name: Strong Goblin attack: 2 defense: 2
```

这里发生了什么事?在被修改之前，地精是2/2。然后，我们制造一个范围，其中地精受到双重攻击修改器的影响，所以在范围内它是一个4/2的生物。一旦退出作用域，修改器的析构函数就会触发，并断开自己与代理的连接，因此在查询值时不再影响它们。因此，地精本身再次恢复为2/2的生物。

#### 总结

责任链是一种非常简单的设计模式，它允许组件依次处理命令(或查询)。`CoR`最简单的实现是创建一个指针链，从理论上讲，可以用一个普通的`vector`替换它，或者，如果希望快速删除，也可以用一个`list`。更复杂的代理链实现还利用中介模式和观察者模式允许我们处理查询事件(信号),在最终的值返回给客户端之前，让每个订阅者对最初传递的对象(它是贯穿整个链的单个引用)执行修改。