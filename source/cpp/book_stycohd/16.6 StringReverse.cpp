#include <string>
#include <iostream>
#include <algorithm>

int main ()
{
   using namespace std;

   string sampleStr ("Hello String! We will reverse you!");
   cout << "The original sample string is: " << endl;
   cout << sampleStr << endl << endl;

   reverse (sampleStr.begin (), sampleStr.end ());

   cout << "After applying the std::reverse algorithm: " << endl;
   cout << sampleStr << endl;

   return 0;
}