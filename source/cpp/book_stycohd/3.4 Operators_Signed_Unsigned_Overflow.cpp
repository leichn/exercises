#include <iostream>
using namespace std;

int main()
{
   unsigned short uShortValue = 65535;
   cout << "Incrementing unsigned short " << uShortValue << " gives: ";
   cout << ++uShortValue << endl;

   short signedShort = 32767;
   cout << "Incrementing signed short " << signedShort<< " gives: ";
   cout << ++signedShort << endl;

   return 0;
}