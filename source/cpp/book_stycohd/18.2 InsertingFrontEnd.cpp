#include <list>
#include <iostream>
using namespace std;

template <typename T>
void DisplayContents (const T& container)
{
	for (auto element = container.cbegin();
        element != container.cend();
        ++ element )
       cout << *element << ' ';

   cout << endl;
}

int main ()
{
	std::list <int> linkInts{ -101, 42 };

   linkInts.push_front (10);
   linkInts.push_front (2011);
   linkInts.push_back (-1);
   linkInts.push_back (9999);

   DisplayContents(linkInts);

   return 0;
}


/*
// for non-C++11 compliant compilers
template <typename T>
void DisplayContents (const T& Input)
{
   for (T::const_iterator iElement = Input.begin () 
       ; iElement != Input.end ()
       ; ++ iElement )
       cout << *iElement << ' ';

   cout << endl;
}
*/

