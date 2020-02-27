#include <vector>
#include <iostream>
using namespace std;

/* 
// for older C++ compilers (non C++11 compliant)
void DisplayVector(const vector<int>& inVec)
{
   for (vector<int>::const_iterator element = inVec.begin ();	
        element != inVec.end ();
        ++ element )
      cout << *element << ' ';

   cout << endl;
}
*/

void DisplayVector(const vector<int>& inVec)
{
   for(auto element = inVec.cbegin();  // auto and cbegin() are C++11
       element != inVec.cend (); // cend() is new in C++11
       ++ element )
      cout << *element << ' ';

   cout << endl;
}

int main ()
{
   // Instantiate a vector with 4 elements, each initialized to 90
   vector <int> integers (4, 90);

   cout << "The initial contents of the vector: ";
   DisplayVector(integers);

   // Insert 25 at the beginning
   integers.insert (integers.begin (), 25);

   // Insert 2 numbers of value 45 at the end
   integers.insert (integers.end (), 2, 45);

   cout << "Vector after inserting elements at beginning and end: ";
   DisplayVector(integers);

   // Another vector containing 2 elements of value 30
   vector <int> vecAnother (2, 30);

   // Insert two elements from another container in position [1]
   integers.insert (integers.begin () + 1, 
      vecAnother.begin (), vecAnother.end ());

   cout << "Vector after inserting contents from another vector: ";
   cout << "in the middle:" << endl;
   DisplayVector(integers);

   return 0;
}