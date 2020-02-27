#include <list>
#include <iostream>
using namespace std;

template <typename T>
void DisplayContents(const T& container)
{
   for(auto element = container.cbegin();
       element != container.cend();
       ++ element )
     cout << *element << ' ';

   cout << endl;
}

int main()
{
	std::list <int> linkInts{ 4, 3, 5, -1, 2017 };

   // Store an iterator obtained in using insert()
   auto iValue2 = linkInts.insert(linkInts.begin(), 2);

   cout << "Initial contents of the list:" << endl;
   DisplayContents(linkInts);

   cout << "After erasing element '"<< *iValue2 << "':" << endl;
   linkInts.erase(iValue2);
   DisplayContents(linkInts);

   linkInts.erase(linkInts.begin(), linkInts.end());
   cout << "Number of elements after erasing range: ";
   cout << linkInts.size() << endl;

   linkInts.clear();

   return 0;
}