### 中介者模式

我们编写的大部分代码都有不同的组件(类)通过直接引用或指针相互通信。然而，在某些情况下，你并不希望对象必须意识到对方的存在。或者，也许我们确实希望它们能够相互了解，但我们仍然不希望它们通过指针或引用进行通信，因为它们可能会过时，而我们又不想解引用一个nullptr。

因此，中介模式是一种促进组件之间通信的机制。当然，中介本身需要被参与其中的每个组件访问，这意味着它应该是一个全局静态变量，或者只是一个注入到每个组件中的引用。

#### 聊天室

因特网上的聊天室是中介模式的一个典型例子，我们先讨论对它的实现。一个最简单的实现如下：

```c++
struct Person {
  string name;
  ChatRoom* room = nullptr;
  vector<string> chat_log;
  Person(const string& name);
  void receive(const string& origin, const string& message);
  void say(const string& message) const;
  void pm(const string& who, const string& message) const;
};
```

我们得到了一个有姓名(用户id)、聊天日志和指向实际聊天室的指针的人。我们有一个构造函数和三个成员函数:

- receive() 用于接收信息。通常，该函数将在用户屏幕上显示消息，并将其添加到聊天日志中。
- say()用于向房间里的每个人广播信息。
- pm()用于传递私人消息，需要指定消息要发送的人员的名称。

让我们实际地实现一下聊天室，它并不是特别复杂：
```c++
struct ChatRoom {
  vector<Person*> people;  // assume append-only
  void join(Person* p);
  void broadcast(const string& origin, const string& message);
  void message(const string& origin, const string& who, const string& message);
};
```
究竟是使用指针、引用还是shared_ptr来实际存储聊天室用户列表，最终取决于我们自己：惟一的限制是std::vector不能存储引用。所以，我决定在这里使用指针。聊天室的API非常简单:

- join()让一个人加入房间。我们暂时不打算实现leave()，而是将这个想法推迟到本章的后续示例中

- broadcast()将消息发送给所有人(除了发消息的人自身)。

- message()发送一个私有消息。

join()的实现如下：

```c++
void ChatRoom::join(Person* p) {
  string join_msg = p->name + " joins the chat";
  broadcast("room", join_msg);
  p->room = this;
  people.push_back(p);
}
```

就像一个经典的IRC聊天室一样，我们向房间里的每个人广播有人加入的消息。然后，我们设置人的房间指针，并将它们添加到房间中的人员列表中。现在，让我们看看broadcast():这是向每个房间参与者发送消息的地方。记住，每个参与者都有自己的Person::receive()函数来处理消息，所以实现有点琐碎:
```c++
void ChatRoom::broadcast(const string& origin, const string& message) {
  for (auto p : people)
    if (p->name != origin) p->receive(origin, message);
}
```

我们是否想要阻止广播信息向我们自己传播是一个值得讨论的问题。最后，下面是使用 `message()` 实现的私有消息传递:

```c++
void ChatRoom::message(const string& origin, const string& who,
                       const string& message) {
  auto target = find_if(begin(people), end(people),
                        [&](const Person* p) { return p->name == who; });
  if (target != end(people)) {
    (*target)->receive(origin, message);
  }
}
```

这将在人员列表中搜索收件人，如果找到了收件人(因为谁知道，他们可能已经离开房间了)，就将消息发送给那个人。回到Person的say()和pm()实现:

```c++
void Person::say(const string& message) const {
  room->broadcast(name, message);
}

void Person::pm(const string& who, const string& message) const {
  room->message(name, who, message);
}
```

至于receive()，这是在屏幕上实际显示消息并将其添加到聊天日志的好地方。

```c++
void Person::receive(const string& origin, const string& message) {
  string s{origin + ": \"" + message + "\""};
  cout << "[" << name << "'s chat session] " << s << "\n";
  chat_log.emplace_back(s);
}
```
我们在这里做了更多的工作，不仅显示消息来自谁，还显示我们目前所在的聊天会话——这对于诊断谁在什么时候说了什么很有用。

```c++
ChatRoom room;
Person john{"john"};
Person jane{"jane"};
room.join(&john);
room.join(&jane);
john.say("hi room");
jane.say("oh, hey john");

Person simon("simon");
room.join(&simon);
simon.say("hi everyone!");

jane.pm("simon", "glad you could join us, simon");
```

#### 中介与事件

在聊天室的例子中，我们遇到了一个一致的主题:每当有人发布消息时，参与者都需要通知。这是20章中讨论的Observer模式的完美场景:中介者模式拥有一个由所有参与者共享的事件;然后，参与者可以订阅事件来接收通知，他们还可以触发事件，从而触发通知。

事件并没有内置到c++中(与c#不同)，所以我们将在这个演示中使用一个库解决方案。Boost.Signals2为我们提供了必要的功能。

让我们举个简单的例子:想象一款有球员和足球教练的足球游戏。当教练看到他们的球队得分时，他们自然想要祝贺球员。当然，他们需要一些关于该事件的信息，比如谁进球了，到目前为止他们已经进了多少球。我们可以为任何类型的事件数据引入基类：

```c++
struct EventData {
  virtual ~EventData() = default;
  virtual void print() const = 0;
};
```

我们添加了print()函数，这样每个事件都可以打印到命令行，还添加了一个虚拟析构函数以使ReSharper停止处理它。现在，我们可以从这个类派生来存储一些与目标相关的数据:

```c++
struct PlayerScoredData : EventData {
  string player_name;
  int goals_scored_so_far;
  PlayerScoredData(const string& player_name, const int goals_scored_so_far)
      : player_name(player_name), goals_scored_so_far(goals_scored_so_far) {}

  void print() const override {
    cout << player_name << " has scored! (their " << goals_scored_so_far
         << " goal)"
         << "\n";
  }
};
```

我们将再次构建一个中介模式，不过，当我们有了事件驱动的基础设施，它们就不再需要了:

```c++
struct Game {
  signal<void(EventData*)> events;  // observer
};
```

事实上，你可以只使用全局信号而不需要一个Game类，但我们在这里使用的是最小惊奇原则，如果一个Game&被注入到一个组件中，我们知道这里有一个明显的依赖性。

不管怎样，我们现在可以构造玩家类了。当然，球员有自己的名字、在比赛中进球的次数，还有一段关于调停比赛的参考:

```c++
struct Player {
  string name;
  int goals_scored = 0;
  Game& game;
  Player(const string& name, Game& game) : name(name), game(game) {}

  void score() {
    goals_scored++;
    PlayerScoredData ps{name, goals_scored};
    game.events(&ps);
  }
};
```

这里的Player::score()是一个有趣的函数:它使用事件信号创建一个PlayerScoredData，并将其发布给所有订阅者。谁得到这个事件?当然是一名教练了。

```c++
struct Coach {
  Game& game;
  explicit Coach(Game& game) : game(game) {
    // celebrate if player has scored <3 goals
    game.events.connect([](EventData* e) {
      PlayerScoredData* ps = dynamic_cast<PlayerScoredData*>(e);
      if (ps && ps->goals_scored_so_far < 3) {
        cout << "coach says: well done, " << ps->player_name << "\n";
      }
    });
  }
};
```

Coach类的实现是微不足道的;我们的教练连名字都不知道。但我们确实给了他一个构造函数，用于创建游戏订阅。事件，这样，无论何时发生什么事情，coach都可以在提供的lambda (slot)中处理事件数据。

注意lambda的参数类型是EventData*——我们不知道一个球员是否已经得分或已经被发送，所以我们需要dynamic_cast(或类似的机制)来确定我们得到了正确的类型。

有趣的是，所有神奇的事情都发生在设置阶段:不需要明确地为特定信号征募插槽。客户端可以自由地使用它们的构造函数创建对象，然后，当玩家得分时，通知被发送:

```c++
Game game;
Player player{ "Sam", game };
Coach coach{ game };
player.score();
player.score();
player.score(); // ignored by coach
```

这将产生以下输出:

```c++
coach says: well done, Sam
coach says: well done, Sam
```

输出只有两行长，因为在第三个目标上，教练不再印象深刻。

#### 总结

中介者设计模式本质上是提议引入一个中间组件，系统中的每个人都可以引用该组件并可以使用它来相互通信。可以通过标识符（用户名、唯一 ID 等）代替直接内存地址进行通信。

中介者的最简单实现是一个成员列表和一个函数，它遍历列表并执行预期的操作——无论是列表中的每个元素还是有选择的。


中介者的一个更复杂的实现可以使用事件来允许参与者订阅（和取消订阅）系统中发生的事情。这样，从一个组件发送到另一个组件的消息可以被视为事件。在这种设置中，如果参与者不再对某些事件感兴趣或即将完全离开系统，他们也很容易取消订阅某些事件。