#include <iostream>
using namespace std;

void DisplayArray(int numbers[], int length)
{
   for (int index = 0; index < length; ++index)
      cout << numbers[index] << " ";

   cout << endl;
}

void DisplayArray(char characters[], int length)
{
   for (int index = 0; index < length; ++index)
      cout << characters[index] << " ";

   cout << endl;
}

int main()
{
   int myNumbers[4] = {24, 58, -1, 245};
   DisplayArray(myNumbers, 4);

   char myStatement[7] = {'H', 'e', 'l', 'l', 'o', '!', '\0'};
   DisplayArray(myStatement, 7);

   return 0;
}