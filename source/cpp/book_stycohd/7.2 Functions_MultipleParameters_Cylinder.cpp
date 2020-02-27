#include <iostream>
using namespace std;

const double Pi = 3.14159265;

// Declaration contains two parameters
double SurfaceArea(double radius, double height);

int main()
{
   cout << "Enter the radius of the cylinder: ";
   double radius = 0;
   cin >> radius;
   cout << "Enter the height of the cylinder: ";
   double height = 0;
   cin >> height;

   cout << "Surface area: " << SurfaceArea(radius, height) << endl;

   return 0;
}

double SurfaceArea(double radius, double height)
{
   double area = 2 * Pi * radius * radius + 2 * Pi * radius * height;
   return area;
}