#include <iostream>

int main()
{
   using namespace std;

   const double pi = 22.0 / 7;
   cout << "The value of constant pi is: " << pi << endl;

   // Uncomment next line to fail compilation
   pi = 3.14; 

   return 0;
}