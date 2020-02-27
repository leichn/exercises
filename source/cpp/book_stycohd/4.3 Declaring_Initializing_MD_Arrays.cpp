#include <iostream>
using namespace std;

int main()
{
   int threeRowsThreeColumns [3][3] = \
   {{-501, 205, 2011}, {989, 101, 206}, {303, 456, 596}}; 
   
   cout << "Row 0: " << threeRowsThreeColumns [0][0] << " " \
                     << threeRowsThreeColumns [0][1] << " " \
                     << threeRowsThreeColumns [0][2] << endl;


   cout << "Row 1: " << threeRowsThreeColumns [1][0] << " " \
                     << threeRowsThreeColumns [1][1] << " " \
                     << threeRowsThreeColumns [1][2] << endl;

   cout << "Row 2: " << threeRowsThreeColumns [2][0] << " "\
                     << threeRowsThreeColumns [2][1] << " " \
                     << threeRowsThreeColumns [2][2] << endl;

   return 0;
}