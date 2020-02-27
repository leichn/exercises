#include<iostream>
#include<string>
using namespace std;

int main()
{
   cout << "Enter your name: ";
   string name;
   getline(cin, name);
   cout << "Hi " << name << endl;

   return 0;
}
