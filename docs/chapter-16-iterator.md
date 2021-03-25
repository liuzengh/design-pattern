### 迭代器

在开始处理复杂的数据结构时，都会遇到*遍历(traversal)*的问题。这可以用不同的方法来处理，但最常见的遍历方法，比如，vector，是使用一种称为*迭代器(iterator)*的东西。

简单地说，迭代器是一个对象，它可以指向集合中的一个元素，也知道如何移动到集合中的下一个元素。因此，只需要实现`++`操作符和`!=`操作符(这样就可以比较两个迭代器并检查它们是否指向相同的东西)。

c++标准库大量使用迭代器，因此我们将讨论它们的使用方式，然后我们将看看如何制作我们自己的迭代器以及迭代器的限制是什么。

#### 标准库中的迭代器

假设你有一个名字列表，例如:

```c++
vector<string> names {"john","jane", "jill", "jack"}
```

如果希望获得`names`集合中的第一个名字，则需要调用一个名为`begin()`的函数。这个函数不会按值或引用给你名字;相反，它给您一个迭代器:

```c++
vector<string>::iterator it = names.begin();
```

函数begin()既是vector的成员函数，也是全局函数。全局函数对于数组(c风格数组，而不是std::array)特别有用，因为它们不能包含成员函数。

因此`begin()`返回一个可以视为指针的迭代器:对于`vector`，它具有类似的机制。例如，可以对迭代器进行解引用以打印实际的名称:

```c++
cout << "first name is " << *it << "\n";
// first name is john
```

给定的迭代器知道如何前进，即移动到下一个元素。重要的是要意识到`++`指的是向前移动的概念，也就是说，对于指针向前移动(即递增内存地址)，它与++是不同的。

```c++
++it; // now points to jane
```

也可以使用迭代器(与指针一样)修改它所指向的元素:

```c++
it->append(" goodall"s);
cout << "full name is " << *it << "\n";
// full name is jane goodall
```

现在，与`begin()`对应的当然是`end()`，但它不是指向最后一个元素，而是指向最后一个元素之后的元素。这是一个拙劣的例证

```c++
        1 2 3 4
begin() ^       ^ end()
```

可以使用`end()`作为终止条件。例如，让我们使用`it`迭代器变量打印其余的名称

```c++
while (++it != names.end())
{
    cout << "another name: " << *it << "\n";
}
// another name: jill
// another name: jack
```

除了`begin()`和`end()`之外，还有`rbegin()`和`rend()`，它们允许我们在集合中向后移动。在本例中，正如您可能已经猜到的，`rbegin()`指向最后一个元素，而`rend()`指向第一个之前的一个元素:

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

前面有两件事值得指出。首先，即使向后遍历`vector`，我们仍然在迭代器上使用`++`操作符。其次，我们可以做算术:同样，当我写入`ri + 1`时，`this`指向`ri`前面的元素，而不是后面的元素。

我们也可以有不允许修改对象的常量迭代器:它们通过`cbegin()/cend()`返回，当然，也有相反的类型`crbegin()/crend()`：

```c++
vector<string>::const_reverse_iterator jack = crbegin(names);
// won't work
*jack += "reacher";
```

最后，值得一提的是现代c++结构，它是一个基于范围的`for`循环，用于从容器的`begin()`一直迭代到`end()`

```c++
for (auto& name : names)
    cout << "name = " << name << "\n";
```

注意迭代器在这里是自动解引用的:变量名是一个引用，但也可以按值进行迭代。

### 遍历二叉树

```c++
template<typename T>
struct Node 
{
    T value;
    Node<T>* left;
    Node<T>* right;
    Node<T>* parent;
    BinaryTree<T>* tree;
    explict Node(T value) : 
        value(value), 
        left(nullptr), 
        right(nullptr),
        parent(nullptr),
        tree(nullptr) 
    {
        
    };
    Node(T value, Node<T>* left, Node<T>* right):
        value(value), 
        left(left), 
        right(right),
        parent(nullptr),
        tree(nullptr) 
    {
        this->left->tree = this->right->tree = tree;
        this->left->parent = this->right->parent = this;
    }
};
```
最后，我们引入一个实用程序成员函数来设置树指针。这是在所有节点的子节点上递归完成的:
```c++

```


### 迭代与协程

我们有一个严重的问题:在遍历代码中，operator++是一团难以读懂的混乱，它与你在Wikipedia上读到的关于树遍历的任何内容都不匹配。它能起作用，但它之所以能起作用，只是因为我们将迭代器预初始化为从最左边的节点开始，而不是从根节点开始，也可以说，这也是有问题和令人困惑的。

因此，递归是不可能的。现在，如果我们有一种既能吃蛋糕又能吃蛋糕的机制:可以执行正确递归的可恢复函数?这就是`协程(coroutines)`的作用。



### 总结

迭代器设计模式在c++中无处不在，有显式的也有隐式的(例如基于范围的)形式。不同类型的迭代器可以用于迭代不同的对象:例如，反向迭代器可以应用于vector对象，但不能应用于单链表。

实现自己的迭代器就像提供++和!=操作符一样简单。大多数迭代器都是简单的指针`façades`，在它们被丢弃之前，用于遍历集合一次。

协程修复了迭代器中出现的一些问题:它们允许在调用之间保持状态，这是其他语言(如c#)很久以前就实现了的。因此，协程允许我们编写递归算法。

通过协程，我们可以实现后序树遍历，如下所示:



协程是c++的未来，它解决了许多传统迭代器丑陋或不合适的问题。