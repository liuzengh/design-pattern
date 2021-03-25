### 策略模式

假设您决定使用包含多个字符串的数组或向量，并将它们作为列表输出 `["just", "like", "this"]`。

如果考虑不同的输出格式，您可能知道需要获取每个元素，并将其与一些额外的标记一起输出。但对于HTML或LaTeX这样的语言，列表还需要开始和结束标记或标记。这两种格式中的任何一种对列表的处理都是相似的(需要输出每个条目)和不同的(条目的输出方式)。这些都可以用一个单独的策略来处理。

我们可以制定一个渲染列表的策略:

- 渲染开始标签或元素
- 渲染每一个list元素
- 渲染关闭标签或元素

可以针对不同的输出格式制定不同的策略，然后将这些策略输入一个通用的、不变的算法来生成文本。这是另一种存在于动态(运行时可替换)和静态(由模板组成的、固定的)类型中的模式。让我们一起来探讨下它们。

#### 动态策略

因此，我们的目标是打印一个markdown或html格式的简单的文本项列表：

```c++
enum class OutputFormat 
{
    markdown,
    html
};
```

我们可以在基类ListStrategy中定义打印策略框架：

```c++
struct ListStrategy 
{
    virtual void start(ostringstream& oss) { };
    virtual void end(ostringstream& oss) { };
    virtual void add_list_item(ostringstream& oss, const string& item) {};
};
```

现在让我们跳到文本处理组件。这个组件将调用打印列表的成员函数， `append_list()`

```c++
struct TextProcessor 
{
    void append_list(const verctor<string> items)
    {
        listStrategy->start(oss);

        for(auto &&item : items)
            listStrategy->add_list_item(oss, item)

        listStrategy->end(oss);
    }
    private:
        ostringstream oss;
        unique_ptr<ListStrategy> listStrategy;
};
```

在`TextProcessor`中我们定义了一个oss的输出字符串缓冲区，我们正在使用的策略是渲染列表，当然还有`append_list()`，它指定了为了实际渲染一个给定的策略列表，需要采取的一系列步骤。需要注意的是，正如前面所使用的，组合是两种可能的选项之一，可用于实现框架算法的具体实现。相反，我们可以添加像`add_list_item()`这样的函数作为虚成员，由派生类覆盖:这就是模板方法模式。


现在我们可以继续为列表实现不同的打印策略，比如`HtmlListStrategy`

```c++
struct HtmlListStrategy : ListStrategy 
{
    void start(ostringstream& oss) override
    {
        oss << "<ul>\n";
    }

    void end(ostringstream& oss) override
    {
        oss << "</ul>\n";
    }

    void add_list_item(ostringstream& oss, const string& item) override
    {
        oss << "<li>" << item << "</li> \n"
    }


}; 
```

我们可以用类似的方式实现`MarkdownListStrategy`，但是因为Markdown不需要开始/结束标记，所以我们将只覆盖`add_list_item()`函数。

```c++
struct MarkdownListStrategy : ListStrategy
{
    void add_list_item(ostringstream& oss) override 
    {
        oss << "*" << item;
    }
};
```

现在我们可以开始使用文本处理器，为其提供不同的策略并得到不同的结果。例如:

```c++

TextProcessor tp;
tp.set_output_format(OutputFormat::markdown);
tp.append_list({"foo", "bar", "baz"});
cout << tp.str() << endl;

// Output:
// * foo
// * bar
// * baz

```

我们可以为在运行时可切换的策略做准备，这正是我们称之为动态策略的原因。这是在`set_output_format()`函数中完成的，它的实现很简单

```c++
 void set_output_format(OutputFormat format)
    {
        switch(format)
        {
            case OutputFormat::markdown:
                list_strategy = make_unique<MarkdownListStrategy>();
            case OutputFormat::html:
                list_strategy = make_unique<HtmlListStrategy>();
            break;

        }
    }
```

现在，从一种策略切换到另一种策略是很简单的，你可以直接看到结果:


```c++
tp.clear(); // clears the buffer
tp.set_output_format(OutputFormat::Html);
tp.append_list({"foo", "bar", "baz"});
cout << tp.str() << endl;
// Output:
// <ul>
// <li>foo</li>
// <li>bar</li>
// <li>baz</li>
// </ul>
```

#### 静态策略