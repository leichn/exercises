#include <algorithm>
#include <vector>
#include <string>
#include <iostream>
using namespace std;

template <typename T>
void DisplayContents(const T& container)
{
   for (auto element = container.cbegin();
        element != container.cend();
        ++ element)
      cout << *element << endl;
}

int main ()
{
   vector<string> vecNames{"John", "jack", "sean", "Anna"};

   // insert a duplicate 
   vecNames.push_back ("jack");

   cout << "The initial contents of the vector are: " << endl;
   DisplayContents(vecNames);

   cout << "The sorted vector contains names in the order:" << endl;
   sort (vecNames.begin (), vecNames.end ());
   DisplayContents(vecNames);

   cout << "Searching for \"John\" using 'binary_search':" << endl;
   bool elementFound = binary_search (vecNames.begin (), vecNames.end (),
                                       "John");

   if (elementFound)
      cout << "Result: \"John\" was found in the vector!" << endl;
   else
      cout << "Element not found " << endl;

   // Erase adjacent duplicates
   auto newEnd = unique (vecNames.begin (), vecNames.end ());
   vecNames.erase (newEnd, vecNames.end ());

   cout << "The contents of the vector after using 'unique':" << endl;
   DisplayContents(vecNames);

   return 0;
}