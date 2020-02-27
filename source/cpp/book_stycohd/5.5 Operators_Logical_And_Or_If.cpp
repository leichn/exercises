#include <iostream>
using namespace std;

int main()
{
   cout << "Use boolean values(0 / 1) to answer the questions" << endl;
   cout << "Is it raining? ";
   bool isRaining = false;
   cin >> isRaining;

   cout << "Do you have buses on the streets? ";
   bool busesPly = false;
   cin >> busesPly;

   // Conditional statement uses logical AND and NOT
   if (isRaining && !busesPly)
      cout << "You cannot go to work" << endl;
   else
      cout << "You can go to work" << endl;

   if (isRaining && busesPly)
      cout << "Take an umbrella" << endl;

   if ((!isRaining) && busesPly)
      cout << "Enjoy the sun and have a nice day" << endl;

   return 0;
}