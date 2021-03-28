### 第15章 解释器模式

你已经猜到了，解释器设计模式的目标是解释输入，特别是文本输入，不过公平地说，这真的无关紧要。解释器的概念与编译理论和大学里教授的类似课程有很大的联系。由于我们在这里没有足够的空间来深入研究不同类型的解析器的复杂性和诸如此类的东西，这一章的目的只是简单地展示一些你可能想要解释的事情的例子。

这里有几个相当明显的例子:
- 数字字面值(如42或1.234e12)需要被解释以有效地存储在二进制文件中。在c++中，这些操作通过C API(如`atof()`)和更复杂的库(如`Boost.LexicalCast`)来实现。
- 正则表达式帮助我们在文本中找到模式，但您需要认识到的是，正则表达式本质上是一种独立的、嵌入式领域特定语言(DSL)。当然，在使用它们之前，必须正确地解释它们
- 任何结构化数据，无论是CSV、XML、JSON，还是更复杂的数据，在使用之前都需要进行解释。
- 在解释器应用的顶峰，我们已经有了成熟的编程语言。毕竟，像C或Python这样的语言的编译器或解释器在使某些东西可执行之前必须真正理解该语言。

鉴于与解释有关的挑战的扩散和多样性，我们将简单地看一些例子。这些都说明了如何构建解释器:要么从零开始，要么利用一个库，在工业规模上帮助完成这些事情。

### 数值表达式计算器

假设我们决定解析非常简单的数学表达式，例如`3+(5-4)`，也就是说，我们将把自己限制在加法、减法和方括号中。我们需要一个程序能够读取这样的表达式，当然，也能够计算表达式的最终值。

我们将手工构建这样一个计算器，不借助于任何解析框架。这应该能够突出解析文本输入所涉及的一些复杂性。




### 词法分析

解释表达式的第一步称为词法分析，它涉及到将字符序列转换为`token`序列。`token`通常是一个基本语法元素，我们应该以这些元素的平面序列结束。在我们的例子中，`token`可以是：

- 整数
- 操作符（加法或减法）
- 一个开或闭的括号

我们可以定义如下的结构：

```c++
struct Token
{
    enum Type { integer, plus, minus, lparen, rparen } type;
    string text;
    // 这里有问题explicit 一般只修饰单个参数的构造函数避免显式转换。
    explicit Token(Type type, const string& text):
        type{ type },
        text{ text }
        {

        }
    friend ostream& operator<<(ostream& os, const Token& obj)
    {
        return os << "`" << obj.text << "`";
    }
};
```

你将注意到`Token`不是`enum`，因为除了类型之外，我们还希望存储与此`Token`相关的文本，因为它并不总是预定义的。

现在，给定一个包含表达式的std::string，我们可以定义一个词法分析过程，将文本转换为`vector<Token>`:

```c++
vector<Token> lex(const string& input)
{
    vector<Token> result;
    for(int i = 0; i < input.size(); ++i)
    {
        switch(input[i])
        {
            case '+':
                result.emplace_back( Token{Token::plus, "+"} );
                break;
            case '-'
                result.emplace_back( Token{Token::plus, "-"} );
                break;
            case '(':
                result.emplace_back( Token{Token::lparen, "("} );
                break;
            case ')':
                result.emplace_back( Token{Token::rparen, ")"} );
            default:
                // numer ???
        }
    }
}
```

解析预定义的标记很容易。事实上，我们可以把到`<map<BinaryOperation, char>>`进行简化。但是，解析一个数字并不那么容易。如果我们碰到了1，我们应该等待，看看下一个字符是什么。为此我们写了一个单独的代码块：

```c++
ostringstream buffer;
buffer << input[i];
for (int j = i + 1; j < input.size(); ++j)
{
    if (isdigit(input[j]))
    {
        buffer << input[j];
        ++i;
    }
    else
    {
        result.push_back(Token{ Token::integer, buffer.str() });
        break;
    }
};
```

当我们继续读取数字时，我们将它们添加到缓冲区中。完成后，我们从整个缓冲区中创建一个`token`，并将其添加到生成的`vector`中。


### 语法分析

语法分析(`parsing`)过程将一系列记号转换成有意义的、通常面向对象的结构。在顶部，有一个树的所有元素都需要实现的抽象父类型是很有用的：

```c++
struct Element
{
    virtual int eval( ) const = 0;
};
```

该类型的`eval()`函数计算该元素的数值。接下来，我们可以创建一个用于存储整数值的元素(例如1，5、42):

```c++
struct Interger : Element
{
    int value;
    explicit Interger(const int value):
        value{value} 
    {

    };
    int eval( ) const override { return value; }
};
```

如果没有整数，则必须有加法或减法之类的运算。在我们的例子中，所有的操作都是二元的，这意味着它们有两个部分。

```c++
struct BinaryOperation : Element
{
    enum Type { addition, substraction } type;
    shared_ptr<Element> lhs, rhs;
    
    int eval( ) const override
    {
        if(type == addition)
            return lhs->eval() + rhs->eval();
        return lhs->eval() - rhs->eval();
    }
};
```

注意，在上面的代码里面，我使用的是`enum`而不是`enum class`, 这样我就可以在后面写`BinaryOperation::add`。

不管怎样，我们来看看解析过程。我们所需要做的就是将一个记号序列转换为表达式的二叉树。从一开始，它可以看起来如下：

```c++
shared_ptr<Element> parse(const vector<Token>& tokens)
{
    auto result = make_unique<BinaryOperation>();
    bool have_lhs = false; // this will need some explaining :)
    for(auto token : tokens)
    {
        switch(token.type)
        {
            // process each of the tokens in turn
        }
    }
    return result;
}
```

在前面的代码中，我们只需要讨论`have_lhs`变量。记住，你试图得到的是一个树，在树的根（`root`），我们期望一个二元表达式(`BinaryExpression`)，根据定义，它有两棵子树。但是当我们在一个数字上时，我们怎么知道它是在表达式的左边还是右边呢?没错，我们不知道，所以我们才要追踪这个。

现在让我们逐案分析这些问题。首先，这些整数直接映射到我们的整数构造，所以我们所要做的就是将文本转换为数字。(顺便说一句，如果我们愿意，我们也可以在词法分析阶段这样做。)

```c++
case Token::interger
{
    int value = boost::lexical_cast<int>(token.text);
    auto integer = make_shared<Integer>(value);
    if(!has_lhs)
    {
        result->lhs = integer;
        have_lhs = true;
    }
    else
        result->rhs = integer;
}
```

加号和减号只是确定我们正在处理的操作的类型，所以它们很容易

```c++
case Token::plus:
    result->type = BinaryOperation::addition;
    break;
case Token::minus:
    result->type = BinaryOperation::subtraction;
break;
```

然后是左括号。是的，只有左括号，我们没有明确地检测到右边。这里的思想很简单:找到右括号(我现在忽略嵌套的括号)，删除整个子表达式，递归地`parse()`它，并将其设置为我们当前正在处理的表达式的左边或右边:

```c++
case Token::lparen:
{
    int j = i;
    for (; j < tokens.size(); ++j)
        if (tokens[j].type == Token::rparen)
            break; // found it!
    vector<Token> subexpression(&tokens[i + 1], &tokens[j]);
    auto element = parse(subexpression); // recursive call
    if (!have_lhs)
    {
        result->lhs = element;
        have_lhs = true;
    }
    else 
        result->rhs = element;
    i = j; // advance
}

```


在真实的场景中，你需要更多的安全特性:不仅要处理嵌套的括号(我认为这是必须的)，还要处理缺少右括号的不正确表达式。如果真的没有右括号，你会怎么处理?抛出一个异常呢?试着分析剩下的内容并假设结束在最后?别的吗?所有这些问题都留给读者作为练习。

从c++本身的经验中，我们知道为解释过程错误编写有意义的错误消息是非常困难的。事实上,你会发现一个现象叫做跳过(`skipping`),lexer或解析器将尝试跳过错误代码,直到遇到有意义的输入:这种方法被静态分析工具所采用, 当用户键入未完成的代码时，期望它能正确工作。

### 使用词法分析器和语法分析器

通过实现`lex()`和`parse()`，我们最终可以解析表达式并计算其值:

```c++
string input{ "(13-4)-(12+1)" };
auto tokens = lex(input);
auto parsed = parse(tokens);
cout << input << " = " << parsed->eval() << endl;
// prints "(13-4)-(12+1) = -4"
```

### 使用Boost.Spirit进行语法分析

在现实世界中，几乎没有人手工编写解析器来处理复杂的东西。当然，如果解析的是`XML`或`JSON`等简单的数据存储格式，那么手工编写解析器是很容易的。但如果你正在实现自己的DSL或编程语言，这就不是一个选项。

`Boost.Spirit`是一个库，通过为构造语法分析器提供简洁(但不是特别直观)的API来帮助创建语法分析器。该库不试图显式地分离词法分析和解析阶段(除非你真的想这样做)，允许你定义如何将文本构造映射到你定义的类型的对象上。

让我以`Tlön`[^1]编程语言为例，来向你展示`Boost.Spirit`的一些用法。

注1：`Tlön`是一个玩具语言，我创建它是为了演示一个想法:如果你不喜欢现有的语言，那就创建一个新的语言。`Tlön`使用`Boost.Spririt`和交叉编译(transpiles)为c++。它是开源的，可以在下面找到: [https://github.com/nesteruk/tlon](https://github.com/nesteruk/tlon)


#### 抽象语法树


首先，你需要你的抽象语法树（`Abstrct Syntax Tree, AST`）。在这方面，我只创建了一个支持访问者设计模式的基类，因为遍历这些结构非常重要。

```c++
struct ast_element
{
    virtual ~ast_element() = default;
    virtual void accept(ast_element_visitor& visitor) = 0;
};
```

然后，这个接口用于在我的语言中定义不同的代码结构:

```c++
struct property : ast_element
{
    vector<wstring> names;
    type_specification type;
    bool is_constant { false };
    wstring default_value;
    void accept(ast_element_visitor& visitor) override
    {
        visitor.visit(*this);
    }
}；
```

前面的属性定义有四个不同的部分，每个部分存储在一个公共可访问的字段中。注意，它使用了`type_specification`，它本身就是另一个`ast_element`。

AST的每个类都需要适应`Boost.Fusion`另一个支持编译时(元编程)和运行时算法融合(因此得名)的Boost库。适应代码非常简单:

```c++
BOOST_FUSION_ADAPT_STRUCT(
    tlön::property,
    (std::vector<std::wstring>, names),
    (tlön::type_specification, type),
    (bool, is_constant),
    (std::wstring, default_value)
)
```

Spirit解析为常见的数据类型，如`std::vector`或`std::optional`，没有问题。它在多态性方面确实有更多的问题:`Spirit`宁愿您使用一个`variant`，也就是说，而不是让AST类型彼此继承。

```c++
typedef variant<function_body, property, function_signature> class_member;
```


#### 语法分析器

`Boost.Spirit`允许我们将解析器定义为一组规则。所使用的语法非常类似于正则表达式或`BNF (Bachus-Naur Form)`表示法，只不过操作符放在符号之前，而不是之后。下面是一个示例规则:

```c++
class_declaration_rule %=
    lit(L"class ") >> +(alnum) % '.'
    >> -(lit(L"(") >> -parameter_declaration_rule % ',' >> lit(")"))
    >> "{"
    >> *(function_body_rule | property_rule | function_signature_rule)
    >> "}";
```

前面要求类声明以单词`class`开头。然后，它需要一个或多个单词(每个单词都是一个或多个字母数字字符，因此`+(alnum))`，用句号'分隔。'这就是%操作符的用途。正如您可能已经猜到的，结果将映射到一个向量上。随后，在花括号之后，我们期望零个或多个函数、属性或函数签名的定义，这些字段将使用一个变体映射到与我们之前的定义对应的位置。

当然，有些元素是`AST`元素的整个层次结构的根。在我们的例子中，这个根被称为一个文件(惊讶吧!)，这里有一个函数，它既解析文件，又对文件进行美化打印：

```c++
template<typename TLanguagePrinter, typename Iterator>
wstring parse(Iterator first, Iterator last)
{
    using spirit::qi::phrase_parse;
    file f;
    file_parser<wstring::const_iterator> fp{};
    auto b = phrase_parse(first, last, fp, space, f);
    if (b)
    {
        return TLanguagePrinter{}.pretty_print(f);
    }
    return wstring(L"FAIL");
 }
```

`TLanguagePrinter`实际上是一个访问者，它知道如何用不同的语言(比如c++)来呈现我们的AST。


#### 打印机

在解析了语言之后，我们可能想要编译它，或者在我的例子中，将它转换成其他语言。考虑到我们之前已经在整个AST层次结构中实现了`accept()`方法，这是相当容易的。

唯一的挑战是如何处理这些变体类型，因为它们需要特殊的访问者。在`std::variant`的情况下，你想要的是`std::visit()`，但是因为我们使用`boost::variant`，所以要调用的函数是`boost::accept_visitor()`。这个函数要求你为它提供从`static_visitor`继承的类的实例，并为每种可能的类型提供函数调用重载。这里有个例子:

```c++
struct default_value_visitor : static_visitor<>
{
    cpp_printer& printer;

    explicit default_value_visitor(cpp_printer& printer)
        : printer{printer}
    {
    }

    void operator()(const basic_type& bt) const
    {
        // for a scalar value, we just dump its default
        printer.buffer << printer.default_value_for(bt.name);
    }

    void operator()(const tuple_signature& ts) const
    {
        for (auto& e : ts.elements)
        {
            this->operator()(e.type);
            printer.buffer << ", ";
        }
        printer.backtrack(2);
        }
 };
```

然后调用`accept_visitor(foo, default_value_ visitor{…})`，根据变量中实际存储的对象类型，将调用正确的重载。

### 总结

首先，需要说明的是，相对而言，解释器设计模式是不常见的，构建解析器的挑战现在被认为是不必要的，这就是为什么我看到许多英国大学(包括我自己的大学)的计算机科学课程中删除了它。此外，除非您计划从事语言设计工作，或者制作静态代码分析工具，否则您不太可能在高需求中找到构建解析器的技能。



也就是说，解释的挑战是计算机科学中一个完全独立的领域，设计模式的书中的一个章节不能合理地解释它。如果你对这个主题感兴趣，我建议你看看`Lex/Yacc`、`ANTLR`和其他许多专门针对`lexer/parse`构造的框架。我还建议为流行的`IDE`编写静态分析插件，这是了解真实的`ast`外观、遍历方式甚至修改方式的好方法。