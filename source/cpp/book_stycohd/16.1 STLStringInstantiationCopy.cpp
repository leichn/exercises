#include <string>
#include <iostream>

int main ()
{
   using namespace std;
   const char* constCStyleString = "Hello String!";
   cout << "Constant string is: " << constCStyleString << endl;

   std::string strFromConst (constCStyleString);  // constructor
   cout << "strFromConst is: " << strFromConst << endl;

    std::string str2 ("Hello String!");
   std::string str2Copy (str2);
   cout << "str2Copy is: " << str2Copy << endl;
   
   // Initialize a string to the first 5 characters of another
   std::string strPartialCopy (constCStyleString, 5);
   cout << "strPartialCopy is: " << strPartialCopy << endl;

   // Initialize a string object to contain 10 'a's
   std::string strRepeatChars (10, 'a');
   cout << "strRepeatChars is: " << strRepeatChars << endl;

   return 0;
}
