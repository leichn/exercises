#include <list>
#include <string>
#include <iostream>

using namespace std;
   
int main()
{
   list <string> names;
   names.push_back("Jack");
   names.push_back("John");
   names.push_back("Anna");
   names.push_back("Skate");

   cout << "The contents of the list are: ";

   list <string>::const_iterator element;
   for (element = names.begin(); element != names.end(); ++element)
      cout << *element << " ";
   cout << endl;

   cout << "The contents after reversing are: ";
   names.reverse();
   for (element = names.begin(); element != names.end(); ++element)
      cout << *element << " ";
   cout << endl;

   cout << "The contents after sorting are: ";
   names.sort();
   for (element = names.begin(); element != names.end(); ++element)
      cout << *element << " ";
   cout << endl;

   return 0;
}