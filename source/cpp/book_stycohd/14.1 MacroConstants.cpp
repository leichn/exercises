#include <iostream>
#include<string>
using namespace std;

#define ARRAY_LENGTH 25
#define PI 3.1416
#define MY_DOUBLE double
#define FAV_WHISKY "Jack Daniels"

/*
// Superior alternatives (comment those above when you uncomment these)
const int ARRAY_LENGTH = 25;
const double PI = 3.1416;
typedef double MY_DOUBLE;
const char* FAV_WHISKY = "Jack Daniels";
*/

int main()
{
   int MyNumbers [ARRAY_LENGTH] = {0};
   cout << "Array's length: " << sizeof(MyNumbers) / sizeof(int) << endl;

   cout << "Enter a radius: ";
   MY_DOUBLE Radius = 0;
   cin >> Radius;
   cout << "Area is: " << PI * Radius * Radius << endl;

   string FavoriteWhisky (FAV_WHISKY);
   cout << "My favorite drink is: " << FAV_WHISKY << endl;

   return 0;
}