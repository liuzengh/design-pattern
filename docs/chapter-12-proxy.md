### 代理

当我们关注装饰器设计模式时，我们看到了增强对象功能的不同方式。代理设计模式与此类似，但其目标通常是在提供某些内部增强功能的同时准确（或尽可能接近）保留正在使用的`API`。

`Proxy` 并不是真正的同质 `API`，因为人们构建的不同种类的代理数量相当多并且服务完全不同
目的。在本章中，我们将介绍一系列不同的代理对象，你可以在网上找到更多信息。


#### 智能指针

代理模式最简单、最直接的例子是智能指针。智能指针对普通指针进行了包装，同时保留引用计数，重写了某些操作符，但总的来说，它提供和普通指针一样的使用接口。

```c++
struct BankAccount {
  void deposit(int amount) { ... }
};

BankAccount *ba = new BankAccount;
ba->deposit(123);
auto ba2 = make_shared<BankAccount>();
ba2->deposit(123);  // 相同的 API!
```

因此，智能指针在某些地方可以用来替代普通指针。例如，`if (ba){...}`无论`ba`是普通指针还是智能指针，`*ba`在这两种情况下可以获得底层对象。

当然，差异是存在的。最明显的一个是你不需要在智能指针上调用delete。但除此之外，它真的试图尽可能地接近一个普通的指针。



#### 属性代理

在其他编程语言中，属性用于指示字段和该字段的一组`getter/setter`方法。在c++[^1]中没有属性，但是如果我们想继续使用一个字段，同时给它特定的访问/修改(`accessor/mutator`)行为，我们可以构建一个*属性代理*

本质上，属性代理是一个可以伪装成属性的类，所以我们可以这样定义它:

```c++
template <typename T>
struct Property {
  T value;
  Property(const T initial_value) { *this = initial_value; }
  operator T() {
    return value;
    // 执行一些getter操作
  }
  T operator=(T new_value) {
    return value = new_value;
    // 执行一些setter操作
  }
};
```

在前面的实现中，我已经向通常要自定义(或直接替换)的位置添加了注释，如果要采用这种方法，注释大致对应于`getter /setter`的位置。

因此，我们的类`Property<T>`本质上是`T`的替换，不管它是什么。它的工作原理是简单地允许与T的转换，并允许两者都使用value字段。现在，你可以把它用作一个字段。

```c++
struct Creature {
  Property<int> strength{10};
  Property<int> agility{5};
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
struct Image {
  virtual void draw() = 0;
};
```

`Bitmap`的即时(与惰性相反)实现将在构建时从文件中加载图像，即使该图像实际上并不需要。(是的，下面的代码是模拟的。)

```c++
struct Bitmap : Image {
  Bitmap(const string &filename) {
    cout << "Loading image from " << filename << endl;
  }
  void draw() override { cout << "Drawing image " << filename << endl; }
};
```

构造这个位图的行为将触发图像的加载:

```c++
Bitmap img{ "pokemon.png" };  // 从pokemon.png加载图像
```

那不是我们想要的。我们想要的是那种只在使用draw()方法时加载自身的位图。现在，我想我们可以跳回到位图，让它变得懒惰，但假设它是固定的，不能修改(或继承，就此而言)。

因此，我们可以构建一个虚拟代理，它将聚合原来的位图，提供一个相同的接口，并重用原来的位图功能：

```c++
struct LazyBitmap : Image {
  LazyBitmap(const string filename) : filename(filename) {}
  ~LazyBitmap() { delete bmp; }  // 如果*bmp没有先被new出来，这里就会出错

  void draw() override {
    if (!bmp) bmp = new Bitmap(filename);
    bmp->draw();
  }

 private:
  Bitmap *bmp{nullptr};
  string filename;
};
```

我们到了。正如您所看到的，这个`LazyBitmap`的构造函数要轻得多:它所做的只是存储用于加载图像的文件名，这样图像实际上就不会被加载。

所有神奇的事情都发生在`draw()`中:这是我们检查`bmp`指针的地方，以查看底层(`eager`!)位图是否已经构造好了。如果没有，我们就构造它，然后调用它的`draw()`函数来实际绘制图像。

现在假设您有一些使用`Image`类型的API:

```c++
void draw_image(Image& img) {
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

#### 通信代理

Suppose you call a member function foo() on an object of type Bar. Your typical assumption is that Bar has been allocated on the same machine as the one running your code, and you similarly expect Bar::foo() to execute in the same process.

Now imagine that you make a design decision to move Bar and all its members off to a different machine on the network. But you still want the old code to work! If you want to keep going as before, you’ll need a communication proxy—a component that proxies the calls “over the wire” and of course collects results, if necessary.

Let’s implement a simple ping-pong service to illustrate this. First, we define an interface:


假设你在 `Bar` 类型的对象上调用成员函数 `foo()`。你的典型假设是 `Bar` 与运行你的代码的机器分配在同一台机器上，并且你同样希望与`Bar::foo()` 在同一进程中执行。

现在假设你做出了一个设计决定，将 `Bar` 及其所有成员移到网络上的另一台机器上。但是你仍然希望旧代码能够工作！如果你想像以前一样继续，你需要一个通信代理——一个代理“通过线路”的调用的组件，当然如果需要的话收集结果。

让我们实现一个简单的乒乓服务（ping-pong service）来说明这一点。首先，我们定义一个接口：

```c++
struct Pingable {
  virtual wstring ping(const wstring& message) = 0;
};
```

如果我们正在构建乒乓进程，我们可以将 `Pong` 实现为
如下：

```c++
struct Pong : Pingable {
  wstring ping(const wstring& message) override { return message + L" pong"; }
};
```

基本上，你 ping 一个 `Pong`，它会将单词 `“ pong”` 附加到消息的末尾并返回该消息。请注意，我在这里没有使用 `ostringstream&`，而是在每次都时创建一个新字符串：这个 `API` 很容易复制为 `Web` 服务。

我们现在可以尝试这个设置，看看它是如何在进程中工作的：

```c++
void tryit(Pingable& pp) { wcout << pp.ping(L"ping") << "\n"; }
Pong pp;
for (int i = 0; i < 3; ++i) {
  tryit(pp);
}
```
最终的结果是上述代码按照我们想要的方式打印了 3 次 “ping pong”。
现在，假设你决定将 `Pingable` 服务重新定位到很远很远的 `Web` 服务器。也许你甚至决定使用其他平台，例如 `ASP.NET`，而不是 `C++`：

```c#
[Route("api/[controller]")] 
public class PingPongController : Controller {
  [HttpGet("{msg}")] 
  public string Get(string msg) { return msg + " pong"; }
}  // achievement unlocked: use C# in a C++ book
```

通过此设置，我们将构建一个名为 `RemotePong` 的通信代理
这将用于代替 `Pong`。微软的 `REST SDK` 在这里派上了用场。

```c#
struct RemotePong : Pingable {
  wstring ping(const wstring& message) override {
    wstring result;
    http_client client(U("http://localhost:9149/"));
    uri_builder builder(U("/api/pingpong/"));
    builder.append(message);
    pplx::task<wstring> task = client.request(methods::GET, builder.to_string())
                                   .then([=](http_response r) {
                                     return r.extract_string();
                                   });
    task.wait();
    return task.get();
  }
};
```

注1: Microsoft REST SDK 是一个用于处理 REST 服务的 C++ 库。它既是开源的又是跨平台的。你可以在 GitHub 上找到它：https:/ github.com/Microsoft/cpprestsdk.

如果你不习惯 `REST SDK`，前面的内容可能看起来有点令人困惑；除了 `REST` 支持之外，`SDK` 还使用了并发运行时，这是一个 `Microsoft` 库，用于并发支持。实现此功能后，我们现在可以进行一个更改：

```c++
RemotePong pp;  // was Pong
for (int i = 0; i < 3; ++i) {
  tryit(pp);
}
```

就是这样，你得到相同的输出，但实际的实现可以在地球另一端某个地方的 `Docker` 容器中的 `Kestrel` 上运行。

#### 总结

本章介绍了一些代理。与装饰器模式不同，代理不会尝试通过添加新成员来扩展对象的功能（除非它无能为力）。它试图做的只是增强现有成员的潜在行为。

存在大量不同的代理：

- 属性代理是替代对象，可以在分配和/或访问期间替换字段并执行附加操作。
- 虚拟代理提供对底层对象的虚拟访问，并且可以实现延迟对象加载等行为。你可能觉得你正在处理一个真实的对象，但底层实现可能尚未创建，例如，可以按需加载。
- 通信代理允许我们更改对象的物理位置（例如，将其移动到云端），但允许我们使用几乎相同的 `API`。当然，在这种情况下，`API` 只是远程服务（如 REST API）的一个垫片（`shim`）。
- 日志代理除了调用底层函数之外，还可以执行日志记录。

注2：在程序设计领域，垫片（shim）是一种小型函数库，可以用来截取 API 调用、修改传入参数，最后自行处理对应操作或者将操作交由其它地方执行。垫片可以在新环境中支持老 API，也可以在老环境里支持新 API。一些程序并没有针对某些平台开发，也可以通过使用垫片来辅助运行。

还存在很多其他代理，而且你自己构建的代理很可能不会属于预先存在的类别，而是会执行一些特定于你专业领域内的操作。