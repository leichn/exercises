#include <iostream>
#include <string.h>
using namespace std;

class MyString
{
private:
   char* buffer;

public:
   MyString() {}
   MyString(const char* initString) // constructor
   {
      buffer = NULL;
      cout << "Default constructor: creating new MyString" << endl;
      if(initString != NULL)
      {
         buffer = new char [strlen(initString) + 1];
         strcpy(buffer, initString);

         cout << "buffer points to: 0x" << hex;
         cout << (unsigned int*)buffer << endl;
      }
   }

   MyString(const MyString& copySource) // Copy constructor
   {
      buffer = NULL;
     cout << "Copy constructor: copying from MyString" << endl;
      if(copySource.buffer != NULL)
      {
         // allocate own buffer 
         buffer = new char [strlen(copySource.buffer) + 1];

         // deep copy from the source into local buffer
         strcpy(buffer, copySource.buffer);

         cout << "buffer points to: 0x" << hex;
         cout << (unsigned int*)buffer << endl;
      }
   }

   MyString operator+ (const MyString& addThis) 
   {
      MyString newString;

      if (addThis.buffer != NULL)
      {
         newString.buffer = new char[GetLength() + strlen(addThis.buffer) + 1];
         strcpy(newString.buffer, buffer);
         strcat(newString.buffer, addThis.buffer);
      }

      return newString;
   }

   // Destructor
   ~MyString()
   {
      cout << "Invoking destructor, clearing up" << endl;
      delete [] buffer;
   }

   int GetLength() 
   { return strlen(buffer); }

   const char* GetString()
   { return buffer; }
};

void UseMyString(MyString str)
{
   cout << "String buffer in MyString is " << str.GetLength();
   cout << " characters long" << endl;

   cout << "buffer contains: " << str.GetString() << endl;
   return;
}

int main()
{
   MyString sayHello("Hello from String Class");
   UseMyString(sayHello);

   return 0;
}