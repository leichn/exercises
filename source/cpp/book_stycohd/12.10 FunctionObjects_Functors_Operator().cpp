#include <iostream>
#include <string>
using namespace std;

class Display
{
public:
   void operator () (string input) const
   {
      cout << input << endl;
   }
};

int main ()
{
   Display displayFuncObj;

   // equivalent to displayFuncObj.operator () ("Display this string!");
   displayFuncObj ("Display this string!"); 

   return 0;
}