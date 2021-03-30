### 备忘录模式

首先看一个例子来认识备忘录模式。

#### 银行账户
下面是一个银行账户的类， 现在我们决定只使用deposit()函数创建一个银行账户。在前面的例子中，它是void的，而现在，deposit()将返回一个Memento。然后Memento可以回滚帐户到以前的状态:
```c++
class BankAccount
{ 
    int balance = 0;
    public: 
    explicit BankAccount(const int balance): balance(balance) {}
    Memento deposit(int amount)
    { 
        balance += amount;
        return { balance };
    }

    void restore(const Memento& m)
    { 
        balance = m.balance;
    } 
};
```

至于Memento本身，我们可以采用一个简单的实现方法:

```c++
class Memento
{ 
    int balance;
    public: 
        Memento(int balance): balance(balance)
    { 
    } 
    friend class BankAccount;
};
```

这里有两件事需要特别注意：

- Memento类是不可变的。想象一下，如果我们可以更改余额， 那么我们可以将帐户回滚到它从未处于的状态。

- 备忘录将BankAccount声明为一个友元类。这允许帐户实际使用balance字段。另一个同样有效的替代方法是将Memento作为BankAccount的内部类。

下面是备忘录模式的具体使用：
```c++
void memento()
{ 
    BankAccount ba{ 100 };
    auto m1 = ba.deposit(50);
    auto m2 = ba.deposit(25);
    cout << ba << "\n"; // Balance: 175
    // undo to m1
    ba.restore(m1);
    cout << ba << "\n"; // Balance: 150

    // redo
    ba.restore(m2);
    cout << ba << "\n"; // Balance: 175
}
```

这个实现已经足够好了，只是缺少了一些东西。例如，您永远不会得到表示开盘余额的Memento，因为构造函数不能返回值。

#### 撤销与回滚

如果我们要存储每一个由银行帐户产生的Memento，该怎么办呢? 我们将引入一个新的银行账户类BankAccount2，它将保存它生成的每一个Memento:
```c++
class BankAccount2 // supports undo/redo
{ 
    int balance = 0;
    vector<shared_ptr<Memento>> changes;
    int current;
    public: 
    explicit BankAccount2(const int balance) : balance(balance)
    { 
        changes.emplace_back(make_shared<Memento>(balance));
        current = 0;
    }
```

我们现在已经解决了返回初始余额的问题:初始变化的Memento也被存储。当然，这个Memento实际上并没有返回，所以为了回滚到它，我想您可以实现一些reset()函数——这完全取决于您。

我们使用shared_ptr来存储memento，也使用shared_ptr来返回它们。此外，我们使用当前字段作为进入更改列表的“指针”，如果我们决定撤消并后退一步，我们总是可以redo和恢复到我们之前的状态。

下面我们来看一下deposit()的实现:
```c++
shared_ptr<Memento> deposit(int amount)
{ 
    balance += amount;
    auto m = make_shared<Memento>(balance);
    changes.push_back(m);
    ++current;
    return m;
}
```
我们添加了一个方法, 该方法基于一个Memento来恢复帐户状态:
```c++
void restore(const shared_ptr<Memento>& m)
{ 
    if (m)
    { 
        balance = m->balance;
        changes.push_back(m);
        current = changes.size() - 1;
    } 
}
```
恢复的过程与我们之前看到的明显不同。首先，我们实际上检查shared_ptr是否已初始化。此外，当我们恢复一个memento时，我们实际上是将该memento推入更改列表，以使得一个undo操作能够正确地执行。

下面是undo()的实现:
```c++
shared_ptr<Memento> undo()
{ 
    if (current > 0)
    { 
        --current;
        auto m = changes[current];
        balance = m->balance;
        return m;
    }
    return{};
}
```

只有当current大于零时，我们才能使用undo()。此时，我们将指针后移并获取相应的changes成员，然后返回对应的balance。如果不能回滚到以前的memento，则返回一个默认构造的shared_ptr。

redo()的实现如下：
```c++
shared_ptr<Memento> redo()
{  
    if (current + 1 < changes.size())
    {  
        ++current;
        auto m = changes[current];
        balance = m->balance;
        return m;
    }
    return{};
}
```
同样，我们需要能够执行一些redo:如果可以，我们可以安全地redo，如果不行，我们什么都不做，并返回一个空指针。把它们结合起来，我们现在可以开始使用undo/redo功能了:

```c++
BankAccount2 ba{ 100 };
ba.deposit(50);
ba.deposit(25); // 125
cout << ba << "\n";
ba.undo();
cout << "Undo 1: " << ba << "\n"; // Undo 1: 150
ba.undo();
cout << "Undo 2: " << ba << "\n"; // Undo 2: 100
ba.redo();
cout << "Redo 2: " << ba << "\n"; // Redo 2: 150
ba.undo(); // back to 100 again
```