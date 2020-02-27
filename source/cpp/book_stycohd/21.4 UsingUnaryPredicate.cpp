// A structure as a unary predicate
template <typename numberType>
struct IsMultiple
{
   numberType Divisor;

   IsMultiple (const numberType& divisor)
   {
      Divisor = divisor;
   }

   bool operator () (const numberType& element) const
   {
      // Check if the dividend is a multiple of the divisor
      return ((element % Divisor) == 0);
   }
};

#include <algorithm>
#include <vector>
#include <iostream>
using namespace std;


int main ()
{
   vector <int> numsInVec{ 25, 26, 27, 28, 29, 30, 31 };
   cout << "The vector contains: 25, 26, 27, 28, 29, 30, 31" << endl;

   cout << "Enter divisor (> 0): ";
   int divisor = 2;
   cin >> divisor;

   // Find the first element that is a multiple of 4 in the collection
   auto element = find_if (numsInVec.begin (),
                           numsInVec.end (),
                           IsMultiple<int>(divisor) );

   if (element != numsInVec.end ())
   {
      cout << "First element in vector divisible by " << divisor;
      cout << ": " << *element << endl;
    }

   return 0;
}