/*
 * Created by 123456 on 2022/7/27.
 * Description : 构建线程安全的队列。
 */

#ifndef CPP_CONCURRENCY_THREADSAFEQUEUE_HPP
#define CPP_CONCURRENCY_THREADSAFEQUEUE_HPP

#include <mutex>
#include <queue>
#include <memory>
#include <condition_variable>

template<class T>
class threadsafe_queue
{
private:
    mutable std::mutex m;    //由于需要在为const成员函数的empty()中对互斥量m加锁(m状态改变了), 所以m需要声明为mutable。
    std::queue<T> data_queue;
    std::condition_variable data_cond;

public:
    threadsafe_queue() { }

    threadsafe_queue(const threadsafe_queue<T> &other_queue)
    {
        std::lock_guard<std::mutex> lock(m);
        data_queue = other_queue.data_queue;
    }

    /* 不允许使用赋值运算符简单的赋值 */
    threadsafe_queue& operator=(const threadsafe_queue<T> &) = delete;

    void push(T newValue)
    {
        std::lock_guard<std::mutex> lock(m);
        data_queue.push(newValue);

        /* 这里的notify_one不保证线程一定会被通知到 */
        data_cond.notify_one();
    }

    /* 重载的两个wait_and_pop函数是休眠并等待队列中有数据，而try_pop函数是判断没数据后直接返回 */
    void wait_and_pop(T &value)
    {
        std::unique_lock<std::mutex> lock(m);
        data_cond.wait(lock, [this] { return !data_queue.empty(); });
        value = data_queue.front();
        data_queue.pop();
    }

    std::shared_ptr<T> wait_and_pop()
    {
        std::unique_lock<std::mutex> lock(m);
        data_cond.wait(lock, [this] { return !data_queue.empty(); });
        std::shared_ptr<T> valuePtr = std::make_shared<T>(data_queue.front());
        data_queue.pop();
        return valuePtr;
    }

    bool try_pop(T &value)
    {
        /* 因为try_pop一旦发现队列为空就直接返回，所以不需要使用条件变量wait，也不需要使用与之配合使用的std::unique_lock */
        std::lock_guard<std::mutex> lock(m);
        if(data_queue.empty())
        {
            return false;
        }
        value = data_queue.front();
        data_queue.pop();
        return true;
    }

    std::shared_ptr<T> try_pop()
    {
        std::lock_guard<std::mutex> lock(m);
        if(data_queue.empty())
        {
            return std::shared_ptr<T>();
        }
        std::shared_ptr<T> valuePtr = std::make_shared<T>(data_queue.front());
        data_queue.pop();
        return valuePtr;
    }

    bool empty() const
    {
        std::lock_guard<std::mutex> lock(m);
        return data_queue.empty();
    }

};

#endif //CPP_CONCURRENCY_THREADSAFEQUEUE_HPP
