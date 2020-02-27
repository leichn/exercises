#include <iostream>
using namespace std;

int main()
{
Start:
   int Num1 = 0, Num2 = 0;

   cout << "Enter two integers: " << endl;
   cin >> Num1;
   cin >> Num2;

   cout << Num1 << " x " << Num2 << " = " << Num1 * Num2 << endl;
   cout << Num1 << " + " << Num2 << " = " << Num1 + Num2 << endl;

   cout << "Do you wish to perform another operation (y/n)?" << endl;
   char repeat = 'y';
   cin >> repeat;

   if (repeat == 'y')
	   goto Start;

   cout << "Goodbye!" << endl;

   return 0;
}