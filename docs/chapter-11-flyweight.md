### 享元模式

享元(有时也称为token或cookie)是一种临时组件，相当于是对某个对象的智能引用。通常，享元适用于用于拥有大量非常相似的对象，并且希望最小化专用于存储所有这些值的内存量的情况。

让我们看一下与此模式相关的一些场景。

#### 用户名字

想象一个大型多人在线游戏。我跟你赌20美元，不止一个叫`John Smith`的用户，因为这是一个流行的名字。因此，如果我们一遍又一遍地存储这个名称(以ASCII格式)，我们将为每个这样的用户花费11个字节。相反，我们可以只存储一次名称，然后存储一个指向该名称的每个用户的指针(只有8个字节)。真是省了不少空间。

也许将名字分割成姓和名更有意义:这样，`Fitzgerald Smith`将由两个指针(16字节)表示，分别指向名和姓。事实上，如果我们使用索引而不是名称，我们可以大大减少字节数。你不会指望有2^64个不同的名字，对吗?

我们可以对它进行类型定义，以便稍后进行调整

```c++
typedef uint32_t key;
```

根据这个定义，我们可以将用户定义为:

```c++
struct User 
{
    User( const string& first_name, const string& last_name ):
        first_name{ add(first_name) },
        last_name{ add(last_name) }
    { }

    protected:
        key first_name, last_name;
        static bimap<key, string> names;
        static key seed;
        static key add (const string& s) { }
};
```

如你所见，构造函数利用`add()`函数的返回值来初始化成员`first_name`和`last_name`。该函数根据需要将键-值对(键是从`seed`中生成的)插入到`names`中。我在这里使用`boost::bimap`(一个双向映射)，因为它更容易搜索副本。记住，如果姓或名已经在bimap中，我们只返回它的索引。

下面是`add()`函数的实现：

```c++
static key add( const string& s )
{
    auto it = names.right.find( s );
    if( it == names.right.end() )
    {
        names.insert( {++seed, s} );
        return seed;
    }
    return it->second;
}
```

这是`get-or-add`机制的标准执行。如果您以前没有接触过`bimap`，您可能想要查阅`bimap`的文档，了解更多关于它如何工作的信息。

所以现在，如果我们想把姓和名给暴露出来(该成员对象位于保护段，类型是`key`，其实不是很有用!)，我们可以提供适当的`getter`和`setter`。

```c++
const string& get_first_name() const
{
    return name.left.find(first_name)->second;
}

const string& get_last_name const
{
    return name.left.find(last_name)->second;
}
```

例如，要定义用户的流输出操作符，你可以简单地编写

```c++
friend ostream& operator<<(ostream& os, const User& obj)
{
    return os << "first_name: " << obj.get_first_name()
              << " last_name: " << obj.get_last_name();
}
```

就是这样的，我不打算提供节约了多少空间的统计数据（这个真的取决于你的样本大小），但是当有大量重复的用户名的时候，这样做的确显著的节约了空间。如果你还打算进一步节约空间，可以通过改变`key`的类型定义来进一步调整`sizeof(key)`的大小。




#### Boost.Flyweight

在前面的示例中，我们动手实现了一个享元，其实我们可以直接使用Boost库中提供的`boost::flyweight`，这使得`User`类的实现变得非常简单。

```c++
struct User2
{
    flyweight<string> first_name, last_name;
    User2(const string& first_name, const string& last_name) : 
        first_name { first_name },
        last_name { last_name }
        { }
};
```
您可以通过运行以下代码来验证它实际上是享元:

```c++
User2 john_doe{ "John", "Doe" };
User2 jane_doe{ "Jane", "Doe" };
cout << boolalpha <<
(&jane_doe.last_name.get() == &john_doe.last_name.get());
// true
```

#### String Ranges

如果你调用std::string::substring()，是否会返回一个全新构造的字符串?最后的结论是:如果你想操纵它，那当然，但是如果你想改变子字符串来影响原始对象呢?一些编程语言(例如Swift、Rust)显式地将子字符串作为范围返回，这同样是Flyweight模式的实现，它节省了所使用的内存量，此外还允许我们通过范围操作底层对象。

c++中与字符串范围等价的是string_view，数组还有其他的变体——只要避免复制数据就行!让我们试着构建我们自己的，非常琐碎的弦范围。

让我们假设我们在一个类中存储了一堆文本，我们想获取该文本的一个范围并将其大写，有点像字处理器或IDE可能做的事情。我们可以只大写每个单独的字母并完成它，但是让我们假设我们想保持底层的纯文本在它的原始状态，并且只在我们使用流输出操作符时大写。

#### 简单方法

一种非常简单明了的方法是定义一个布尔数组，它的大小与纯文本字符串匹配，值指示是否要大写该字符。我们可以这样实现它:

```c++
class FormattedText
{
    string plainText;
    bool *cap;
    public:
        explicit  FormattedText( const string& plainText ) : plainText { plainText } 
        {
            caps = new bool [ plainText.length() ];
        }

        ~FormattedText( )
        {
            delete [ ] caps;
        }
};
```
我们现在可以用一个实用方法来大写一个特定的范围:

```c++
void capitalize(int start, int end)
{
    for( int i = start, i <= end; ++i )
        caps[ i ] = true;
}
```
然后定义一个使用布尔掩码的流输出操作符:

```c++
friend std::ostream& operator << (std::ostrem& os, const Formatted& obj)
{
    string s;
    for( int i = 0; i < obj.plainText.length(); ++i)
    {
        char c = obj.plainText[ i ];
        s += ( obj.cap[ i ] ? toupper( c ) : c );
    }
    return os << s;
}
```

不要误解我的意思，这种方法是有效的。在这里:

```c++
FormattedText ft("This is a brave new world");
ft.capitalize(10, 15);
cout << ft << endl;
// prints "This is a BRAVE new world"
```

但是，再一次地，将每个角色定义为拥有一个
布尔标记，当只使用开始和结束标记时。

#### 享元实现



#### 总结

享元模式是一种节约空间的技术。它存在许多确切的变体：
有时，享元作为API `token`返回，允许你执行修改；而在其他时候，享元是隐式的，隐藏在场景后面—就像我们的`User`一样，客户端并不打算知道实际使用的享元。