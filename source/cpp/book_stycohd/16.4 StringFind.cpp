#include <string>
#include <iostream>

int main ()
{
   using namespace std;

   string sampleStr ("Good day String! Today is beautiful!");
   cout << "Sample string is:" << endl << sampleStr << endl << endl;

   // Find substring "day" - find() returns position
   size_t charPos = sampleStr.find ("day", 0);

   // Check if the substring was found...
   if (charPos != string::npos)
      cout << "First instance \"day\" at pos. " << charPos << endl;
   else
      cout << "Substring not found." << endl;

   cout << "Locating all instances of substring \"day\"" << endl;
   size_t subStrPos = sampleStr.find ("day", 0);

   while (subStrPos != string::npos)
   {
      cout << "\"day\" found at position " << subStrPos << endl;

      // Make find() search forward from the next character onwards
      size_t nSearchPosition = subStrPos + 1;

      subStrPos = sampleStr.find ("day", nSearchPosition);
   }

   return 0; 
}

/*
cout << endl;

cout << "Locating all instances of character 'a'" << endl;
const char charToSearch = 'a';
charPos = sampleStr.find(charToSearch, 0);

while (charPos != string::npos)
{
	cout << "'" << charToSearch << "' found";
	cout << " at position: " << charPos << endl;

	// Make the 'find' function search forward from the next character onwards
	size_t charSearchPos = charPos + 1;

	charPos = sampleStr.find(charToSearch, charSearchPos);
}
*/