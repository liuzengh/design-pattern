### 迭代器

在开始处理复杂的数据结构时，都会遇到 *遍历(traversal)* 的问题。这可以用不同的方法来处理，但最常见的遍历方法，比如，`vector`，是使用一种称为 *迭代器(iterator)* 的东西。

简单地说，迭代器是一个对象，它可以指向集合中的一个元素，也知道如何移动到集合中的下一个元素。因此，只需要实现`++`操作符和`!=`操作符(这样就可以比较两个迭代器并检查它们是否指向相同的东西)。

c++标准库大量使用迭代器，因此我们将讨论它们的使用方式，然后我们将看看如何制作我们自己的迭代器以及迭代器的局限。

#### 标准库中的迭代器

假设你有一个名字列表，例如:

```c++
vector<string> names {"john","jane", "jill", "jack"}
```

如果想要获得`names`中的第一个名字，可以调用`begin()`函数。这个函数不会返回第一个名字的值或引用给你; 相反，它会返回一个迭代器给你:

```c++
vector<string>::iterator it = names.begin();
```

`begin()`既是`vector`的成员函数，也是全局函数。全局的`begin()`对于不能包含成员函数`c`语言风格的数组（而不是`std::array`)）特别有用。

你可以把`begin()`返回一个的迭代器看作指针: 对于`vector`来说，功能上是相似的。例如，可以对迭代器进行提领操作(*dereference*)来打印实际的名称:

```c++
cout << "first name is " << *it << "\n";
// first name is john
```

迭代器`it`是可以 *前进(advance)* 的，即移动到下一个元素。迭代器上的自增操作`++`强调的是向前移动的概念，也就是说，和指针向前移动的`++`操作(即递增内存地址)是不相同的。

```c++
++it; // now points to jane
```

也可以使用迭代器(像指针一样)修改它所指向的元素:

```c++
it->append(" goodall"s);
cout << "full name is " << *it << "\n";
// full name is jane goodall
```

`begin()`函数对应的当然是`end()`函数，但它不是指向最后一个元素，而是指向最后一个元素之后的元素：

```c++
        1 2 3 4
begin() ^       ^ end()
```

可以使用`end()`作为终止条件。例如，让我们使用`it`来打印列表中其余的名称：

```c++
while (++it != names.end())
{
    cout << "another name: " << *it << "\n";
}
// another name: jill
// another name: jack
```

除了`begin()`和`end()`之外，还有`rbegin()`和`rend()`，它们允许我们在集合中反向移动。在本例中，你可能已经猜到，`rbegin()`指向最后一个元素，而`rend()`指向第一个之前的一个元素:

```c++
for (auto ri = rbegin(names); ri != rend(names); ++ri)
{
    cout << *ri;
    if (ri + 1 != rend(names)) // iterator arithmetic
    cout << ", ";
}
cout << endl;
// jack, jill, jane goodall, john
```

上面的代码有两点需要注意。首先，即使向后遍历`vector`，我们仍然在迭代器上使用`++`操作符。其次，我们可以对`it`做算术操作: `ri + 1`指向的是`ri`前一个元素，而不是后一个元素。

`STL`中也提供不允许修改对象的常量迭代器:它们通过`cbegin()/cend()`返回，与之对应的是`crbegin()/crend()`：

```c++
vector<string>::const_reverse_iterator jack = crbegin(names);
// won't work
*jack += "reacher";
```

最后，值得一提的是, 现代c++里面*基于范围的`for`循环(range based for loop)*，从容器的`begin()`一直迭代到`end()`。

```c++
for (auto& name : names)
    cout << "name = " << name << "\n";
```

注意迭代器在这里是自动提领的: 变量名`name`是一个引用，但也可以按值进行迭代。

### 遍历二叉树

让我们回顾一下数据结构里面遍历二叉树的练习。首先，我们定义树中的的节点：

```c++
template<typename T>
struct Node 
{
    T value;
    Node<T>* left;
    Node<T>* right;
    Node<T>* parent;
    BinaryTree<T>* tree;
};
```
每个节点都有指向其左孩子结点、右孩子结点，父结点(如果有的话)以及整个树的指针。可以单独构造一个叶节点，也可以使用其子节点来构造内部结点。

```c++
 explicit Node(const T& value) 
        : value(value)
        , left(nullptr)
        , right(nullptr)
        , parent(nullptr)
        , tree(nullptr) 
    { }

Node(const T& value, Node<T>* left, Node<T>* right)
    : value(value)
    , left(left)
    , right(right)
    , parent(nullptr)
    , tree(nullptr) 
    {
        this->left->tree = this->right->tree = tree;
        this->left->parent = this->right->parent = this;
    }
};
```

最后，我们引入一个通用的成员函数来设置树指针。这是通过在所有节点的子节点上递归完成的:

```c++
void set_tree(BinaryTree<T>* t)
{
    tree = t;
    if(left) 
        left->set_tree(t);
    if(right)
        right->set_tree(t);
}
```

有了这些，我们现在可以定义一个称为`BinaryTree`的结构，它正是这个结构允许迭代:

```c++
template <typename T> 
struct BinaryTree
{
    Node<T>* root = nullptr;
    explicit BinaryTree(Node<T>* const root)
        : root{ root }
    {
        root->set_tree(this);
    }
};
```

现在我们可以为树定义一个迭代器。迭代二叉树有三种常见的方法，我们首先要实现的是前序遍历`preorder`:

- 一旦遇到该元素，就返回该元素。
- 递归地遍历左子树
- 递归地遍历右子树

让我们从一个构造函数开始:

```c++
template <typename U>
struct PreOrderIterator
{
    Node<U>* current;
    explicit PreOrderIterator(Node<U>* current)
        : current(current)
    {

    }
    // 其他成员
};
```

需要定义`operator != `来与其他迭代器进行比较。因为迭代器的相当于指针，所以这很简单:

```c++
bool operator!=(const PreOrderIterator<U>& other)
{
    return this->current != other.current;
}
```

我们需要定义`*`操作符来实现提领:

```c++
Node<U>& operator*() { return *current; }
```


现在，最后一个问题是如何从我们的二叉树中把迭代器给暴露出来。如果将前序遍历其定义为树的默认迭代器，则可以如下所示补充成员函数：

```c++
typedef PreOrderIterator<T> iterator;

iterator begin()
{
    Node<T>* n = root;
    if(n)
        while(n->left)
            n = n->left;
    return iterator { n };
}

iterator end()
{
    return iterator {nullptr};
}
```

现在，困难的部分来了:遍历树。这里的挑战是我们使用递归算法，遍历发生在`++`操作符中，所以我们最终实现如下所示:

```c++
PreOrderIterator<U>& operator++()
{
    if(current->right)
    {
        current = current->right;
        while(current->left)
            current = current->left;
    }
    else
    {
        Node<T>* p = current->parent;
        while(p && current == p->right)
        {
            current = p;
            p = p->parent;
        }
        current = p;
    }
    return *this;
}
```

这太乱了!而且，它看起来一点也不像树遍历的经典实现，因为我们没有递归。

值得注意的是，`begin()`迭代器并不是从整个树的根开始; 相反，它从最左边可用的节点开始。

现在所有的部分都准备好了，下面是我们如何执行遍历:

```c++
BinaryTree<string> family{
    new Node<string>{
        "me", 
        new Node<string>{
            "mother",
            new Node<string>{"mother's mother"},
            new Node<string>{"mother's father"}},
        new Node<string>{"father"}
    }
};

for (auto it = family.begin(); it != family.end(); ++it)
{
    cout << (*it).value << "\n";
}
```

您也可以将这种遍历形式暴露为一个单独的对象，即:

```c++
class pre_order_traversal
{
    BinaryTree<T>& tree;
    public:
        pre_order_traversal(BinaryTree<T>& tree) : tree{ tree } {}
        iterator begin() { return tree.begin(); }
        iterator end() { return tree.end(); }
} pre_order;
```

可以这样来遍历：

```c++
for (const auto& it: family.pre_order)
{
    cout << it.value << "\n";
}
```

类似地，可以定义`in_order`和`post_order`遍历算法来暴露合适的迭代器。

### 迭代与协程

我们遇到一个严重的问题:在遍历代码中，`operator++`难以读懂，它与你在`Wikipedia`上读到的关于树遍历的任何内容都不匹配。它能起作用，但它之所以能起作用，只是因为我们将迭代器初始化为最左边的节点，而不是从根节点开始，这样做是有问题和令人困惑的。


为什么我们会有这个问题?因为`++`操作符是不可恢复的:它不能在两次调用之间保持其堆栈，因此，不能实现递归。现在，我们是否有一种两全其美的方法：可执行正确递归的可恢复函数? 我们可以用`协程(coroutines)`来实现。

利用协程，我们可以像这样实现后序遍历:

```c++
generator<<Node<T>*> post_order_impl(Node<T>* node)
{
    if(node)
    {
        for(auto x : post_order_impl(node->left))
            co_yield x;
        for(auto y : post_order_impl(node->right))
            co_yield y;
        co_yield node;
    }
}
generator<Node<T>*> post_order()
{
    return post_order_impl(root);
}
```

这不是很棒吗?算法终于又可读了!而且，看不到`begin()/end()`: 我们只是返回一个 *生成器(generator)* ，逐步返回`co_yield`提供生成的值。在生成每个值之后，我们可以挂起当前操作并执行其他操作(例如，打印值)，然后在不丢失上下文的情况下恢复迭代。这使的递归成为可能并允许我们写出如下的代码:

```c++
for(auto it : family.post_order())
{
    cout << it->value << endl;
}
```

协程是c++的未来，它解决了许多传统迭代器丑陋或不合适的问题。


### 总结

迭代器设计模式在c++中无处不在，有显式的也有隐式的(例如基于范围的)形式。不同类型的迭代器可以用于迭代不同的对象:例如，反向迭代器可以应用于`vector`，但不能应用于单链表。

实现自己的迭代器就像提供`++`和`!=`操作符一样简单。大多数迭代器都是简单的指针的外观(`façades`)，在它们被丢弃之前，用于遍历集合一次。

协程修复了迭代器中出现的一些问题:它们允许在调用之间保持状态，这是其他语言(如`c#`)很久以前就实现了的。因此，协程允许我们编写递归算法。


