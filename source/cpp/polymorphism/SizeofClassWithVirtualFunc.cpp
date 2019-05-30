#include <iostream>
using namespace std;

class Class1
{
private:
   int a, b;

public:
   void DoSomething() {}
};

class Class2
{
private:
   int a, b;

public:
   virtual void DoSomething() {}
};

int main() 
{
   cout << "sizeof(Class1) = " << sizeof(Class1) << endl;
   cout << "sizeof(Class2) = " << sizeof(Class2) << endl;

   return 0;
}