### 模板方法

策略模式和模版方法模式非常相似，就像工厂方法和工厂模式一样，我打算把这两个方法整合成一个设计模式来说明。

策略模式和模版方法的不同点在于，策略模式使用（静态或动态的）组合，然而模版方法使用继承。
但是核心的准则是在某个地方定义算法的框架，而在另外一个地方实现算法的细节，这符合开闭原则（在这里我们简单的扩展了下系统）


#### 游戏模拟

大多数棋类游戏都非常相似:游戏开始(先进行一些设置)，
玩家轮流游戏，直到决定了获胜者，然后便可以宣布获胜者。不管游戏是国际象棋、跳棋或其他什么，
我们都可以定义如下算法：

```c++
class Game
{
    public:
        void run()
        {
            start();
            while(!have_winner())
                take_turn();
            cout << "Player " << get_winner() << "wins.\n";
        }
};
```

正如你所看到的一样，`run()`方法将游戏运行起来，并简单的调用一系列其他方法。这些方法都被定义为纯虚的，同时声明在保护
字段，使得其他非派生类无法调用这些方法。

```c++
protected:
    virtual void start() = 0;
    virtual void bool have_winner() = 0;
    virtual void take_turn() = 0;
    virtual int get_winner() = 0;
```
公平的说，上面的一些方法，尤其是返回值为`void`，并不是必须要定义为纯虚的。例如，如果一些游戏没有显示的`start()`方法，把`start()`方法声明为纯虚的就违法了里氏替换原则，因为子类中其实并不需要这个方法，但是也必须得实现这一接口方法。在策略模式那章中
我们故意的使用了不采用任何操作的虚函数方法，但是在模版方法里面这样的例子就显得不那么清楚。

现在我们在之前的基础上，加上一些和所有游戏相关的共有方法：玩家的个数和当玩家的索引。

```c++
class Game
{
    public:
        explicit Game(int number_of_players) :
            number_of_players(number_of_players),
            current_player{0} 
            {}
    protected:
        int current_player;
        int number_of_players;
};// 省略其他成员
```

`Game`类可以被拓展出来实现象棋（`chess`）游戏。

```c++
class Chess : public Game
{
    public:
        explicit Chess() : Game { 2 } { }
    protected:
        void start( ) override { }
        bool have_winner( ) override { return turns == max_turns; }
        void take_turn( ) override
        {
            turns++;
            current_player = ( current_player + 1 ) % number_of_players;
        }
        int get_winner( ) override { return curren_player; }

    private:
        int turns{ 0 };
        int max_turns{ 10 };

}
```

象棋游戏涉及两个玩家，在构造函数中把参数2传递给父类。然后我们重写了所有必要的函数，实现了简单的模拟逻辑，游戏在第10轮结束。下面是输出：

```c++
1   Starting a game of chess with 2 players
2   Turn 0 taken by player 0
3   Turn 1 taken by player 1
4 ...
5   Turn 8 taken by player 0
6   Turn 9 taken by player 1
7   Player 0 wins.
```

#### 总结

与使用组合并分为静态和动态的策略模式不同，模板方法使用继承，
因此，它只能是静态的，因为一旦对象被构造，就没有办法操纵它的继承特性

模板方法中唯一的设计决策是，你想让模板方法使用的方法是纯虚的，
还是实际上有一个主体(即使该主体是空的)。
如果你预见到一些方法对所有的继承者来说都是不必要的，那就把它们变成无操作的方法。