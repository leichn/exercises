#include <algorithm>
#include <iostream>
#include <vector>
#include <list>
using namespace std;

// A unary function
template <typename elementType>
void FuncDisplayElement(const elementType& element)
{
   cout << element << ' ';
};
   
// struct that behaves as a unary function
template <typename elementType>
struct DisplayElement
{
    void operator () (const elementType& element) const
    {
        cout << element << ' ';
    }
};

int main ()
{
   vector <int> numsInVec{ 0, 1, 2, 3, -1, -9, 0, -999 };
    cout << "Vector of integers contains: " << endl;

   for_each (numsInVec.begin (),    // Start of range
             numsInVec.end (),        // End of range
             DisplayElement<int> () ); // Unary function object

    // Display the list of characters
   list <char> charsInList{ 'a', 'z', 'k', 'd' };
   cout << endl << "List of characters contains: " << endl;

   for_each (charsInList.begin(),
             charsInList.end(),
	         FuncDisplayElement<char>);

    return 0;
}