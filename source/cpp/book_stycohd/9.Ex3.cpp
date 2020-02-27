#include <iostream>
#include <string>
using namespace std;

class Human
{
   int Age;
   string Name;

public: 
   Human(string InputName, int InputAge) 
	    : Name(InputName), Age(InputAge) {}
};

int main()
{
	Human FirstMan("Adam", 24);
	return 0;
}