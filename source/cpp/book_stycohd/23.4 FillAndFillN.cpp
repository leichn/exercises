#include <algorithm>
#include <vector>
#include <iostream>

int main ()
{
    using namespace std;

    // Initialize a sample vector with 3 elements
    vector <int> numsInVec (3);

    // fill all elements in the container with value 9
    fill (numsInVec.begin (), numsInVec.end (), 9);

    // Increase the size of the vector to hold 6 elements
    numsInVec.resize (6);

    // Fill the three elements starting at offset position 3 with value -9
    fill_n (numsInVec.begin () + 3, 3, -9);

    cout << "Contents of the vector are: " << endl;
    for (size_t index = 0; index < numsInVec.size (); ++ index)
    {
        cout << "Element [" << index << "] = ";
        cout << numsInVec [index] << endl;
    }

    return 0;
}
