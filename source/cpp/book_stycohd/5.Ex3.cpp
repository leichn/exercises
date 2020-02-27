#include <iostream>
using namespace std;

int main()
{
   cout << "Enter a boolean value true(1) or false(0): ";
   bool Value1 = false;
   cin >> Value1;

   cout << "Enter another boolean value true(1) or false(0): ";
   bool Value2 = false;
   cin >> Value2;

   cout << "Result of bitwise operators on these operands: " << endl;
   cout << "Bitwise AND: " << (Value1 & Value2) << endl;
   cout << "Bitwise OR: " << (Value1 | Value2) << endl;
   cout << "Bitwise XOR: " << (Value1 ^ Value2) << endl;

   return 0;
}