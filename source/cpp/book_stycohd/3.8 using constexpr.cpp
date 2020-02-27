// 3.8 using constexpr.cpp : Defines the entry point for the console application.
//

#include <iostream>
constexpr double GetPi() { return 22.0 / 7; }
constexpr double TwicePi() { return 2 * GetPi(); }

int main()
{
   using namespace std;
   const double pi = 22.0 / 7;

   cout << "constant pi contains value " << pi << endl;
   cout << "constexpr GetPi() returns value " << GetPi() << endl;
   cout << "constexpr TwicePi() returns value " << TwicePi() << endl;

   return 0;
}