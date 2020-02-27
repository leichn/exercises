#include <string>
#include <iostream>

int main ()
{
   using namespace std;

   string stlString ("Hello String"); // sample

    // Access the contents of the string using array syntax
   cout << "Display elements in string using array-syntax: " << endl;
   for (size_t charCounter = 0;
        charCounter < stlString.length ();
        ++ charCounter)
    {
       cout << "Character [" << charCounter << "] is: ";
       cout << stlString [charCounter] << endl;
    }
   cout << endl;

    // Access the contents of a string using iterators
   cout << "Display elements in string using iterators: " << endl; 
   int charOffset = 0;

   for (auto charLocator = stlString.cbegin();
        charLocator != stlString.cend ();
        ++ charLocator)
   {
      cout << "Character [" << charOffset ++ << "] is: ";
      cout << *charLocator << endl;
   }
   cout << endl;

   // Access contents as a const char*
   cout << "The char* representation of the string is: ";
   cout << stlString.c_str () << endl;

   return 0;
}