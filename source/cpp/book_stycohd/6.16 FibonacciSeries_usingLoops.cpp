#include <iostream>
using namespace std;

int main()
{
   const int numsToCalculate = 5;
   cout << "This program will calculate " << numsToCalculate \
        << " Fibonacci Numbers at a time" << endl;

   int num1 = 0, num2 = 1;
   char wantMore = '\0';
   cout << num1 << " " << num2 << " ";

   do 
   {
      for (int counter = 0; counter < numsToCalculate; ++counter)
      {
         cout << num1 + num2 << " ";

         int num2Temp = num2;
         num2 = num1 + num2;
         num1 = num2Temp;
      }

      cout << endl << "Do you want more numbers (y/n)? ";
      cin >> wantMore;
   }while (wantMore == 'y');

   cout << "Goodbye!" << endl;

   return 0;
}