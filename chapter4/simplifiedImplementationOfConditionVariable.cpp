/*
 * Created by 123456 on 2022/7/27.
 * Description : c++条件变量相关的一些底层实现原理。
 * wait的工作原理: wait()需要和一种互斥量配合使用，std::condition_variable只能与std::mutex一起使用，std::condition_variable_any
 *                 可以与更合适的互斥量(如shared_mutex)一起使用。而且wait在调用时会假设互斥量已经上锁，①当第二参数即判断条件是否满足
 *                 的谓词为真时直接返回，继续执行下面的代码，②当条件为假时先释放互斥量的锁，然后放弃cpu主动休眠一段时间，知道被notify
 *                 唤醒时，先对互斥量加锁然后再次检查条件是否满足，如不满足则再次放弃锁并休眠。由与可能需要多次对互斥量进行加锁解锁，
 *                 所以wait()只能与灵活的unique_lock配合使用，而不能与lock_guard一起使用。
 */


#include <mutex>
#include <thread>
#include <chrono>
#include <queue>
#include <condition_variable>



/***   不使用条件变量的线程等待原理: 获取锁后检测共享标志变量，①如果标志可用就直接返回，
       ②如果标志不可以就先释放锁，然后主动休眠一段时间，醒来时再检测标志是否可用           ***/
bool flag;
std::mutex m;

void wait_for_flag()
{
    std::unique_lock<std::mutex> lock(m);
    if(!flag)
    {
        lock.unlock();

        /* 主动放弃cpu, 休眠100s */
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        lock.lock();
    }
}



/***         忙碌-等待的简单实现                                                          ***/

/* std::condition_variable::wait是忙碌-等待的优化，要注意的一点是wait假设在调用前lock已经上锁，如果实际传入的lock未上锁可能会
   未定义的行为，例如std::mutex不允许二次解锁 */

template<class Predicate>
void minimal_wait(std::unique_lock<std::mutex> &lock, Predicate pred)
{
    if(!pred())
    {
        lock.unlock();   //

        /* unlock()和lock()之间的时间间隔可能很短，取决于cpu的调度策略和具体工作负载 */

        lock.lock();
    }
}



/***         实例: 使用std::condition_variable处理数据等待(伪代码或者说是不完全的实现，因为只是为了演示wait和notify的使用方法)                                                     ***/

/* 有两个线程，一个准备数据并往队列中添加，一个使用数据来处理一些东西 */
std::queue<data_chunk> data_queue;
std::mutex mut;
std::condition_variable data_cond;

//生产者线程
void data_preparation_thread()
{
    while(more_data_to_prepare())
    {
        const data_chunk data = prepare_data();
        std::lock_guard<std::mutex> lock(mux);
        data_queue.push(data);
        data_queue.notify_one();
    }
}

//消费者线程
void data_processing_thread()
{
    while(true)
    {
        /* 由于wait的底层实现，所以需要unique_lock的灵活性来配合wait使用，不能使用lock_guard */
        std::unique_lock<std::mutex> lock(mut);

        data_cond.wait(lock, []{ return !data_queue.empty(); })

        data_chunk data = data_queue.front();
        data_queue.pop();

        /* 由于对数据的处理可能很耗时，一直占有锁会降低系统的效率和资源的利用率，所以可以利用unique_lock灵活性提前释放锁，减小锁的粒度 */
        lock.unlock();

        process(data);

        if(is_last_chunk(data))
        {
            break;
        }
    }
}
