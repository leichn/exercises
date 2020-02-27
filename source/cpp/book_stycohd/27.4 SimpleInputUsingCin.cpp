#include<iostream>
using namespace std;

int main()
{
   cout << "Enter an integer: ";
   int inputNum = 0;
   cin >> inputNum;

   cout << "Enter the value of Pi: ";
   double Pi = 0.0;
   cin >> Pi;

   cout << "Enter three characters separated by space: " << endl;
   char char1 = '\0', char2 = '\0', char3 = '\0';
   cin >> char1 >> char2 >> char3;

   cout << "The recorded variable values are: " << endl;
   cout << "inputNum: " << inputNum << endl;
   cout << "Pi: " << Pi << endl;
   cout << "The three characters: " << char1 << char2 << char3 << endl;

   return 0;
}
