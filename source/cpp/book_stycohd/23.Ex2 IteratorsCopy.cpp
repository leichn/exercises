#include <vector>
#include <algorithm>
#include <list>
#include <string>
#include <iostream>
   
using namespace std;

int main()
{
   list <string> listNames;
   listNames.push_back("Jack");
   listNames.push_back("John");
   listNames.push_back("Anna");
   listNames.push_back("Skate");

   vector <string> vecNames(4);
   copy(listNames.begin(), listNames.end(), vecNames.begin());

   vector <string> ::const_iterator iNames;
   for (iNames = vecNames.begin(); iNames != vecNames.end(); ++iNames)
      cout << *iNames << ' ';

   return 0;
}