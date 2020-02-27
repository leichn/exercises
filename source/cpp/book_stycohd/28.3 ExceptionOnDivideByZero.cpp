#include<iostream>
using namespace std;

double Divide(double dividend, double divisor)
{
   if(divisor == 0)
      throw "Dividing by 0 is a crime";

   return (dividend / divisor);
}

int main()
{
   cout << "Enter dividend: ";
   double dividend = 0;
   cin >> dividend;
   cout << "Enter divisor: ";
   double divisor = 0;
   cin >> divisor;

   try
   {
      cout << "Result is: " << Divide(dividend, divisor);
   }
   catch(char* exp)
   {
      cout << "Exception: " << exp << endl;
      cout << "Sorry, can't continue!" << endl;
   }

   return 0;
}
