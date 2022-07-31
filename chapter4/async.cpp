/*
 * Created by jinfan36 on 2022/7/31.
 * Description : std::async的用法
 *               ①第二个模板实参应该是一个可调用对象如普通函数，成员函数，lambda表达式，仿函数(重载了调用运算符的类)。
 *               ②对async的使用类似与对bind的使用，即对实参直接拷贝，无法传入左值引用，使用std::ref可以在模板传参时传入左值引用，但async对右值的拷贝操作将使用移动的方式转移原始数据。
 */


#include <future>
#include <string>

struct X
{
    void foo(int, std::string const&);
    std::string bar(std::string const&);
};

X x;
std::future<void> f1 = std::async(&X::foo, &x, 42, "hello");  //调用x->foo(42, ""hello")
auto f2 = std::async(&X::bar, x, "goodbay");        //调用temx.bar("goodbay"), tem为x的拷贝副本




struct Y
{
    double operator()(double);
};

Y y;
auto f3 = std::async(Y(), 3.141);                  //调用temy(3.141), temy通过Y的移动构造函数得到
auto f4 = std::async(std::ref(y), 3.141);       //调用y(3.141)




/* 函数baz的声明 */
X baz(X&);
auto f = std::async(baz, std::ref(x));   //调用baz(x)




/* 只能移动的函数对象类 */
class move_only
{
public:
    move_only();
    move_only(move_only&&);
    move_only(move_only const&) = delete;
    move_only& operator=(move_only&&);
    move_only& operator=(move_only const&) = delete;

    void operator()();
};

auto f5 = std::async(move_only());     //调用tmp(), tmp是通过std::move(move_only())构造得到





/*  async的两种可选启动方式(作为async调用的第一个实参):
 *  ①std::launch::deferred: 同步方式，表明函数调用延迟到wait()或get()函数调用时才执行，然后阻塞线程知道future就绪为止，并返回计算结果
 *  ②std::launch::async: 异步方式，表明函数必须在其所在的独立线程上执行
 *  ③std::launch::deferred | std::launch::async: 表明实现可以选择这两种方式的一种。当不填写第一个参数，不确定是哪种启动模式。
 */

auto f6 = std::async(std::launch::async, Y(), 1.2);                         //在新线程上执行
auto f7 = std::async(std::launch::deferred, baz, std::ref(x));           //在wait()或get()调用时执行
auto f8 = std::async(std::launch::deferred | std::launch::async, baz, std::ref(x));
auto f9 = std::async(baz, std::ref(x));

auto result = f7.get();
