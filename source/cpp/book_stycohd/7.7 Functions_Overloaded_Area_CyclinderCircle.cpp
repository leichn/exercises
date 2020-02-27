#include <iostream>
using namespace std;

const double Pi = 3.14159265;

double Area(double radius);   // for circle
double Area(double radius, double height); // overloaded for cylinder

int main() 
{
   cout << "Enter z for Cylinder, c for Circle: ";
   char userSelection = 'z';
   cin >> userSelection;

   cout << "Enter radius: ";
   double radius = 0;
   cin >> radius;

   if (userSelection == 'z')
   {
      cout << "Enter height: ";
      double height = 0;
      cin >> height;

      // Invoke overloaded variant of Area for Cyclinder
      cout << "Area of cylinder is: " << Area (radius, height) << endl;
   }
   else
      cout << "Area of circle is: " << Area (radius) << endl;

   return 0;
}

// for circle
double Area(double radius) 
{
   return Pi * radius * radius;
}

// overloaded for cylinder
double Area(double radius, double height)
{
   // reuse the area of circle 
   return 2 * Area (radius) + 2 * Pi * radius * height;
}