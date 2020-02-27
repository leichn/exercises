#include <iostream>
#include <vector>
using namespace std;

template <typename T>
void DisplayVector(const vector<T>& inVec)
{
   for(auto element = inVec.cbegin();  // auto & cbegin() are C++11
       element != inVec.cend (); // cend() is C++11
       ++ element )
     cout << *element << ' ';

   cout << endl;
}

int main ()
{
   vector<int> integers;

   // Insert sample integers into the vector:
   integers.push_back (50);
   integers.push_back (1);
   integers.push_back (987);
   integers.push_back (1001);

   cout << "Vector contains " << integers.size () << " elements: ";
   DisplayVector(integers);

   // Erase one element at the end
   integers.pop_back ();

	cout << "After a call to pop_back()" << endl;
   cout << "Vector contains " << integers.size () << " elements: ";
   DisplayVector(integers);

   return 0;
}