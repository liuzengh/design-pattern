### 适配器

我过去经常旅行，旅行时的电源适配器使得我能够将欧洲标准的插头插在英国或美国标准的插座上，这是对适配器模式很好的类比：我们想要的接口和现有的接口不同时，我们可以在现有的接口上构造一个适配器来支持想要的接口。

#### 场景

这里有一个简单的例子:假设你正在使用一个擅长绘制像素的库。另一方面，你处理的是几何物体线条，矩形，诸如此类。你想继续使用这些对象，但也需要渲染，所以你需要调整你的几何体以*适配(adpat)*基于像素的表示。

让我们从定义示例的(相当简单的)域对象开始：

```c++
struct Point 
{
    int x, y;
};

struct Line 
{
    Point start, end;
};
```

现在让我们来讨论向量几何。一个典型的向量对象可能是由直线`Line`对象的集合定义的。我们可以定义一对纯虚迭代器方法，而不是从`vector<Line>`中继承

```c++
struct VectorObject 
{
    virtual std::vector<Line>::iterator begin( ) = 0;
    virtual std::vector<Line>::iterator end( ) = 0;
};
```
因此，如果你想定义一个矩形对象`Rectangle`，你可以在`vector<Line>`中保存一系列的点，并简单地暴露端点。

```c++
struct VectorRectangle : VectorObject 
{
    
    VectorRectangle( int x, int y, int width, int height ):
        width_( width ),
        height_( height )
    {
    lines.emplace_back( Line{ Point{ x, y }, Point{ x + width, y } });
    lines.emplace_back(Line{ Point{ x + width, y }, Point{ x + width, y + height} });
    lines.emplace_back(Line{ Point{x,y},
        Point{x,y+height} });
    lines.emplace_back(Line{ Point{ x,y + height },
        Point{ x + width, y + height } });
    }
    std::vector<Line>::iterator begin( ) override 
    {
        return lines_.begin();
    }
    std::vector<Line>::iterator end( ) override 
    {
        return lines_.end( );
    }
    private:
        int width_;
        int height_;
        std::vector<Line> lines_;
};
```

现在，设置好了。假设我们想在屏幕上画线段，甚至矩形。不幸的是，我们不能做不到，因为绘图的唯一接口是这样的:

```c++
void DrawPoints(CPaintDC& dc, std::vector<Point>::iterator start,
std::vector<Point>::iterator end
)
{
    for( auto i = start; i != end; ++i )
        dc.SetPixel( i->x, i->y, 0 );
};
```

我在这里使用来自`MFC`(微软基础类)的`CPaintDC`类，但这不是重点。关键是我们需要像素点， 但我们只有直线。我们需要一个适配器。

#### 适配器

好，假设我们要画几个矩形:

```c++
vector<shared_ptr<VectorObject>> vectorObjects
{
    make_shard<VectorObject>(10, 10, 100, 100),
    make_shard<VectorObject>(10, 10, 100, 100)
}
```

为了绘制这些对象，我们需要将它们从一系列的线转换成大量的点。为此，我们创建了一个单独的类来存储这些点，并将它们作为一对迭代器公开:

```c++
struct LineToPointAdapter
{
    using Points = vector<Point>;
    LineToPointAdapter(Line& line)
    {
        // TODO
    }
    virtual Points::iterator begin() 
    {
        return points.begin();
    }
    virtual Points::iterator end()
    {
        return points.end();
    }
    private:
        Points points;
}
```

从线段到许多点的转换正好发生在构造函数中，因此适配器是`eager`[^2]。转换的实际代码也相当简单:

```c++
LineToPointAdapter(Line& line)
{
    int left = min(line.start.x, line.end.x);
    int right = max(line.start.x, line.end.x);
    int top = min(line.start.y, line.end.y);
    int bottom = max(line.start.y, line.end.y);
    int dx = right - left;
    int dy = line.end.y - line.start.y;

     // only vertical or horizontal lines
    if (dx == 0)
    {
     // vertical
        for (int y = top; y <= bottom; ++y)
        {
            points.emplace_back( Point{ left, y } );
        }
    }
    else if (dy == 0)
    {
        for (int x = left; x <= right; ++x)
        {
            points.emplace_back(Point{ x, top });
        }
     }
}
```

上面的代码很简单:我们只处理完全垂直或水平的行，忽略其他所有东西。现在我们可以使用这个适配器来实际渲染一些对象。我们从示例中选取两个矩形，并简单地像这样渲染它们:

```c++
for (auto&& obj : vectorObjects)
{
    for (auto&& line : *obj)
    {
        LineToPointAdapter lpo{ line };
        DrawPoints(dc, lpo.begin(), lpo.end());
    }
};
```

漂亮!我们所要做的就是，获取每个向量对象的每条线段，为这条线构造一个`LineToPointAdapter`，然后迭代适配器生成的点集，将它们提供给`drawpoint()`。这样的确可行！(相信我，确实如此。)


#### 适配器的临时变量

然而，我们的代码有一个主要的问题:`DrawPoints()`会在我们可能需要的每一次屏幕刷新时被调用，这意味着相同的行对象的相同数据会被适配器重新生成，比如，无数次。

我们能做些什么呢?一方面，我们可以在应用程序启动时预先定义所有的点：

```c++
vector<Point> points;
for (auto&& o : vectorObjects)
{
    for (auto&& l : *o)
    {
        LineToPointAdapter lpo{ l };
        for (auto&& p : lpo)
            points.push_back(p);
    }
}
```

然后`drawpoint()`的实现简化为:

```c++
DrawPoints(dc, points.begin(), points.end());
```

但让我们暂时假设，原始的`vectorObjects`集合可以改变。缓存这些点是没有意义的，但我们仍然希望避免潜在重复数据的不断更新。我们如何处理这个问题?当然，使用缓存!

首先，为了避重复生成，我们需要独特的标识线段的方法，这也就意味着我们需要独特的标识点的方式。我们可以使用 ReSharper's 生成|哈希函数(**Generate | Hash function**)来解决这个问题:

```c++
struct Point
{
    int x, y;
    friend std::size_t hash_value(const Point& obj)
    {
        std::size_t seed = 0x725C686F;
        boost::hash_combine(seed, obj.x);
        boost::hash_combine(seed, obj.y);
        return seed;`
    }
};

struct Line
{
    Point start, end;
    friend std::size_t hash_value(const Line& obj)
    {
        std::size_t seed = 0x719E6B16;
        boost::hash_combine(seed, obj.start);
        boost::hash_combine(seed, obj.end);
        return seed;
    }
};
```

在前面的例子中，我选择了Boost’s散列实现。现在，我们可以构建一个新的`LineToPointCachingAdapter`，以便它缓存这些点，并仅在必要时重新生成它们。除了以下细微差别之外，实现几乎是相同的。

首先，适配器现在有一个缓存:

```c++
static map<size_t, Points> cache;
```

这里的`size_t`类型正是`Boost's`散列函数返回的类型。现在，当涉及到迭代生成的点时，我们将生成如下所示的点:

```c++
virtual Points::iterator begin() 
{
    return cache[line_hash].
    begin(); 
}

virtual Points::iterator end() { 
    return cache[line_hash].
    end(); 
}
```

这个算法的有趣之处在于:在生成这些点之前，我们要检查它们是否已经生成过了。如果有，我们就退出; 如果没有，则生成它们并添加到缓存中:

```c++
LineToPointCachingAdapter(Line& line)
{
    static boost::hash<Line> hash;
    line_hash = hash(line); // note: line_hash is a field!
    if (cache.find(line_hash) != cache.end())
        return; // we already have it
    Points points;
    // same code as before
    cache[line_hash] = points;
}
```


耶!多亏了散列函数和缓存，我们大大减少了转换的次数。唯一遗留的问题是当不再需要过期的点时，我们需要删除它们。这个有挑战性的问题留给读者作为练习。

#### 总结

适配器是一个非常简单的概念:它允许你调整所拥有的接口以适应所需要的接口。适配器的唯一实际问题是，在适应过程中，有时需要生成临时数据，以满足数据的其他表示形式。当这种情况发生时，请使用缓存:确保只在必要时才生成新数据。哦，如果你想在缓存的对象发生变化时清理过时数据，那么你还需要做更多的工作。

我们还没有真正解决的另一个问题是延迟加载:当前适配器实现一创建就执行转换。如果你只想在实际使用适配器时完成工作，该怎么办?这很容易做到，留给读者作为练习。