#include <iostream>
using namespace std;
 
class Base{
public:
     void f1()
     {
          cout<<"B::f1()"<<endl;
     }
     virtual void f2()
     {
          cout<<"B::f2()"<<endl;
     }
};
 
class Derived : public Base
{
public:
     //覆盖
     void f1()
     {
          cout<<"D::f1()"<<endl;
     }
     //重写
     virtual void f2()
     {
          cout<<"D::f2()"<<endl;
     }
};
 
int main()
{
     Base* pbase = new Base();
     Base* pderived = new Derived();
     pbase->f1();             //调用Base::f1()
     pbase->f2();             //调用Base::f2()
     pderived->f1();          //调用Base::f1()
     pderived->f2();          //调用Derived::f2()，体现多态性
     return 0;
}

