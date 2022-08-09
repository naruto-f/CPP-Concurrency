/*
 * Created by 123456 on 2022/8/9.
 * Description : 使用std::atomic_flag实现自旋锁
 * Summary : ①std::atomic_flag的优点 : 有限的特性(clear()和test_and_set())让其非常适合用做自旋锁
 *           ②std::atomic_flag的缺点 : 局限性太强，没有非修改查询操作，甚至不能像普通的布尔变量一样使用，实际操作中最好使用std::atomic<bool>
 */

#ifndef CPP_CONCURRENCY_SPINLOCK_H
#define CPP_CONCURRENCY_SPINLOCK_H

#include <atomic>

/* 实现了最基本的自旋锁互斥量，它已经足够std::lock_guard<>使用了，其本质就是在lock()中等待，所以不可能有竞争的存在，并且可以确保互斥 */
class sqinlock_mutex {
public:
    /* std::atomic_flag类型的对象必须使用ATOMIC_FLAG_INIT初始化，且初始化标志位是清除状态，并且互斥量处于解锁状态 */
    sqinlock_mutex() : flag(ATOMIC_FLAG_INIT) {}


    void lock()
    {
        /* 为了锁上互斥量就是将循环使用test_and_set直到返回值为false，说明此时当前线程已获得互斥量 */
        while(flag.test_and_set(std::memory_order_acquire));
    }

    /* 解锁互斥量就是简单的将标志清除 */
    void unlock()
    {
        flag.clear(std::memory_order_release);
    }

private:
    std::atomic_flag flag;
};

#endif //CPP_CONCURRENCY_SPINLOCK_H
