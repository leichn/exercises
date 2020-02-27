#include <vector>
#include <iostream>
#include <algorithm>

template <typename elementType>
class Multiply
{
public:
    elementType operator () (const elementType& elem1,
                             const elementType& elem2)
    {
        return (elem1 * elem2);
    }
};

int main ()
{
    using namespace std;

   vector <int> vecMultiplicand{ 0, 1, 2, 3, 4 };
   vector <int> vecMultiplier{ 100, 101, 102, 103, 104 };

    // A third container that holds the result of multiplication
    vector <int> vecResult;

    // Make space for the result of the multiplication
    vecResult.resize (vecMultiplier.size());
    transform (vecMultiplicand.begin (), // range of multiplicands
                vecMultiplicand.end (), // end of range
                vecMultiplier.begin (),  // multiplier values
                vecResult.begin (), // range that holds result
                Multiply <int> () );    // the function that multiplies

    cout << "The contents of the first vector are: " << endl;
    for (size_t index = 0; index < vecMultiplicand.size (); ++ index)
        cout << vecMultiplicand [index] << ' ';
    cout << endl;

    cout << "The contents of the second vector are: " << endl;
    for (size_t index = 0; index < vecMultiplier.size (); ++index)
        cout << vecMultiplier [index] << ' ';
    cout << endl;

    cout << "The result of the multiplication is: " << endl;
    for (size_t index = 0; index < vecResult.size (); ++ index)
        cout << vecResult [index] << ' ';

    return 0;
}