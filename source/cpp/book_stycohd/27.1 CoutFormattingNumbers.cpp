#include <iostream>
#include <iomanip>
using namespace std;

int main()
{
   cout << "Enter an integer: ";
   int input = 0;
   cin >> input;

   cout << "Integer in octal: " << oct << input << endl;
   cout << "Integer in hexadecimal: " << hex << input << endl;

   cout << "Integer in hex using base notation: ";
   cout<<setiosflags(ios_base::hex|ios_base::showbase|ios_base::uppercase);
   cout << input << endl;

   cout << "Integer after resetting I/O flags: ";
   cout<<resetiosflags(ios_base::hex|ios_base::showbase|ios_base::uppercase);
   cout << input << endl;

   return 0;
}
