#include <iostream>
#include <algorithm>
#include <vector>

int main()
{
   using namespace std;
   vector<int> numsInVec{ 2017, 0, -1, 42, 10101, 25 };

   cout << "Enter number to find in collection: ";
   int numToFind = 0;
   cin >> numToFind;

   auto element = find (numsInVec.cbegin (), // Start of range
                        numsInVec.cend (),   // End of range
                        numToFind);          // Element to find

   // Check if find succeeded
   if (element != numsInVec.cend ())
      cout << "Value " << *element << " found!" << endl;
   else
      cout << "No element contains value " << numToFind << endl;

   cout << "Finding the first even number using find_if: " << endl;

   auto evenNum = find_if (numsInVec.cbegin(), // Start of range
                           numsInVec.cend(),  // End of range
                 [](int element) { return (element % 2) == 0; } );

   if (evenNum != numsInVec.cend ())
   {
      cout << "Number '" << *evenNum << "' found at position [";
      cout << distance (numsInVec.cbegin (), evenNum) << "]" << endl;
   }

   return 0;
}