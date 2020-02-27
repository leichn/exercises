#include <iostream>
using namespace std;

int main()
{
   int dogsAge = 30;
   cout << "Initialized dogsAge = " << dogsAge << endl;

   int* pointsToAnAge = &dogsAge;
   cout << "pointsToAnAge points to dogsAge" << endl;

   cout << "Enter an age for your dog: ";

   // store input at the memory pointed to by pointsToAnAge
   cin >> *pointsToAnAge;  

   // Displaying the address where age is stored
   cout << "Input stored at 0x" << hex << pointsToAnAge << endl;

   cout << "Integer dogsAge = " << dec << dogsAge << endl;

   return 0;
}