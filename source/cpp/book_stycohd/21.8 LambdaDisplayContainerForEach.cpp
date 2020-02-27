// this has been moved to Chapter 22, Listing 22.1


#include <algorithm>
#include <iostream>
#include <vector>
#include <list>

using namespace std;

int main ()
{
    vector <int> vecIntegers;

    for (int nCount = 0; nCount < 10; ++ nCount)
        vecIntegers.push_back (nCount);

    list <char> listChars;
    for (char nChar = 'a'; nChar < 'k'; ++nChar)
        listChars.push_back (nChar);

    cout << "Displaying the vector of integers: " << endl;

    // Display the array of integers
    for_each ( vecIntegers.begin ()    // Start of range
          , vecIntegers.end ()        // End of range
		  , [](int Num) {cout << Num << ' '; } ); // Unary function object

    cout << endl << endl;
    cout << "Displaying the list of characters: " << endl;

    // Display the list of characters
    for_each ( listChars.begin ()        // Start of range
          , listChars.end ()        // End of range
          , [](int Num) {cout << Num << ' '; } );// Unary function object

	cout << endl;

    return 0;
}
