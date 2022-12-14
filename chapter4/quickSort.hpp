/*
 * Created by 123456 on 2022/8/5.
 * Description : 使用函数式编程(FP)范式的快速排序算法的几种做法
 */

#include <list>
#include <algorithm>
#include <future>



/***                         FP模式的快速排序递归版本(串行)                             ***/

/* 这个函数有别于std::sort(), 因为std::sort()的入参是容器的非const迭代器，所以可以直接改变容器本身，而这里参数是整个容器，所以会有一些避免不了的拷贝开销 */
template<typename T>
std::list<T> sequential_quick_sort(std::list<T> input)
{
    if(input.empty())
    {
        return input;
    }

    std::list<T> result;

    /* 使用原list的首元素作为排序中心，先将其转移到最后要返回list中 */
    result.splice(result.begin(), input, input.begin());
    const T& pivot = *result.begin();

    /* 使用std::partion将input列表排序: 比排序中心pivot大的放前面，小的放后面，但不保证相对位置不变，如想保持稳定需使用std::stable_partion */
    auto divide_iter =  std::partition(input.begin(), input.end(), [&](const T& t) { return t < pivot; });

    /* 将比pivot小的元素都转移到lower_part中 */
    std::list<T> lower_part;
    lower_part.splice(lower_part.end(), input, input.begin(), divide_iter);

    /* 递归处理两个子list，这里由于我们明确知道以后不会再用到lower_part和input, 所以可以放心的将其绑定到右值引用从而减小参数拷贝的开销 */
    auto newLower(sequential_quick_sort(std::move(lower_part)));
    auto newHigher(sequential_quick_sort(std::move(input)));

    /* 以正确的顺序合并递归排序后的两个子list和中枢元素pivot */
    result.splice(result.end(), newHigher);
    result.splice(result.begin(), newLower);

    return result;
}



/***                           快速排序并行版                          ***/

template<typename T>
std::list<T> parallel_quick_sort(std::list<T> input)
{
    if(input.empty())
    {
        return std::list<T>();
    }

    std::list<T> result;
    result.splice(result.begin(), input, input.begin());
    const T& pivot = *result.begin();

    auto divideIter = std::partition(input.begin(), input.end(), [&](const T& value) { return value < pivot; });

    std::list<T> lower_part;
    lower_part.splice(lower_part.end(), input, input.begin(), divideIter);

    /* 调用std::async()使用另外的线程对lower_part部分排序，大于的部分继续使用递归排序 */
    std::future<std::list<T>> newLower(std::async(&parallel_quick_sort<T>, std::move(lower_part)));
    auto newHigher(parallel_quick_sort(std::move(input)));

    result.splice(result.end(), newHigher);

    /* ①在调用splice前，先调用newLower的get函数检索计算结果，有可能需要等待任务完成
     * ②futrue的get函数返回一个包含结果的右值引用，所以可以移动
     */
    result.splice(result.begin(), newLower.get());

    return result;
}




