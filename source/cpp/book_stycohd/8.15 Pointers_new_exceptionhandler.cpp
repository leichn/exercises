#include <iostream>
using namespace std;

// remove the try-catch block to see this application crash 
int main()
{
   try
   {
      // Request a LOT of memory!
      int* pointsToManyNums = new int [0x1fffffff];

      // Use the allocated memory 

	  delete[] pointsToManyNums;
   }
   catch (bad_alloc)
   {
      cout << "Memory allocation failed. Ending program" << endl;
   }
   return 0;
}