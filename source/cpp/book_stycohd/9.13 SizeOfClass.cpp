#include <iostream>
#include <string.h>
using namespace std;
   
class MyString
{
private:
   char* buffer;

public:
   MyString(const char* initString) // default constructor
   {
      buffer = NULL;
	  if(initString != NULL)
      {
         buffer = new char [strlen(initString) + 1];
         strcpy(buffer, initString);
      }
   }

   MyString(const MyString& copySource) // copy constructor
   {
      buffer = NULL;
	  if(copySource.buffer != NULL)
      {
          buffer = new char [strlen(copySource.buffer) + 1];
          strcpy(buffer, copySource.buffer);
      }
   }

   ~MyString()
   {
      delete [] buffer;
   }

   int GetLength() 
   { return strlen(buffer); }

   const char* GetString()
   { return buffer; }
};

class Human
{
private:
   int age;
   bool gender;
   MyString name;

public:
   Human(const MyString& InputName, int InputAge, bool InputGender)
      : name(InputName), age (InputAge), gender(InputGender) {}

   int GetAge ()
   { return age; }
};

int main()
{
   MyString mansName("Adam");
   MyString womansName("Eve");

   cout << "sizeof(MyString) = " << sizeof(MyString) << endl;
   cout << "sizeof(mansName) = " << sizeof(mansName) << endl;
   cout << "sizeof(womansName) = " << sizeof(womansName) << endl;

   Human firstMan(mansName, 25, true);
   Human firstWoman(womansName, 18, false);

   cout << "sizeof(Human) = " << sizeof(Human) << endl;
   cout << "sizeof(firstMan) = " << sizeof(firstMan) << endl;
   cout << "sizeof(firstWoman) = " << sizeof(firstWoman) << endl;

   return 0;
}