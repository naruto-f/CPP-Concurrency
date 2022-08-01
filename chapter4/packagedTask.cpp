/*
 * Created by 123456 on 2022/8/1.
 * Description : std::packaged_task<signature>是一个可调用对象，模板参数是一个函数签名
 *               优点: 可以将任务打包，并适时的取回future
 */


#include <iostream>
#include <future>
#include <vector>
#include <deque>
#include <thread>
#include <mutex>
#include <utility>

/***         通过packaged_task一种偏特化说明packaged_task的工作原理                          ***/
template<>
class std::packaged_task<std::string(std::vector<char>*, int)>
{
public:
    /* 构造函数接受一个可调用对象，这个可调用对象的签名要与模板参数的函数签名相同或能隐式转换 */
    template<typename Callable>
    explicit packaged_task(Callable &&f);

    /* 当异步任务需要返回值，需要等待future变为就绪 */
    std::future<std::string> get_future();

    /* 当packaged_task被作为函数调用时，实参由函数调用运算符传到底层函数，并将返回值作为异步结果存储在future中，可通过get_futrue获取 */
    void operator()(std::vector<char>*, int);
};



/***   gui程序: 使用packaged_task实现一个线程请求对界面更新，由gui后台处理线程完成界面更新，且不用发送自定义信息给更新线程                                                            ***/
std::mutex tasks_mutex;
std::deque<std::packaged_task<void()>> tasks;         //任务队列

bool gui_shutdown_message_received();
void get_and_process_gui_message();

/* 由gui后台处理线程运行的函数 */
void gui_thread()
{
    while(!gui_shutdown_message_received())
    {
        get_and_process_gui_message();
        std::packaged_task<void()> task;
        {
            /* 这里的作用域运算符是为了限制lock的生命周期，减小锁的粒度 */
            std::lock_guard<std::mutex> lock(tasks_mutex);

            /* 当前任务队列为空, 则直接进行下一轮循环处理过程 */
            if(tasks.empty())
            {
                continue;
            }

            /* 因为这里我们能明确的知道队列首的任务会pop且不再使用，所以可以使用std::move将其移动赋值给task */
            task = std::move(tasks.front());
            tasks.pop_front();
        }
        task();
    }

}

std::thread gui_bg_thread(gui_thread);     //gui后台处理线程


/* 当用户点击或使用gui界面功能时，由前台线程运行的函数，功能是将由packaged_task包装的任务插入任务队列，并在插入队列前保留future */
template<class Func>
std::future<void> post_task_for_gui_thread(Func f)
{
    /* 这里使用最简单的无参无返回值的形式，当然也可以使用更复杂的函数形式
     * 需要注意的是，此时模板实参为void(), 所以当初始化task时传入的可调用对象若有返回值则返回值将被丢弃
     */
    std::packaged_task<void()> task(f);

    /* 将任务插入队列前保留future，从而当后台处理线程调用task后且future变为就绪就可以获取返回值 */
    auto res = task.get_future();

    std::lock_guard<std::mutex> lock(tasks_mutex);
    tasks.push_back(std::move(task));

    return res;
}