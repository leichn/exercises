#include <iostream>
#include <string>
using namespace std;

int main()
{
   // Declare a variable to store an integer
   int InputNumber;

   cout << "Enter an integer: ";

   // store integer given user input
   cin >> InputNumber;

   // The same with text i.e. string data
   cout << "Enter your name: ";
   string InputName;
   cin >> InputName;

   cout << InputName << " entered " << InputNumber << endl;

   return 0;
}