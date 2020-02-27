#include<iostream>
#include<algorithm>
#include<vector>
using namespace std;

template <typename T>
void DisplayContents(const T& container)
{
	for (auto element = container.cbegin();
         element != container.cend();
         ++element)
      cout << *element << ' ';
   cout << endl;
}

int main()
{
	vector<int> vecNumbers{ 25, -5, 122, 2011, -10001 };
   DisplayContents(vecNumbers);

   sort(vecNumbers.begin(), vecNumbers.end());
   DisplayContents(vecNumbers);

   sort(vecNumbers.begin(), vecNumbers.end(),
      [](int Num1, int Num2) {return (Num1 > Num2); });
   DisplayContents(vecNumbers);

   cout << "Number do you wish to add to all elements: ";
   int numcontainer = 0;
   cin >> numcontainer;

   for_each(vecNumbers.begin(), vecNumbers.end(),
      [=](int& element) {element += numcontainer; });

   DisplayContents(vecNumbers);

   return 0;
}