#include <vector>
#include <iostream>
#include <string>
#include <sstream>

using namespace std;

char DisplayOptions()
{
   cout << "What would you like to do?" << endl;
   cout << "Select 1: To enter length & breadth " << endl;
   cout << "Select 2: Query a value given an index" << endl;
   cout << "Select 3: To display dimensions of all packages" << endl;
   cout << "Select 4: To quit!" << endl << "> ";

   char ch;
   cin >> ch;

   return ch;
}

class Dimensions
{
   int length, breadth;
   string strOut;
public:
   Dimensions(int inL, int inB) : length(inL), breadth(inB) {}

   operator const char* ()
   {
      stringstream os;
      os << "Length "s << length << ", Breadth: "s << breadth << endl;
      strOut = os.str();
      return strOut.c_str();
   }
};

int main()
{
   vector <Dimensions> vecData;

   char chUserChoice = '\0';
   while ((chUserChoice = DisplayOptions()) != '4')
   {
      if (chUserChoice == '1')
      {
         cout << "Please enter length and breadth: " << endl;
         int length = 0, breadth = 0;
         cin >> length;
       cin >> breadth;

       vecData.push_back(Dimensions(length, breadth));
      }
      else if (chUserChoice == '2')
      {
         cout << "Please enter an index between 0 and ";
         cout << (vecData.size() - 1) << ": ";
         size_t index = 0;
         cin >> index;

         if (index < (vecData.size()))
         {
            cout << "Element [" << index << "] = " << vecData[index];
            cout << endl;
         }
      }
      else if (chUserChoice == '3')
      {
         cout << "The contents of the vector are: ";
         for (size_t index = 0; index < vecData.size(); ++index)
            cout << vecData[index] << ' ';
         cout << endl;
      }
   }
   return 0;
}
