#include <iostream>
using namespace std;

int main()
{
   int age = 30;

   int* pointsToInt = &age;
   cout << "pointsToInt points to age now" << endl;

   // Displaying the value of pointer
   cout << "pointsToInt = 0x" << hex << pointsToInt << endl;

   int DogsAge = 9;
   pointsToInt = &DogsAge;
   cout << "pointsToInt points to DogsAge now" << endl;

   cout << "pointsToInt = 0x" << hex << pointsToInt << endl;

   return 0;
}