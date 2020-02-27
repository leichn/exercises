#include <iostream>
#include<string>
using namespace std;

#define SQUARE(x) ((x) * (x))
#define PI 3.1416
#define AREA_CIRCLE(r) (PI*(r)*(r))
// #define AREA_CIRCLE(r) (PI*r*r) // uncomment this to understand why brackets are important (comment previous line)
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))


int main()
{
   cout << "Enter an integer: ";
   int num1 = 0;
   cin >> num1;

   cout << "SQUARE(" << num1 << ") = " << SQUARE(num1) << endl;
   cout << "Area of a circle with radius " << num1 << " is: ";
   cout << AREA_CIRCLE(num1) << endl;

   cout << "Enter another integer: ";
   int num2 = 0;
   cin >> num2;

   cout << "MIN(" << num1 << ", " << num2 << ") = ";
   cout << MIN (num1, num2) << endl;

   cout << "MAX(" << num1 << ", " << num2 << ") = ";
   cout << MAX (num1, num2) << endl;

   return 0;
}