#include <iostream>
using namespace std;

class Motor
{
public:
   void SwitchIgnition()
   {
      cout << "Ignition ON" << endl;
   }
   void PumpFuel()
   {
      cout << "Fuel in cylinders" << endl;
   }
   void FireCylinders()
   {
      cout << "Vroooom" << endl;
   }
};

class Car:private Motor
{
public:
   void Move()
   {
      SwitchIgnition();
      PumpFuel();
      FireCylinders();
   }
};

int main()
{
   Car myDreamCar;
   myDreamCar.Move();

   return 0;
}