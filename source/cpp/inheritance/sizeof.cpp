#include <iostream>
using namespace std;

class Base
{
private:
    int x = 1;
    int y = 2;
    const static int z = 3;
/*
public:
    virtual void test() {};
*/
};

class Derived : public Base
{
private:
    int u = 11;
    int v = 22;
    const static int w = 33;
/*
public:
    virtual void test() {};
*/
};

int main()
{
    Base base;
    Derived derived;

    cout << "sizeof(Base) = " << sizeof(Base) << endl;
    cout << "sizeof(Derived) = " << sizeof(Derived) << endl;

    return 0;
}