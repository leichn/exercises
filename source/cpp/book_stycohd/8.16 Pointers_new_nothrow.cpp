#include <iostream>
using namespace std;

int main()
{
   // Request LOTS of memory space, use nothrow 
   int* pointsToManyNums = new(nothrow) int [0x1fffffff];

   if (pointsToManyNums) // check pointsToManyNums != NULL
   {
      // Use the allocated memory 
      delete[] pointsToManyNums;
   }
   else 
      cout << "Memory allocation failed. Ending program" << endl;

   return 0;
}