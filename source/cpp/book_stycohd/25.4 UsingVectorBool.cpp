#include <vector>
#include <iostream>
using namespace std;

int main ()
{
   vector <bool> boolFlags{ true, true, false };  // instantiated to hold 3 bool flags
   boolFlags [0] = true;
   boolFlags [1] = true;
   boolFlags [2] = false;

   boolFlags.push_back (true); // insert a fourth bool at the end

   cout << "The contents of the vector are: " << endl;
   for (size_t îndex = 0; îndex < boolFlags.size (); ++ îndex)
      cout << boolFlags [îndex] << ' ';

   cout << endl;
   boolFlags.flip ();

   cout << "The contents of the vector are: " << endl;
   for (size_t îndex = 0; îndex < boolFlags.size (); ++ îndex)
      cout << boolFlags [îndex] << ' ';

   cout << endl;

   return 0;
}