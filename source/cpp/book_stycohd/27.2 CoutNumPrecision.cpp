#include <iostream>
#include <iomanip>
using namespace std;

int main()
{
   const double Pi = (double)22.0 / 7;
   cout << "Pi = " << Pi << endl;

   cout << endl << "Setting precision to 7: " << endl;
   cout << setprecision(7);
   cout << "Pi = " << Pi << endl;
   cout << fixed << "Fixed Pi = " << Pi << endl;
   cout << scientific << "Scientific Pi = " << Pi << endl;

   cout << endl << "Setting precision to 10: " << endl;
   cout << setprecision(10);
   cout << "Pi = " << Pi << endl;
   cout << fixed << "Fixed Pi = " << Pi << endl;
   cout << scientific << "Scientific Pi = " << Pi << endl;

   cout << endl << "Enter a radius: ";
   double radius = 0.0;
   cin >> radius;
   cout << "Area of circle: " << 2*Pi*radius*radius << endl;

   return 0;
}
