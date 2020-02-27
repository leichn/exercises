template <typename elementType>
class SortAscending
{
public:
   bool operator () (const elementType& num1,
      const elementType& num2) const
   {
      return (num1 < num2);
   }
};
   
#include<iostream>
#include<vector>
#include<algorithm>
int main()
{
   std::vector <int> numsInVec;

   // Insert sample numbers: 100, 90... 20, 10
   for (int sample = 10; sample > 0; --sample)
      numsInVec.push_back(sample * 10);

   std::sort(numsInVec.begin(), numsInVec.end(),
      SortAscending<int>());

   for (size_t index = 0; index < numsInVec.size(); ++index)
      cout << numsInVec[index] << ' ';

   return 0;
}
