#include <vector>
   
int main ()
{
   // vector of integers
   std::vector<int> integers;

   // vector with 3 elements initialized using C++11 list initialization
   std::vector<int> initVector{ 202, 2017, -1 };

   // Instantiate a vector with 10 elements (it can still grow)
   std::vector<int> tenElements (10);

   // Instantiate a vector with 10 elements, each initialized to 90
   std::vector<int> tenElemInit (10, 90);

   // Instantiate one vector and initialize it to the contents of another
   std::vector<int> copyVector (tenElemInit);

   // Vector initialized to 5 elements from another using iterators
   std::vector<int> partialCopy (tenElements.cbegin(),
                                 tenElements.cbegin() + 5);

   return 0;
}