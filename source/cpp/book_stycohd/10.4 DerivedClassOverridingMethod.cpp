#include <iostream>
using namespace std; 

class Fish
{
private:
   bool isFreshWaterFish;

public:
   // Fish constructor
   Fish(bool IsFreshWater) : isFreshWaterFish(IsFreshWater){}

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
   Tuna(): Fish(false) {}

   void Swim()
   {
      cout << "Tuna swims real fast" << endl;
   }
};

class Carp: public Fish
{
public:
   Carp(): Fish(true) {}

   void Swim()
   {
      cout << "Carp swims real slow" << endl;
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

   return 0;
}