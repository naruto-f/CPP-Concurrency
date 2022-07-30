/*
 * Created by jinfan36 on 2022/7/30.
 * Description : c++时间点的简单使用，带超时功能的条件变量wait操作, 当没有什么什么可以等待时，在一个时限内等待条件变量。
 */

#include <chrono>
#include <condition_variable>
#include <mutex>

std::condition_variable cv;
std::mutex m;
bool done;


bool wait_loop()
{
    auto const timeout = std::chrono::steady_clock::now() + std::chrono::milliseconds(500);
    std::unique_lock<std::mutex> lock(m);

    while(!done)
    {
        /* 这里使用wait_until等待到一个确切的时间点，而不是使用wait_for等待一段时间，是为了处理假唤醒问题，因为操作系统只是保证唤醒一个等待的
         * 线程后讲其置为就绪状态，而不保证唤醒后立即运行该线程，具体情况却决于cpu调度程序，所以线程有可能没有被立即调度而继续休眠等待，极端情况
         * 下可能出现无限等待的可能。
         */
        if(cv.wait_until(lock, timeout) == std::cv_status::timeout)
        {
            break;
        }
    }

    return done;
}
