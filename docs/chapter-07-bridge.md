### 桥接模式

如果你一直关注c++编译器(特别是GCC、Clang和MSVC)的最新进展，你可能已经注意到编译速度正在提高。特别是，编译器变得越来越增量化，因此编译器实际上只能重新构建已更改的定义，并重用其余的定义，而不是重新构建整个翻译单元。

我之所以提到c++编译，是因为过去开发人员一直在使用一个奇怪的技巧(又是这个短语!)来优化编译速度。当然，我说的是...

#### Pimpl编程技法

让我先解释一下在Pimpl编程技法中发生的技术方面的事情。假设你决定创建一个`Person`类来存储一个人的姓名并允许他们打印问候。与通常定义Person的成员不同，您继续这样定义类

```c++
struct Person
{
    std::string name;
    void greet();
    Person();
    ~Person();
    class PersonImpl;
    PersonImpl* impl // good place for gsl::owner<T>
}

```

