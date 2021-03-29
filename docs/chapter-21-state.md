### 第21章: 状态模式

我必须承认:我的行为是由我的状态支配的。如果我没有足够的睡眠，我会有点累。如果我喝了酒，我就不会开车了。所有这些都是*状态(states)*，它们支配着我的行为:我的感受，我能做什么，我不能做什。

当然，我可以从一种状态转换到另一种状态。我可以去喝杯咖啡，它能让我从瞌睡中清醒过来(我希望如此!)所以我们可以把咖啡当作触发器，让你真正从困倦过渡到清醒。这里，让我笨拙地为你解释一下：

```c++
         coffee
sleepy ----------> alert
```

所以，状态设计模式是一个非常简单的想法:状态控制行为;状态可以改变;唯一的问题是*谁*引发了状态的变更。

基本上有两种方式:

- 状态是带有行为的实际类，这些行为将实际状态从一个转换到另一个
- 状态和转换只是枚举。我们有一个称为`状态机(state machine)`的特殊组件，它执行实际的转换。

这两种方法都是可行的，但实际上第二种方法是最常见的。这两种我们都会过一遍，但我必须承认我只会简单浏览第一个，因为这不是人们通常做事情的方式。


#### 状态驱动的状态机

我们将从最简单的例子开始:一个电灯开关。它只能处于开和关的状态。我们将构建一个任何状态都能够切换到其他状态的模型:虽然这反映了状态设计模式的经典实现(根据GoF的书)，但我并不推荐这样做。


首先，让我们为电灯开关建模:它只有一种状态和一些从一种状态切换到另一种状态的方法:

```c++
class LightSwitch
{
    State* state;
    public:
        LightSwitch()
        {
            state = new OffState();
        }
        void set_state(State* state)
        {
            this->state = state;
        }
};
```

这一切看起来都很合理。我们现在可以定义状态，在这个特定的情况下，它将是一个实际的类:

```c++
struct State
{
    virtual void on(LightSwitch* ls)
    {
        cout << "Light is already on\n";
    }
    virtual void off(LightSwitch* ls)
    {
        cout << "Light is already off\n";
    }
};
```

这个实现很不直观，所以我们需要慢慢地仔细地讨论它，因为从一开始，关于State类的任何东西都没有意义。

首先，`State`不是抽象的!你会认为一个你没有办法(或理由)达到的状态是抽象的。但事实并非如此。

第二，状态允许从一种状态切换到另一种状态。这对一个通情达理的人来说，毫无意义。想象一下电灯开关:它是改变状态的开关。人们并不指望`State`本身会改变自己，但它似乎就是这样做的。

第三，也许是最令人困惑的，`State::on/off`的默认行为声称我们已经处于这种状态!在我们实现示例的其余部分时，这一点将在某种程度上结合在一起。

现在我们实现`On`和`Off`状态:

```c++
struct OnState : State
{
    OnState() { cout << "Light turned on\n"; }
    void off(LightSwitch* ls) override;
};

struct OffState : State
{
    OffState() { cout << "Light turned off\n"; }
    void on(LightSwitch* ls) override;
};
```

实现OnState::off和OffState::on允许状态本身切换到另一个状态!它看起来是这样的:

```c++
void OnState::off(LightSwitch* ls)
{
    cout << "Switching light off...\n";
    ls->set_state(new OffState());
    delete this;
} // same for OffState::on
```

这就是转换发生的地方。这个实现包含了对`delete This`的奇怪调用，这在真实的c++中是不常见的。这对初始分配状态的位置做出了非常危险的假设。例如，可以使用智能指针重写这个示例，但是使用指针和堆分配清楚地表明状态在这里被积极地销毁。如果状态有析构函数，它将触发，你将在这里执行额外的清理。

当然，我们确实希望开关本身也能切换状态，就像这样：


```c++
class LightSwitch
{
    ...
    void on() { state->on(this); }
    void off() { state->off(this); }
};
```

因此，把所有这些放在一起，我们可以运行以下场景:

```c++
1 LightSwitch ls; // Light turned off
2 ls.on(); // Switching light on...
3 // Light turned on
4 ls.off(); // Switching light off...
5 // Light turned off
6 ls.off(); // Light is already off
```

我必须承认:我不喜欢这种方法，因为它不是直观的。当然，状态可以被告知(观察者模式)我们正在进入它。但是，状态转换到另一种状态的想法——根据GoF的书，这是状态模式的经典实现——似乎不是特别令人满意。

如果我们笨拙地说明从`OffState`到`OnState`的转换，则需要将其说明为

```c++
          LightSwitch::on() -> OffState::on()
OffState -------------------------------------> OnState
```

另一方面，从OnState到OnState的转换使用基状态类，这个类告诉你你已经处于那个状态

```c++
        LightSwitch::on() -> State::on()
OnState ----------------------------------> OnState
```

这里给出的示例可能看起来特别人为，所以我们现在将看看另一个手工创建的设置，其中的状态和转换被简化为枚举成员。

#### 手工状态机

让我们尝试为一个典型的电话会话定义一个状态机。首先，我们将描述电话的状态:

```c++
enum class State
{
    off_hook,
    connecting,
    connected,
    on_hold,
    on_hook
};
```

我们现在还可以定义状态之间的转换，也可以定义为`enum class`:

```c++
enum class Trigger
{
    call_dialed,
    hung_up,
    call_connected,
    placed_on_hold,
    taken_off_hold,
    left_message,
    stop_using_phone
};
```

现在，这个状态机的确切规则，即可能的转换，需要存储在某个地方。

```c++
map<State, vector<pair<Trigger, State>>> rules;
```


这有点笨拙，但本质上`map`的键是我们移动的状态，值是一组表示`Trigger-State`的对，在此状态下可能的触发器以及使用触发器时所进入的状态。

让我们来初始化这个数据结构：

```c++
rules[State::off_hook] = {
    {Trigger::call_dialed, State::connecting},
    {Trigger::stop_using_phone, State::on_hook}
};

rules[State::connecting] = {
    {Trigger::hung_up, State::off_hook},
    {Trigger::call_connected, State::connected}
};
//  more rules here
```

我们还需要一个启动状态，如果我们希望状态机在达到该状态后停止执行，我们还可以添加一个退出(终止)状态：

```c++
State currentState { State::off_hook },
exitState { State::on_hook };
```

完成这些之后，我们就不必为实际运行(我们使用`orchestrating`这个术语)状态机而构建单独的组件了。例如，如果我们想要构建电话的交互式模型，我们可以这样做:

```c++
while(true)
{
    cout << "The phone is currently " << currentState << endl;
    
    select_trigger:
        cout << "Select a trigger:" << "\n";
    
    int i = 0;
    for(auto &&item : rules[currentState])
    {
        cout << i++ << ". " << item.first << "\n";
    }

    int input;
    cin >> input;
    for(input < 0 || (input+1) > rules[currentState].size())
    {
        goto select_trigger;
    }

    currentState = rules[currentState][input].second;
    if(currentState == exitState) break;
}
```

首先:是的，我确实使用`goto`，这是一个很好的例子，说明在什么地方使用`goto`是合适的（译者注：一般不建议在程序里面使用goto,这样会使得程序的控制流比较混乱）。对于算法本身，这是相当明显的:我们让用户在当前状态上选择一个可用的触发器(`operator<<`状态和触发器都在幕后实现了)，并且，如果触发器是有效的，我们通过使用前面创建的规则映射转换到它。

如果我们到达的状态是退出状态，我们就跳出循环。下面是一个与程序交互的示例。

```
1 The phone is currently off the hook
2 Select a trigger:
3 0. call dialed
4 1. putting phone on hook
5 0
6 The phone is currently connecting
7 Select a trigger:
8 0. hung up
9 1. call connected
10 1
11 The phone is currently connected
12 Select a trigger:
13 0. left message
14 1. hung up
15 2. placed on hold
16 2
17 The phone is currently on hold
18 Select a trigger:
19 0. taken off hold
20 1. hung up
21 1
22 The phone is currently off the hook
23 Select a trigger:
24 0. call dialed
25 1. putting phone on hook
26 1
27 We are done using the phone
```

这种手工状态机的主要优点是非常容易理解:状态和转换是普通的枚举类，转换集是在一个简单的`std::map`中定义的，开始和结束状态是简单的变量


#### Boost.MSM 中的状态机

在现实世界中，状态机要复杂得多。有时，你希望在达到某个状态时发生某些操作。在其他时候，你希望转换是有条件的，也就是说，你希望转换只在某些条件存在时发生。

当`Boost.MSM (Meta State Machine)`，一个状态机库，是Boost的一部分，你的状态机是一个通过`CRTP`继承自`state_ machine_def`的类：

```c++
struct PhoneStateMachine : state_machine_def<PhoneStateMachine>
{
    bool angry{ false };
}
```

我添加了一个`bool`变量来指示调用者是否`angry`(例如，在被搁置时); 我们稍后会用到它。现在，每个状态也可以驻留在状态机中，并且可以从`state`类继承:

```c++
struct OffHook : state<> {};
struct Connecting : state<>
{
    template<class Event, class FSM>
    void on_entry(Event const& evt, FSM&)
    {
        cout << "We are connecting..." << endl;
    }
    // also on_exit
};
// other states omitted
```

如你所见，状态还可以定义在进入或退出特定状态时发生的行为。你也可以定义在转换时执行的行为(而不是当你到达一个状态时):这些也是类，但它们不需要从任何东西继承;相反，它们需要提供具有特定签名的`operator()`:

```c++
struct PhoneBeingDestoryed
{
    template<class EVT, class FSM, class SourceState, class TargetState>
    void operator()(EVT const&, FSM& SourceState&, TargetState&)
    {
        cout << "Phone breaks into a million pieces" << endl;
    }
};
```

正如你可能已经猜到的那样，这些参数提供了对状态机的引用，以及你将要进入和进入的状态。

最后，我们有守卫条件(`guard condition`):这些条件决定我们是否可以在第一时间使用一个转换。现在，我们的布尔变量`angry`不是`MSM`可用的形式，所以我们需要包装它:

```c++
struct CanDestoryPhone
{
    template<class EVT, class FSM, class SourceState, class TargetState>
    bool operator()(EVT const&, FSM& fsm, SourceState&, TargetState&)
    {
        return fsm.angry;
    }
};
```

前面的例子创建了一个名为`CanDestroyPhone`的守卫条件，稍后我们可以在定义状态机时使用它。


为了定义状态机规则，`Boost.MSM`使用MPL(元编程库)。具体来说，转换表被定义为`mpl::vector`，每一行依次包含:

- 源状态
- 状态转换
- 目标状态
- 一个要执行的可选操作
- 一个可选守卫条件

因此，有了所有这些，我们可以像下面这样定义一些电话呼叫规则：

```c++
struct transition_table : mpl::vector<
    Row<OffHook, CallDialed, Connecting>,
    Row<Connecting, CallConnected, Connected>,
    Row<Connected, PlacedOnHold, OnHold>,
    Row<OnHold, PhoneThrownIntoWall, PhoneDestoryed, PhoneBeingDestoryed, CanDestoryPhone>
>
{};
```

在前面的方法中，与状态不同，`CallDialed`之类的转换是可以在状态机类之外定义的类。它们不必继承自任何基类，而且很容易为空，但它们必须是类型。

`transition_table`的最后一行是最有趣的:它指定我们只能尝试在`CanDestroyPhone`保护条件下销毁电话，并且当电话实际上被销毁时，应该执行`PhoneBeingDestroyed`操作。

现在，我们可以添加更多的东西。首先，我们添加起始条件:因为我们正在使用`Boost.MSM`，起始条件是一个类型定义，而不是一个变量:

```c++
typedef OffHook initial_state;
```

最后，如果没有可能的转换，我们可以定义要发生的操作。它可能发生!比如，你把手机摔坏了，就不能再用了，对吧?

```c++
template <class FSM, class Event>
void no_transition(Event const& e, FSM&, int state)
{
    cout << "No transition from state " << state_names[state]
         << " on event " << typeid(e).name() << endl;
}
```

`Boost MSM`将状态机分为前端(我们刚刚写的)和后端(运行它的部分)。使用后端API，我们可以根据前面的状态机定义构造状态机:

```c++
msm::back::state_machine<PhoneStateMachine> phone;
```

现在，假设存在`info()`函数，它只打印我们所处的状态，我们可以尝试`orchestrating`以下场景

```c++
1 info(); // The phone is currently off hook
2 phone.process_event(CallDialed{}); // We are connecting...
3 info(); // The phone is currently connecting
4 phone.process_event(CallConnected{});
5 info(); // The phone is currently connected
6 phone.process_event(PlacedOnHold{});
7 info(); // The phone is currently on hold
8 9
phone.process_event(PhoneThrownIntoWall{});
10 // Phone breaks into a million pieces
11
12 info(); // The phone is currently destroyed
13
14 phone.process_event(CallDialed{});
15 // No transition from state destroyed on event struct CallDialed
```

因此，这就是定义更复杂、具有工业强度的状态机的方式。

#### 总结

首先，这是值得强调的`Boost.MSM`是Boost中两种状态机实现之一，另一种是`Boost.statechart`。我很确定还有很多其他的状态机实现。

其次，状态机的功能远不止这些。例如，许多库支持分层状态机的思想:例如，一个`生病(Sick)`的状态可以包含许多不同的子状态，如`流感(Flu)`或`水痘(Chickenpox)`。如果你在处于感染流感的状态，你也同时处于生病的状态。

最后，有必要再次强调现代状态机与状态设计模式的原始形式之间的差异。重复api的存在(例如`LightSwitch::on/off vs. State::on/off`)以及自删除的存在在我的书中是明确的代码气味。不要误会我的方法是有效的，但它是不直观的和繁琐的。

