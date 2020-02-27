#include <string>
#include <algorithm>
#include <iostream>

int main ()
{
   using namespace std;

   string sampleStr ("Hello String! Wake up to a beautiful day!");
   cout << "The original sample string is: " << endl;
   cout << sampleStr << endl << endl;

   // Delete characters from the string given position and count
   cout << "Truncating the second sentence: " << endl;
   sampleStr.erase (13, 28);
   cout << sampleStr << endl << endl;

   // Find a character 'S' in the string using STL find algorithm
   auto iCharS = find (sampleStr.begin (),
                      sampleStr.end (), 'S');

   // If character found, 'erase' to deletes a character
   cout << "Erasing character 'S' from the sample string:" << endl;
   if (iCharS != sampleStr.end ())
      sampleStr.erase (iCharS);

   cout << sampleStr << endl << endl;

   // Erase a range of characters using an overloaded version of erase()
   cout << "Erasing a range between begin() and end(): " << endl;
   sampleStr.erase (sampleStr.begin (), sampleStr.end ());

   // Verify the length after the erase() operation above
   if (sampleStr.length () == 0)
      cout << "The string is empty" << endl;

   return 0;
}