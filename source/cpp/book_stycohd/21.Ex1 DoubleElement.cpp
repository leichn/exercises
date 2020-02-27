template <typename elementType = int>
struct Double
{
   void operator () (const elementType element) const
   {
      cout << element * 2 << ' ';
   }
};

#include<vector>
#include<iostream>
#include<algorithm>
using namespace std;

int main()
{
   vector <int> numsInVec;

   for (int count = 0; count < 10; ++count)
      numsInVec.push_back(count);

   cout << "Displaying the vector of integers: " << endl;

   // Display the array of integers
   for_each(numsInVec.begin(),  // Start of range
            numsInVec.end(), // End of range
            Double <>()); // Unary function object

   return 0;
}
