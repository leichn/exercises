#include <iostream>
using namespace std;

int main()
{
   cout << "Enter a number: ";
   int value = 0;
   cin >> value;

   value += 8;
   cout << "After += 8, value = " << value << endl;
   value -= 2;
   cout << "After -= 2, value = " << value << endl;
   value /= 4;
   cout << "After /= 4, value = " << value << endl;
   value *= 4;
   cout << "After *= 4, value = " << value << endl;
   value %= 1000;
   cout << "After %= 1000, value = " << value << endl;

   // Note: henceforth assignment happens within cout 
   cout << "After <<= 1, value = " << (value <<= 1) << endl;
   cout << "After >>= 2, value = " << (value >>= 2) << endl;

   cout << "After |= 0x55, value = " << (value |= 0x55) << endl;
   cout << "After ^= 0x55, value = " << (value ^= 0x55) << endl;
   cout << "After &= 0x0F, value = " << (value &= 0x0F) << endl;

   return 0;
}