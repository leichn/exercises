#include <iostream>
using namespace std;

int main()
{
   cout << "Enter two integers:" << endl;
   int num1 = 0, num2 = 0;
   cin >> num1;
   cin >> num2;

   bool isEqual = (num1 == num2);
   cout << "Result of equality test: " << isEqual << endl;

   bool isUnequal = (num1 != num2);
   cout << "Result of inequality test: " << isUnequal << endl;

   bool isGreaterThan = (num1 > num2);
   cout << "Result of " << num1 << " > " << num2 << " test: " << isGreaterThan << endl;

   bool isLessThan = (num1 < num2);
   cout << "Result of " << num1 << " < " << num2 << " test: " << isLessThan << endl;

   bool isGreaterThanEquals = (num1 >= num2);
   cout << "Result of " << num1 << " >= " << num2 << " test: " << isGreaterThanEquals << endl;

   bool isLessThanEquals = (num1 <= num2);
   cout << "Result of " << num1 << " <= " << num2 << " test: " << isLessThanEquals << endl;

   return 0;
}