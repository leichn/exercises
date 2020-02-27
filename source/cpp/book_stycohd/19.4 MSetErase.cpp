#include <set>
#include <iostream>
using namespace std;
   
template <typename T>
void DisplayContents (const T& Input)
{
   for (auto element = Input.cbegin(); 
        element != Input.cend ();
         ++ element )
      cout << *element << ' ';

   cout << endl;
}

typedef multiset <int> MSETINT;

int main ()
{
   MSETINT msetInts{ 43, 78, 78, -1, 124 };

   cout << "multiset contains " << msetInts.size () << " elements: ";
   DisplayContents(msetInts);

   cout << "Enter a number to erase from the set: ";
   int input = 0;
   cin >> input;

   cout << "Erasing " << msetInts.count (input);
   cout << " instances of value " << input << endl;

   msetInts.erase (input);

   cout << "multiset now contains " << msetInts.size () << " elements: ";
   DisplayContents(msetInts);

   return 0;
}