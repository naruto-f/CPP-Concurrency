/*
 * Created by 123456 on 2022/7/27.
 * Description : 线程安全的队列的使用实例，部分是伪代码。
 */

#include "threadsafeQueue.hpp"

struct data_chunk
{

};

bool more_data_to_prepare() { return true; }

struct data_chunk data_prepare() { return data_chunk(); }

void process(struct data_chunk data) {  }

bool is_last_chunk(struct data_chunk) { return true; }


/***                         示例                                                   ***/
threadsafe_queue<data_chunk> data_queue;

/* 生产者线程 */
void data_preparation_thread()
{
    while(more_data_to_prepare())
    {
        const data_chunk data = data_prepare();
        data_queue.push(data);
    }
}

/* 消费者线程 */
void data_processing_thread()
{
    while(true)
    {
        data_chunk data;
        data_queue.wait_and_pop(data);
        process(data);
        if(is_last_chunk(data))
        {
            break;
        }
    }
}