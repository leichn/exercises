#include <bitset>
#include <string>
#include <iostream>
   
int main ()
{
   using namespace std;
   bitset <8> inputBits;
   cout << "Enter a 8-bit sequence: ";

   cin >> inputBits;  // store user input in bitset

   cout << "Num 1s you supplied: " << inputBits.count () << endl;
   cout << "Num 0s you supplied: ";
   cout << inputBits.size () - inputBits.count () << endl;

   bitset <8> inputFlipped (inputBits);  // copy
   inputFlipped.flip ();  // toggle the bits

   cout << "Flipped version is: " << inputFlipped << endl;

   cout << "Result of AND, OR and XOR between the two:" << endl;
   cout << inputBits << " & " << inputFlipped << " = ";
   cout << (inputBits & inputFlipped) << endl;  // bitwise AND

   cout << inputBits << " | " << inputFlipped << " = ";
   cout << (inputBits | inputFlipped) << endl;  // bitwise OR

   cout << inputBits << " ^ " << inputFlipped << " = ";
   cout << (inputBits ^ inputFlipped) << endl; // bitwise XOR

   return 0;
}