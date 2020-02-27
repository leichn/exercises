#include <iostream>
using namespace std; 

class Fish
{
protected:
   bool isFreshWaterFish; // accessible only to derived classes

public:
  void Swim()
   {
      if (isFreshWaterFish)
         cout << "Swims in lake" << endl;
      else
         cout << "Swims in sea" << endl;
   }
};

class Tuna: public Fish
{
public:
   Tuna()
   {
      isFreshWaterFish = false; // set base class protected member
   }
};

class Carp: public Fish
{
public:
   Carp()
   {
      isFreshWaterFish = false;
   }
};

int main()
{
   Carp myLunch;
   Tuna myDinner;

   cout << "Getting my food to swim" << endl;

   cout << "Lunch: ";
   myLunch.Swim();

   cout << "Dinner: ";
   myDinner.Swim();

   // uncomment line below to see that protected members
   // are not accessible from outside the class heirarchy
   // myLunch.isFreshWaterFish = false;

   return 0;
}