#include <iostream>
using namespace std;

int main()
{
   const int NUM_ROWS = 3; 
   const int NUM_COLUMNS = 4;

   // 2D array of integers
   int MyInts[NUM_ROWS][NUM_COLUMNS] = { {34, -1, 879, 22}, 
                                      {24, 365, -101, -1}, 
	                                  {-20, 40, 90, 97} };

   // iterate rows, each array of int
   for (int row = 0; row < NUM_ROWS; ++row)
   {
	   // iterate integers in each row (columns)
      for (int column = 0; column < NUM_COLUMNS; ++column)
      {
         cout << "Integer[" << row << "][" << column \
             << "] = " << MyInts[row][column] << endl;
      }
   }

   return 0;
}