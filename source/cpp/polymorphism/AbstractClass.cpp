#include <iostream>
#include <stdio.h>
using namespace std;

class B
{
public:
    virtual void func1() = 0;   // 纯虚函数不能在基类中实现，一定要在派生类中实现
    virtual void func2() = 0;   // 纯虚函数不能在基类中实现，一定要在派生类中实现
    virtual void func3() { cout << "B::func3" << endl; }    // 此虚函数被派生类中函数覆盖
    virtual void func4() { cout << "B::func4" << endl; }    // 此虚函数在派生类中无覆盖
            void func5() { cout << "B::func5" << endl; }    // 此函数被派生类中函数覆盖
            void func6() { cout << "B::func6" << endl; }    // 此函数在派生类中无覆盖

private:
    int x = 1;
    int y = 2;
    static int z;
};

class D : public B
{
public:
    virtual void func1() override { cout << "D::func1" << endl; }
    virtual void func2() override { cout << "D::func2" << endl; }
    virtual void func3() override { cout << "D::func3" << endl; }
            void func5()          { cout << "D::func5" << endl; }   // 不能带 overide

private:
    int u = 11;
    int v = 22;
    static int w;
};

int main()
{
    // B b;  // 编译错误，抽象基类不能被实例化
    D d;

    cout << "sizeof(d) = " << sizeof(d) << endl;
    d.func1();      // 访问派生类中的覆盖函数(覆盖纯虚函数)
    d.func2();      // 访问派生类中的覆盖函数(覆盖纯虚函数)
    d.func3();      // 访问派生类中的覆盖函数(覆盖虚函数)
    d.func5();      // 访问派生类中的覆盖函数(覆盖普通函数)
    d.B::func3();   // 访问基类中的虚函数
    d.B::func4();   // 访问基类中的虚函数
    d.B::func5();   // 访问基类中的普通函数

    return 0;
}