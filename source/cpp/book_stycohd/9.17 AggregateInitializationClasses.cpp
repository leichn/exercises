#include <iostream>
#include<string>
using namespace std;

class Aggregate1
{
public:
   int num;
   double pi;
};

struct Aggregate2
{
   char hello[6];
   int impYears[3];
   string world;
};

int main()
{
   int myNums[] = { 9, 5, -1 }; // myNums is int[3]
   Aggregate1 a1{ 2017, 3.14 };
   cout << "Pi is approximately: " << a1.pi << endl;

   Aggregate2 a2{ {'h', 'e', 'l', 'l', 'o'}, {2011, 2014, 2017}, "world"};

   // Alternatively
   Aggregate2 a2_2{'h', 'e', 'l', 'l', 'o', '\0', 2011, 2014, 2017, "world"};

   cout << a2.hello << ' ' << a2.world << endl;
   cout << "C++ standard update scheduled in: " << a2.impYears[2] << endl;

   return 0;
}