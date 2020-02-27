#include <vector>
#include <list>
#include <iostream>
   
using namespace std;

int main()
{
	vector <int> vecData{ 0, 10, 20, 30 };

   list <int> linkInts;

   // Insert contents of vector into beginning of list
   linkInts.insert(linkInts.begin(),
      vecData.begin(), vecData.end());

   cout << "The contents of the list are: ";

   list <int>::const_iterator element;
   for (element = linkInts.begin()
      ; element != linkInts.end()
      ; ++element)
      cout << *element << " ";

   return 0;
}