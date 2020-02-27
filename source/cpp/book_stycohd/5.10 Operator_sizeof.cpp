#include <iostream>
using namespace std;

int main()
{
   cout << "Use sizeof to determine memory used by arrays" << endl;
   int myNumbers [100] = {0};

   cout << "Bytes used by an int: " << sizeof(int) << endl;
   cout << "Bytes used by myNumbers: " << sizeof(myNumbers) << endl;
   cout << "Bytes used by an element: " << sizeof(myNumbers[0]) << endl;

   return 0;
}
