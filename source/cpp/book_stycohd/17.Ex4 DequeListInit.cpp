#include <deque>
#include <string>
#include <iostream>
using namespace std;

template<typename T> 
void DisplayDeque(deque<T> inDQ)
{
   for (auto element = inDQ.cbegin(); 
   element != inDQ.cend();
      ++element)
      cout << *element << endl;
}

int main()
{
   deque<string> strDq{ "Hello"s, "Containers are cool"s, "C++ is evolving!"s };
   DisplayDeque(strDq);

   return 0;
}