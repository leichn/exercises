#include <set>
#include <iostream>
using namespace std;
   
int main ()
{
   set<int> setInts{ 43, 78, -1, 124 };

   // Display contents of the set to the screen
   for (auto element = setInts.cbegin();
        element != setInts.cend ();
        ++ element )
      cout << *element << endl;

   // Try finding an element
   auto elementFound = setInts.find (-1);

   // Check if found...
   if (elementFound != setInts.end ())
      cout << "Element " << *elementFound << " found!" << endl;
   else
      cout << "Element not found in set!" << endl;

   // finding another 
   auto anotherFind = setInts.find (12345);

   // Check if found...
   if (anotherFind != setInts.end ())
      cout << "Element " << *anotherFind << " found!" << endl;
   else
      cout << "Element 12345 not found in set!" << endl;

   return 0;
}