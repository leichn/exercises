#include<iostream>
#include<algorithm>
#include<vector>
using namespace std;

template <typename T>
void DisplayContents (const T& Input)
{
   for(auto iElement = Input.cbegin() // auto, cbegin and cend: c++11 
      ; iElement != Input.cend ()
      ; ++ iElement )
      cout << *iElement << ' ';
   cout << endl;
}

int main() 
{
   vector<int> vecNumbers;
   vecNumbers.push_back(25);
   vecNumbers.push_back(-5);
   vecNumbers.push_back(122);
   vecNumbers.push_back(2011);
   vecNumbers.push_back(-10001);
   DisplayContents(vecNumbers);

   sort(vecNumbers.begin(), vecNumbers.end());
   DisplayContents(vecNumbers);

   sort(vecNumbers.begin(), vecNumbers.end(),
      [](int Num1, int Num2) {return (Num1 > Num2); } );
   DisplayContents(vecNumbers);

   cout << "Number do you wish to add to all elements: ";
   int NumInput = 0;
   cin >> NumInput;

   for_each(vecNumbers.begin(), vecNumbers.end(), 
      [NumInput](int& element) {element += NumInput;} );

   DisplayContents(vecNumbers);

   return 0;
}