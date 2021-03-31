### 第24章：命令模式

#### 场景
让我们试着模拟一个有余额和透支限额的银行账户。我们将实现deposit()和withdraw()功能：
```c++
1   struct BankAccount
2   {
3     int balance = 0;
4     int overdraft_limit = -500;
5
6     void deposit(int amount)
7     {
8       balance += amount;
9       cout << "deposited " << amount << ", balance is now " <<balance << "\n";
10         
11     }
12
13     void withdraw(int amount)
14     {
15       if (balance - amount >= overdraft_limit)
16       {
17         balance -= amount;
18         cout << "withdrew " << amount << ", balance is now " << balance << "\n";
19           
20       }
21     }
22   };
```
当然，现在我们可以直接调用成员函数，但是让我们假设，出于审计的目的，我们需要记录每一次存款和取款，我们不能在银行帐户中这样做，因为我们已经设计、实现并测试了这个类。

#### 实现命令模式
我们将首先为命令定义一个接口

```c++
1   struct Command
2   {
3     virtual void call() const = 0;
4   };
```

定义好接口后，我们可以使用它来定义一个银行账户命令，该命令将封装关于如何处理银行帐户的信息:

```c++
1   struct BankAccountCommand : Command
2   {
3     BankAccount& account;
4     enum Action { deposit, withdraw } action;
5     int amount;
6
7      BankAccountCommand(BankAccount& account, const Action
8     action, const int amount)
9       : account(account), action(action), amount(amount) {}
```

命令中的信息包含以下内容
•  要操作的账户
•  要采取的行动；选项的集合和存储选项的变量都在一个声明中定义
•  存入或提取的金额

一旦客户提供了此信息，我们就可以获取并使用它来执行存款或取款

```c++
1   void call() const override
2   {
3     switch (action)
4     {
5     case deposit:
6       account.deposit(amount);
7       break;
8     case withdraw:
9       account.withdraw(amount);
10       break;
11     }
12   }
```

通过这种方法，我们可以创建命令，然后对该命令执行帐户权限的修改:

```c++
1   BankAccount ba;
2   Command cmd{ba, BankAccountCommand::deposit, 100};
3   cmd.call();
```

这将把100元存入我们的账户。轻松！如果您担心我们仍然向客户公开原始的deposit()和withdraw()成员函数，您可以将他们设为私有，只需将BankAccountCommand指定为友元类。

#### 撤销操作

由于一个命令封装了关于对银行帐户的某些修改的所有信息，它同样可以回滚这个修改，并将其目标对象返回到其先前的状态。
首先，我们需要决定是否将撤销相关操作插入我们的Command接口。为了简洁起见，我们将在这里做，但总的来说，这是一个设计决策，需要尊重我们在本书开头(第1章)讨论的接口隔离原则。例如，如果您设置一些命令是final的，并且不受撤销机制的约束。比如说，将命令分为可调用的和可撤销的，是有意义的。
不管怎样，这是已经更新的命令；注意我已经有意地从函数中删除了const：

```c++
1   struct Command
2   {
3     virtual void call() = 0;
4     virtual void undo() = 0;
5   };
```

下面是BankAccountCommand::undo()的一个简单实现，其动机是（不正确的）假设账户存款和取款是对称的操作。

```c++
1   void undo() override
2   {
3     switch (action)
4     {
5     case withdraw:
6       account.deposit(amount);
7       break;
8     case deposit:
9       account.withdraw(amount);
10       break;
11     }
12   }
```

为什么说这个实现会崩溃？因为如果你试图提取相当于一个发达国家GDP的金额，你就不会成功，但当回滚交易时，我们没有办法说它失败了！为了获得这些消息，我们修改了withdraw()去返回一个成功的标记。

```c++
1   bool withdraw(int amount)
2   {
3     if (balance - amount >= overdraft_limit)
4     {
5       balance -= amount;
6       cout << "withdrew " << amount << ", balance now " <<
7         balance << "\n";
8       return true;
9     }
10     return false;
11   }
```

这样就好多了！我们现在可以修改整个BankAccountCommand去做两件事：
• 取款时在内部存储一个成功标志。
• 当调用undo()时使用这个标志

```c++
1   struct BankAccountCommand : Command
2   {
3     ...
4     bool withdrawal_succeeded;
5
6     BankAccountCommand(BankAccount& account, const Action action,
7       const int amount)
8       : ..., withdrawal_succeeded{false} {}
9
10     void call() override
11     {
12       switch (action)
13       {
14         ...
15       case withdraw:
16         withdrawal_succeeded = account.withdraw(amount);
17         break;
18       }
19     }
```

你现在明白为什么我要把const从Command中移除了吗？现在我们分配了一个成员变量withdrawal_succeeded，我们不能再声称call()是const的。我想我可以把它保存在undo()上，但这没有什么好处。好了，现在我们有了标志，我们可以改进undo()的实现。

```c++
1   void undo() override
2   {
3     switch (action)
4     {
5     case withdraw:
6       if (withdrawal_succeeded)
7         account.deposit(amount);
8       break;
9       ...
10     }
11   }
```

我们最终可以以一致的方式撤销withdraw()命令。当然本练习的目的是为了说明，除了存储要执行的操作的信息之外，命令还可以存储一些中间信息。这些信息同样对于审计之类的事情有用：如果您检测到100次的withdraw()操作，您可以调查潜在的黑客攻击。

#### 组合命令
一次从账户A到账户B的转账，可以用两个命令模拟：

    1.从A中取X元
    2.从B中存X元

如果我们可以创建并调用一个封装这两个命令的命令，而不是调用两个命令，那就太好了。
这就是我们再第8章讨论的组合设计模式的本质。
让我们定义一个组合命令。我们将从vector<BankAccountCommand>继承。这可能会有问题，因为std::vector没有虚析构函数，但在我们的情况下这不是问题，这里有一个非常简单的定义：

```c++
1    struct CompositeBankAccountCommand : vector 
<BankAccountCommand>, Command
 2   {
 3      CompositeBankAccountCommand(const initializer_list 
<value_type>& items)
 4       : vector<BankAccountCommand>(items) {}
 5
 6     void call() override
 7     {
 8       for (auto& cmd : *this)
 9         cmd.call();
10     }
11
12     void undo() override
13     {
14       for (auto it = rbegin(); it != rend(); ++it)
15         it->undo();
16     }
17   };
```

如您所见，我们所做的只是重用基类构造函数，用两个命令初始化对象。然后重用基类的call()/undo()实现。但是等等，这不对，是吗？基类实现并没有完全切断它，因为它没有包含失败的情况。例如，如果我不能从A处取钱，我就不应该把钱存到B处：整个链应该自行取消。
为了支持这个想法，我们需要更激烈的改变，我们需要：
•  向Command增加成功标记
•  记录每次操作的成功或失败
•  确保该命令只有在最初成功时才能撤销
•  引入一个新的中间类，名为DependentCompositeCommand，它在实际回滚命令时非常小心

当调用每个命令时，我们只有在前一个成功的情况下才会这样做；否则，我们只需将成功标志设置为false。

```c++
1   void call() override
2   {
3     bool ok = true;
4     for (auto& cmd : *this)
5     {
6       if (ok)
7       {
8         cmd.call();
9         ok = cmd.succeeded;
10       }
11       else
12       {
13         cmd.succeeded = false;
14       }
15     }
16   }
```

没有必要覆盖undo()，因为我们的每个命令都会检查自己的成功标志，并且只有在设置为true时才撤销操作。
你可以想象一个更强的形式，一个复合命令只有在它的所有部分都成功的情况下才会成功(想想一个转账，取款成功，存款失败——你想让它通过吗？)—这有点难以实现，我再次把它留给读者作为练习。本节的全部目的是说明当考虑到现实世界的业务需求时，简单的基于命令的方法会变得多么复杂。你是否真的需要这种复杂性……嗯，这取决于你。


#### 命令查询分离

命令查询分离(CQS)的概念是系统中的操作大致分为以下两类：

    •  命令，这是系统执行某些操作的指令，这些操作涉及状态的变化，但不产生任何值
    •  查询，这是对信息的请求，产生值但不改变状态

任何目前直接公开其状态供读写的对象都可以隐藏其状态(使其为私有)，然后可以提供一个单独的接口，而不是提供getter和setter对。我的意思是:假设我们有一种生物具有力量和敏捷这两种属性。我们可以这样定义该生物:

```c++
1   class Creature
2   {
3     int strength, agility;
4   public:
5     Creature(int strength, int agility)
6       : strength{strength}, agility{agility} {}
7
8     void process_command(const CreatureCommand& cc);
9     int process_query(const CreatureQuery& q) const;
10   };
```

如您所见，没有getter和setter，但我们有两个(只有两个!)称为process_command()和process_query()的API成员，它们将用于与生物对象的所有交互。这两个都是专用类，连同CreatureAbility枚举，定义如下:

```c++
1   enum class CreatureAbility { strength, agility };
2
3   struct CreatureCommand
4   {
5     enum Action { set, increaseBy, decreaseBy } action;
6     CreatureAbility ability;
7     int amount;
8   };
9
10   struct CreatureQuery
11   {
12     CreatureAbility ability;
13   };
```

如您所见，该命令描述了您想要更改的成员、您想要如何更改以及更改多少。查询对象只指定要查询的内容，并且我们假定查询的结果从函数返回，而不是在查询对象本身中进行设置(如果其他对象影响这个对象，如我们已经看到的，那么您将这样做)。下面是process_command()的定义:

```c++
1   void Creature::process_command(const CreatureCommand &cc)
2   {
3     int* ability;
4     switch (cc.ability)
5     {
6       case CreatureAbility::strength:
7         ability = &strength;
8         break;
9       case CreatureAbility::agility:
10         ability = &agility;
11         break;
12     }
13     switch (cc.action)
14     {
15       case CreatureCommand::set:
16         *ability = cc.amount;
17         break;
18       case CreatureCommand::increaseBy:
19         *ability += cc.amount;
20         break;
21       case CreatureCommand::decreaseBy:
22         *ability -= cc.amount;
23         break;
24     }
25   }
```

下面是更简单的process_query()定义:

```c++
1   int Creature::process_query(const CreatureQuery &q) const
2   {
3     switch (q.ability)
4     {
5       case CreatureAbility::strength: return strength;
6       case CreatureAbility::agility: return agility;
7     }
8     return 0;
9   }
```

如果您想要记录这些命令和查询的日志或持久性，现在只有两个位置需要完成这一点。所有这些真正的问题是，对于只想以熟悉的方式操作对象的人来说，使用API有多么困难。幸运的是，只要我们愿意，我们总是可以制造getter/setter对;这些函数只需要使用适当的参数来调用process_方法:

```c++
1   void Creature::set_strength(int value)
2   {
3     process_command(CreatureCommand{
4       CreatureCommand::set, CreatureAbility::strength, value
5     });
6   }
7
8   int Creature::get_strength() const
9   {
10      return process_query(CreatureQuery{CreatureAbility:: 
strength});
11   }
```

无可否认，前面的例子非常简单地说明了在执行CQS的系统中实际发生的情况，但它很有希望提供一个概念，说明如何将所有对象接口拆分为命令和查询部分。

#### 总结

命令设计模式很简单:它的基本建议是，对象可以使用封装指令的特殊对象彼此通信，而不是将这些相同的指令指定为方法的参数。有时，您不希望这样的对象改变目标或使它做一些特定的事情;相反，您希望使用这样的对象从目标查询一个值，在这种情况下，我们通常将这样的对象称为查询。虽然在大多数情况下，查询是一个依赖于方法返回类型的不可变对象，但在某些情况下(例如，参见Chain of Responsibility Broker Chain的例子;)当你希望被返回的结果被其他组件修改时。但是组件本身仍然没有修改，只是结果是。命令在UI系统中被大量使用来封装典型的操作(例如，复制或粘贴)，然后允许通过几种不同的方式调用单个命令。例如，您可以使用顶级应用程序菜单、工具栏上的按钮或键盘快捷键进行复制。最后，这些动作可以被组合成宏动作序列，这些宏动作序列可以被记录下来，然后随意重放。


