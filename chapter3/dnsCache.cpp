/*
 * Created by 123456 on 2022/7/27.
 * Description : 使用c++17提供的互斥量std::shared_mutex实现线程安全的dns缓存(读取和更新)，是读写者问题。
 */

#include <map>
#include <string>
#include <mutex>
#include <shared_mutex>

class dns_entry
{
    /* 占位，c++编译器要求在编译时看到类的全部定义以为类分配合适的内存空间, 如果类为空则会分配1字节内存空间而不是0，只有声明会编译报错 */
};

class dns_cache
{
public:
    /* 使用std::shared_lock允许多个线程并发的查询 */
    dns_entry find_entry(const std::string &domain) const
    {
        std::shared_lock<std::shared_mutex> lock(entry_mutex);
        auto resultIter =  entries.find(domain);
        return (resultIter == entries.end()) ? dns_entry() : resultIter->second;
    }

    /* 使用std::lock_guard或std::unique_lock实现同一时间只允许一个线程修改或更新缓存表 */
    void update_or_add_entry(const std::string &domain, const dns_entry &dns_details)
    {
        std::lock_guard<std::shared_mutex> lock(entry_mutex);   //这里将std::lock_guard替换为std::unique_lock效果相同
        entries[domain] = dns_details;
    }
private:
    std::map<std::string, dns_entry> entries;
    /* 声明为mutable的成员变量即使在const成员函数(此类中是find_entry函数)中也可以改变，如果不声明为mutable就在const成员函数中改变，编译会报错 */
    mutable std::shared_mutex entry_mutex;
};

