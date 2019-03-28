#include <iostream>
using namespace std;
 
class A{
public:
     void f1()
     {
          cout<<"A::f1()"<<endl;
     }
     virtual void f2()
     {
          cout<<"A::f2()"<<endl;
     }
};
 
class B:public A
{
public:
     //覆盖
     void f1()
     {
          cout<<"B::f1()"<<endl;
     }
     //重写
     virtual void f2()
     {
          cout<<"B::f2()"<<endl;
     }
};
 
int main()
{
     A* p = new B();
     B* q = new B();
     p->f1();          //调用A::f1()
     p->f2();          //调用B::f2()，体现多态性
     q->f1();          //调用B::f1()
     q->f2();          //调用B::f2()
     return 0;
}

