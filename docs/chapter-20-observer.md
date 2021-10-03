### 观察者

观察者模式是一种流行且必需的模式，但是令人惊讶的是，与其他语言（例如C#）相比，C++和标准库都没有现成的实现。然而，实现一个安全的、正确的观察者（如果存在这样的东西的话）从技术上来说是比较复杂的。在这章中我们将研究它的细节。

#### 属性观察者

人都会变老，这是生活的事实。但是当一个人长大一岁的时候，我们可能想要庆祝他的生日。但是要怎么实现呢？可以给出下面这样一个定义：

```c++
struct Person {
  int age;
  Person(int age) : age{age} {}
};
```

但是我们怎么知道一个人的年龄(age)什么时候发生改变呢？我们不知道。如果需要知道变化，我们可以尝试轮询:每100毫秒读取一个人的年龄，并将新值与之前的值进行比较。这种方法是可行的，但是很繁琐而且不能扩展，我们需要采用更聪明的方法。

当每一个改变年龄字段的写操作发生时，我们想要被通知到。捕捉这个通知的的唯一方法是创建setter。

```c++
struct Person {
  int get_age() const { return age; }
  void set_age(const int value) { age = value; }

 private:
  int age;
};
```

`setter set_age()` 可以通知任何关心年龄变化的人，但是怎么做到的呢？

#### Observer\<T>

一种方法是定义某种类型的基类，任何对获取Person变化感兴趣的人都需要继承自这种基类。

```c++
struct PersonListener {
  virtual void person_changed(Person &p, const string &property_name,
                              const any_new_value) = 0;
};
```

然而，这种方法非常繁琐，因为属性更改可以发生在Person以外的类型上，而且我们也不希望为这些类型生成额外的类。这里使用更通用的定义:

```c++
template <typename T>
struct Observer {
  virtual void field_changed(T &source, const string &field_name) = 0;
};
```

field_changed()中的两个参数希望是自解释的。第一个是对其字段实际更改的对象的引用，第二个是字段的名称。是的，名称是作为字符串传递的，这确实损害了代码的可重构性(如果字段名称改变了怎么办?[^1]

注1：c#在连续的版本中两次明确地解决了这个问题。首先，它引入了一个名为`CallerMemberName`的属性，该属性将调用函数/属性的名称作为参数的字符串值插入。第二个版本简单地引入了nameof(Foo)，它将取符号的名称并将其转换为字符串。

这个实现将允许我们观察Person类的变化，例如，将它们写到终端:

```c++
struct ConsolePersonObserver : Observer<Person> {
  void field_changed(Person &source, const string &field_name) override {
    cout << "Person's " << field_name << " has changed to " << source.get_age()
         << ".\n";
  }
};
```

例如，我们在场景中引入的灵活性允许我们观察多个类的属性变化。例如，如果我们把Creature类加入，现在就可以同时观察这两个类:

```c++
struct ConsolePersonObserver : Observer<Person>, Observer<Creature> {
  void field_changed(Person &source, const string &field_name) {}
  void field_changed(Creature &source, const string &field_name) {}
};
```

另一种替代方法是使用std::any，并去掉泛型实现。试一试!


#### Observable<T>

不管怎样，让我们回到Person。由于这将成为一个可观察类，它将不得不承担新的责任，即:

- 把关心Person变化的所有观察者放置在自己的私有段
- 让观察者可以订阅或者取消订阅 subscribe()/unsubscribe() 发生在Person上的变化。
- 当Person发生改变的时候通知所有的观察者。


所有这些功能都可以移到一个单独的基类中，这样就可以避免为每个潜在的可观察对象复制它。

```c++
template <typename T>
struct Observable {
  void notify(T& source, const string& name);
  void subscrible(Observer<T>* f) { observers.emplace_back(f); };
  void unsubscrible(Observer<T>* f);

 private:
  vector<Observer<T*>> observers;
};
```

我们实现了subscribe()，它只是将一个新的观察者添加到观察者的私有列表中。观察者列表对其他类都不可用，甚至对派生类也不可用。我们不希望其他类随意操纵观察者集合。

接下来，我们需要实现`notify()`函数。想法上很简单：遍历每个观察者，并且依次调用对应的`field_changed()`函数。

```c++
void notify(T &source, const string &name) {
  for (auto &&obs : observers) observes->field_changed(source, name);
}
```

不过只从`Observable<T>`类继承是不够的，我们的类还需要在其字段发生改变的时候，把自己作为参数调用`notify()`函数。

例如，考虑setter `set_age()`函数，它现在有三个职责：

- 检查被观察字段是否已经实际更改。如果年龄是20岁，我们把它赋值为20岁，那么任何的赋值或通知就没有意义了。
- 给被观察的字段赋合理的值。
- 用正确的参数调用notify()函数

因此，set_age()的新实现可能长成这样:

```c++
struct Person : Observable<Person> {
  void set_age(const int age) {
    if (this->age != age) {
      // check_age(age);
      this->age = age;
      notify(*this, "age");
    }
  }

 private:
  int age;
};
```

### 连接观察者和被观察者

现在，我们已经准备好开始使用我们创建的基础设施，以便获得关于人员字段更改的通知(实际上，我们可以称其为属性)。下面是我们的观察者的样子:

```c++
struct ConsolePersonObserver : Observer<Person> {
  void field_changed(Person &source, const string &field_name) override {
    cout << "Person's " << field_name << " has changed to " << source.get_age()
         << ".\n";
  }
};
```
我们可以这样使用：

```c++
Person p{ 20 };
ConsolePersonObserver cpo;
p.subscribe(&cpo);
p.set_age(21); // Person's age has changed to 21.
p.set_age(22); // Person's age has changed to 22.
```

如果你不关心与属性依赖关系和线程安全性/可重入性有关的问题，就可以在这此止步，采用这个实现，并开始使用它。如果你想看到关于更复杂方法的讨论，请继续阅读。

#### 依赖问题

大于16岁的人具有选举权，因此当某个人具有选举权之后我们希望被通知到。首先，我们假设Person类有如下的getter函数：

```c++
bool get_can_vote() const { return age >= 16};
```

注意，get_can_vote()没有支持字段和setter(我们可以引入这样的字段，但它显然是多余的)，但是我们也觉得有必要在它上面通知()。但如何?我们可以试着找出是什么原因导致can_vote改变了它，是set_age()做的！因此，如果我们想要通知投票状态的变化，这些需要在set_age()中完成。准备好，你会大吃一惊的。

```c++
void set_age(const int value) const {
  if (age != value) {
    auto old_can_vote = can_vote();  // store old value
    age = value;
    notify(*this, "age");

    if (old_can_vote != can_vote())  // check value has changed
      notify(*this, "can_vote");
  }
}
```

前面的函数太多了。我们不仅检查年龄是否改变，我们也检查can_vote是否改变，并通知它!你可能会认为这种方法不能很好地扩展，对吧?想象一下can_vote依赖于两个字段，比如age和citizenship——这意味着它们的两个setter都必须处理can_vote通知。更糟糕的是，如果年龄也会以这种方式影响其他10种属性呢?这是一个不可用的解决方案，它会导致脆弱的代码无法维护，因为变量之间的关系需要手动跟踪。

坦白地说，在前一种情况下，can_vote属性依赖age属性。依赖性属性的挑战本质上是Excel等工具的挑战:给定不同单元格之间的大量依赖性，当其中一个单元格发生变化时，您如何知道哪些单元格需要重新计算。

当然，属性依赖关系可以被形式化为某种类型的`map<string, vector<string>>`。这将保留一个受属性影响的属性列表(或者相反，影响属性的所有属性)。遗憾的是，这个`map`必须手工定义，而且要与实际代码保持同步是相当棘手的。

#### 取消订阅和线程安全

我忘记讨论的一件事是观察者如何从可观察对象中取消订阅。通常，您希望从观察者列表中删除自己，这在单线程场景中非常简单：

```c++
void unsubscribe(Observer<T>* observer) {
  observers.erase(remove(observers.begin(), observers.end(), observer),
                  observers.end())
}
```

虽然`erase-remove`习惯用法在技术上是正确的，但它只在单线程场景中是正确的。vector不是线程安全的，所以同时调用subscribe()和unsubscribe()可能会导致意想不到的结果，因为这两个函数都会修改vector。

这很容易解决:只需对所有可观察对象的操作都加一个锁。这看起来很简单：

```c++
template <typename T>
struct Observable {
  void notify(T& source, const string& name) {
    scoped_lock<mutex> lock{mtx};
    ...
  }
  void subscribe(Observer<T>* f) {
    scoped_lock<mutex> lock{mtx};
    ...
  }
  void unsubscribe(Observer<T>* o) {
    scoped_lock<mutex> lock{mtx};
    ...
  }

 private:
  vector<Observer<T>*> observers;
  mutex mtx;
};
```

另一个非常可行的替代方案是使用类似`TPL/PPL`的`concurrent_ vector`。当然，您会失去排序保证(换句话说，一个接一个地添加两个对象并不能保证它们按照那个顺序得到通知)，但它肯定会让您不必自己管理锁。

### 可重入

最后一种实现提供了一些线程安全性，只要有人需要，就锁定这三个关键方法中的任何一个。但是现在让我们设想以下场景:您有一个交通管理组件，它一直监视一个人，直到他到了可以开车的年龄。当他们17岁时，组件就会取消订阅:

```c++
struct TrafficAdministration : Observer<Person> {
  void TrafficAdministration::field_changed(Person& source,
                                            const string& field_name) override {
    if (field_name == "age") {
      if (source.get_age() < 17)
        cout << "Whoa there, you are not old enough to drive!\n";
      else {
        // oh, ok, they are old enough, let's not monitor them anymore
        cout << "We no longer care!\n";
        source.unsubscribe(this);
      }
    }
  }
};
```

这将会出现一个问题，因为当某人17岁时，整个调用链将会是：

> notify() --> field_changed() --> unsubscribe()

这是一个问题，因为在unsubscribe()中，我们最终试图获取一个已经被获取的锁。这是一个可重入问题。处理这件事有不同的方法:

- 一种方法是简单地禁止这种情况。毕竟，至少在这个特定的例子中，很明显这里发生了可重入性
- 另一种方法是放弃从集合中删除元素的想法。相反，我们可以这样写:

```c++
void unsubscribe(Observer<T>* o) {
  auto it = find(observers.begin(), observers.end(), o);
  if (it != observers.end()) *it = nullptr;  // cannot do this for a set
}
```

随后，当使用notify()时，只需要进行额外的检查:

```c++
void notify(T& source, const string& name) {
  for (auto&& obs : observes)
    if (obs) obs->field_changed(source, name);
}
```

#### 通过 Boost.Signals2 来实现 Observer

观察者模式有很多预打包的实现，并且可能最著名的是 `Boost.Signals2` 库。本质上，该库提供了一种称为信号的类型，它表示 `C++` 中的信号术语（在别处称为事件）。可以通过提供函数或 `lambda` 表达式 来订阅此信号。它也可以被取消订阅，当你想通知它时，它可以被解除。

```c++
template <typename T>
struct Observable {
  signal<void(T&, const string&)> property_changed;
};
```

它的调用如下所示:

```c++
struct Person : Observable<Person> {
  void set_age(const int age) {
    if (this->age == age) return;
    this->age = age;
    property_changed(*this, "age");
  }
};
```


API 的实际使用将直接使用信号，当然，除非你决定添加更多 API 陷阱以使其更容易：

```c++
Person p{123};
auto conn = p.property_changed.connect([](Person&, const string& prop_name) {
  cout << prop_name << " has been changed" << endl;
});
p.set_age(20);  // name has been changed
// later, optionally
conn.disconnect();
```

`connect()` 调用的结果是一个连接对象，它也可以用于在你不再需要信号通知时取消订阅。

#### 总结

毫无疑问，本章中提供的代码是一个明显的例子，它过度思考和过度设计了一个超出大多数人想要实现的问题的方式。

让我们回顾一下实现 Observer 时的主要设计决策：

- 决定你希望你的 observable 传达什么信息。例如，如果你正在处理字段/属性更改，则可以包含属性名称。你还可以指定旧/新值，但传递类型可能会出现问题。
- 你想让你的观察者成为`tire class`，还是你只需要一个虚函数列表？
- 你想如何处理取消订阅的观察者？

    - 如果你不打算支持取消订阅——恭喜你，你将节省大量的实现观察者的工作，因为在重入场景中没有删除问题。
    - 如果你计划支持显式的 `unsubscribe()` 函数，你可能不想直接在函数中擦除-删除，而是将元素标记为删除并稍后删除它们。
    - 如果你不喜欢在（可能为空）裸指针上调度的想法，请考虑使用 `weak_ptr` 代替。
 - `Observer<T>` 的函数是否有可能是 从几个不同的线程调用？如果他们是，你需要保护你的订阅列表：
    - 你可以在所有相关函数上放置 `scoped_lock`；或者
    - 你可以使用线程安全的集合，例如 `TBB/PPLcurrenct_vector`。你将失去顺序保证。
- 来自同一来源的多个订阅允许吗？如果是，则不能使用 `std::set`。


遗憾的是，没有理想的 `Observer` 实现能够满足所有条件。 无论你采用哪种实现方式，都需要做出一些妥协。
