#include <iostream>
using namespace std; 

class Vehicle
{
public:
   Vehicle() { cout << "Consructor Vehicle" << endl; }
   ~Vehicle() { cout << "Destructor Vehicle" << endl; }
};
class Car : public Vehicle
{
public:
   Car() { cout << "Consructor Car" << endl; }
   ~Car() { cout << "Destructor Vehicle" << endl; }
};


int main()
{
   Vehicle* pMyRacer = new Car;
   delete pMyRacer;

   return 0;
}