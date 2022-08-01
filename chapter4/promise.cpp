/*
 * Created by 123456 on 2022/8/1.
 * Description : ①std::promise的用法
 *               std::promise/std::future对提供这么一种机制: futrue可以阻塞线程，提供数据的线程可以使用promise
 *               对相关值进行设置(set_value成员函数)，对应的future状态变为就绪. std::promise使用get_future获取
 *               与之相关的future。
 *               ②std::future的局限性: 当很多线程在等待的时候，只有一个线程能获取结果，这时就需要使用std::shared_future来替代std::future.
 *               因为std::future是只能移动不能拷贝的(move_only), 其所有权在不同的实例中互相传递，但是只有一个实例可以获得特定的同步结果;
 *               而std::shared_future实例是可拷贝的，所以多个对象可以引用同一个关联期望值得结果, 但引用同一个std::shared_future还是会造成数据竞争，可以通过加锁解决这个问题;
 *               但推荐这种方法: 可以让每个线程都拥有自己对应的拷贝std::shared_future，此时每个线程都通过自己拥有的std::shared_future对象获取结果，则多个线程访问共享同步结果就是安全的，也不用加锁。
 *               ③任何情况下，调用std::promise或std::packaged_task的析构函数(也会销毁相关的future对象)，编译器会创建一个future，并存储一个std::future_error异常; 也可能销毁值和异常源去违背promise，
 *               这种情况下，编译器没有在future中存储任何东西，线程可能会永远的等下去。
 */

#include <future>
#include <cmath>
#include <thread>

/***    伪代码: 利用promise解决单线程多连接问题，类似于select/poll的实现原理(事件驱动), 但没有考虑异常处理   ***/
void process_connection(connection_set& connections)
{
    while(!done(connections))
    {
        for(connection_iterator connection = connections.begin(), end = connections.end(); connection != end; ++connection)
        {
            /* 对于传入包，与future相关的数据就是数据包的有效负载 */
            if(connection->has_incoming_data())
            {
                data_packet data = connection->incoming();

                /* 假设输入数据包是具有ID和有效负载的，且一个ID映射到一个std::promise */
                std::promise<payload_type> &p = connection->get_promise(data.id);

                /* promise使用set_value成员函数后，future变为就绪 */
                p.set_value(data.payload);
            }

            /* 对于传出包，与future相关的只是简单的成功/失败标志，因此可以利用一对std::promise<bool>/std::future<bool>找出传出成功的数据块 */
            if(connection->has_outgoing_data())
            {
                outgoing_packet data = connection->top_of_outgoing_queue();
                connection->send(data.payload);
                data.promise.set_value(true);
            }
        }
    }
}



/***        可以将异常存在future中                                         ***/
double square_root(double x)
{
    if(x < 0)
    {
        throw std::out_of_range("x < 0");
    }
    return sqrt(x);
}

/* 假设调用square_root()函数不是当前线程，则将看不到异常 */
double y = square_root(-1);

/* ①将调用改为异步调用，函数作为std::async的一部分时，当调用抛出一个异常时
 * ②这个异常将存储在future中，当线程调用future.get()时，会抛出已存储在future中的异常，就能再看到异常了
 * 当将函数打包到std::package_task任务包后，当任务调用时，同样的事也会发生。
 */
std::future<double> f = std::async(square_root, -1);
double z = f.get();

/* 也可以通过函数的显式调用，std::promise也能提供同样的功能
 * 但当存入的是异常而不是数值时，需要调用set_exception成员函数，而非set_value.
 */
extern std::promise<double> some_promise;
try
{
    some_promise.set_value(calculate_value());
}
catch(...)
{
    some_promise.set_exception(std::current_exception());
}

//可以使用std::copy_exception()作为替代方案，其会直接存储新的异常而不抛出，当异常类型已知时应优先使用这种方式，
//不是因为代码实现简单，而是给编译期提供了极大的优化空间。
some_promise.set_exception(std::copy_exception(std::logic_error("foo ")));
