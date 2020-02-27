#include <iostream>
using namespace std;

template <typename T1 = int, typename T2 = double>
class HoldsPair
{
private:
   T1 value1;
   T2 value2;
public:
   HoldsPair(const T1& val1, const T2& val2) // constructor
      : value1(val1), value2(val2) {}

   // Accessor functions
   const T1 & GetFirstValue() const;
   const T2& GetSecondValue() const;
};

// specialization of HoldsPair for types int & int here
template<> class HoldsPair<int, int>
{
private:
   int value1;
   int value2;
   string strFun;
public:
   HoldsPair(const int& val1, const int& val2) // constructor
      : value1(val1), value2(val2) {}

   const int & GetFirstValue() const
   {
      cout << "Returning integer " << value1 << endl;
      return value1;
   }
};
   
int main()
{
   HoldsPair<int, int> pairIntInt(222, 333);
   pairIntInt.GetFirstValue();

   return 0;
}