#include <iostream>
using namespace std;

void SayHello();

int main()
{
   SayHello();
   return 0;
}

void SayHello()
{
   cout << "Hello World" << endl;
   return; // an empty return
}