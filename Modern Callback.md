# Modern Callback
## 问题起源
在异步编程的历史上，callback霸占了程序员相当长的时间。一般来说，callback有两种做法：
一、一次设置回调，多次调用。
这种模式常见于异步网络中。即便到现在也有大量程序员坚持这种用法。好处显然是不用浪费每次设置回调的开销。但对于复杂逻辑来说，就不太友好了。因为回调入口只有一个，要实现复杂的逻辑，只能自己在回调里想方设法折腾。今天我们要谈的不是这种类型。
二、一次设置回调，一次调用。
这种模式常用于文件IO，因为这种类似的操作不会有多次结果。坏处嘛，当然是会在设置回调上多了开销，但应对复杂的逻辑来说，具有相当好的深度可供挖掘。今天重点就是要谈这种类型的回调中的一种。
一次设置，一次调用，还可以根据对失败的处理，细分下使用方式：
1、设置时失败，则返回错误信息，同时不会调用回调接口；
2、异步操作失败，不会调用回调接口；
3、异步操作失败会调用回调接口，但是取消异步操作则不会调用回调接口；
4、任何情况下都会调用回调接口；
今天要谈的就是第4种使用方式。我认为前三种使用方式都是错误的。特别是第一种，将失败处理硬生生拆分成两部分逻辑，看不到任何好处。
那么，第4种异步回调的常用代码大致如下：

    template<typename _Input_t, typename _Callable_t>
    void tostring_async(_Input_t&& value, _Callable_t&& callback) 
    {
        std::thread([callback = std::forward(callback), value = std::forward<_Input_t>(value)]
            {
                callback(std::to_string(value));
            }).detach();
    }
使用代码就不具体讨论了，但凡有异步编程经验的程序员都能想象该如何使用该函数。
然而，这样的回调用起来并没有那么美好。在复杂逻辑下，很快会陷入：
！！！CALLBACK HELL！！！
为了解决这种深度的连续回调逻辑，聪明的程序员又发明了调用链来解决。通常就是future+then方案。代码大致如下：

    template<typename _Input_t>
    std::future<std::string> tostring_async(_Input_t&& value)
    {
        return std::async([value = std::forward<_Input_t>(value)]
            {
                return std::to_string(value);
            });
    }
    
    //使用范例代码
    tostring_async(1.0).then([](std::string && result)
    {
        std::cout << result << std::endl;
    });
然而，上面的代码目前并不能编译通过。因为std::future::then()是即将到来的C++20的功能。不管这段代码是否能编译通过，这种任务链的库还是挺多的，比较知名的如folly，以及比较不知名的libst。
再次然而，如果仅仅是非常非常深的连续回调，大部分程序员还能掌控，配合lambda食用，也不是什么难题。即便没有 future & then 可用的情况下，也只是要一个宽一点的显示器而已。现在的显示器都比较宽，不是问题。但是，要用回调模拟循环，然后再涉及到分支，大部分程序员就很难掌控了。即便诸如陈硕这样的大神，都讨厌这种逻辑。因为要实现循环分支，lambda就不是那么好用了。大体代码会长得像这个样子：

    void do_accept(...)
    {
        if (!ec)
            do_read(...);
    }
    void do_read(...)
    {
        if (!ec)
            do_write(...);
        else
            do_accept(...);
    }
    void do_write(...)
    {
        if (!ec)
            do_read(...);
    }
貌似走到了山穷水尽的末路！
但我们仔细想第二种做法的第4种使用方式，其内在逻辑，其实就是顺序执行逻辑。在不考虑异常的情况下，一行代码执行完毕后，必然会执行下一行代码，且只执行一次下一行代码。只不过在两行代码之间，插入了一个透明的延迟而已。那么，适用这种模式的解决方案就呼之欲出，那就是：协程！！！
哦，这个古老而富有魅力的美女，一下子就让我们拨云见日。赶紧的，支持下协程！以下代码以支持librf为例：

    template<typename _Input_t>
    resumef::future_t<std::string> tostring_async(_Input_t value)
    {
        resumef::promise_t<std::string> awaitable;
        std::thread([value, st = awaitable._state]
            {
                st->set_value(std::to_string(value));
            }).detach();
    
        return awaitable.get_future();
    }
    
    //使用范例代码
    std::string result = co_await tostring_async(val);
    std::cout << result << std::endl;
美，真美！循环，分支全都不是问题。简单到如初学写代码的码农都能掌握，谁还没写过顺序执行的代码呢？！
但是，等等，你的librf完善吗？稳定吗？高效吗？支持Linux吗？我想支持boost::fiber怎么办？我想支持libgo怎么办？
呵呵，能怎么办？赶紧回去996.ICU咯。我相信boost::fiber和libgo都提供了扩展的能力，你只需要翻一翻他们的文档，针对每一个异步函数写一遍适配代码，然后不要忘记写一遍测试范例。记得周末来加班哦！
但你大概率会在下周一继续犯愁。因为你用了一个第三方的异步库，你没权限，或者没源码，或者没能力(嗯有点尴尬)，去扩展这个库。
哦，你都有啊，很好，赶紧回去996.ICU咯。我相信你通过一周的加班加点，终于完美的扩展好了这个第三方异步库，并且针对每个函数每个功能都写了严格的测试范例，完美的完成了所有功能，确保了99.99999%的可靠性(然而还是会崩溃一两次），下一个季度的KPI有指望了！
但你还是有一定概率在下下个周一遇上意外。因为第三方库的作者发布申明，发现了N个XXX漏洞，针对这些漏洞，做了YYY修改，涉及到M个ZZZ接口。你一看ZZZ接口列表，一口老血就喷在了显示器上，然后就觉得这个世界正在变得黑暗，声音也逐渐离你远去----你昏倒在了工位上。
从医院缓缓苏醒的你，并没有放弃。因为你是如此的坚(gu)持(zhi)，是如此的优秀，以至于在昏倒的时候，耶稣上帝还有佛祖(哦还有三清)，感动得给你指明了一条康庄大道。就等你恢复了身体能自己下床尿尿后，把这条“道”给写出来！
妈耶，写了这么多，居然还没有进入正题，罪过罪过。不过，答应人家江南的东西，也总不能老用忙来拖延吧？期望下周没那么忙，能把Modern Callback(下一篇)写完。

## 解决方案
