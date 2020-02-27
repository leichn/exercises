#include <vector>

int main ()
{
    using namespace std;

    // Instantiate an object using the default constructor
    vector <bool> boolFlags1;

    // Initialize a vector with 10 elements with value true
    vector <bool> boolFlags2 (10, true);

    // Instantiate one object as a copy of another
    vector <bool> boolFlags2Copy (boolFlags2);

    return 0;
}
