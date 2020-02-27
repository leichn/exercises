#include <iostream>
using namespace std;

const double Pi = 3.1416;

// output parameter result by reference 
void Area(double radius, double& result)
{
   result = Pi * radius * radius;
}

int main() 
{
   cout << "Enter radius: ";
   double radius = 0;
   cin >> radius;

   double areaFetched = 0;
   Area(radius, areaFetched);

   cout << "The area is: " << areaFetched << endl;
   return 0;
}