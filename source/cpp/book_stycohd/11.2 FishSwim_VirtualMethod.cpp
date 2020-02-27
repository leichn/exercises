#include <iostream>
using namespace std;

class Fish
{
public:
   virtual void Swim()
   {
      cout << "Fish swims!" << endl;
   }
};

class Tuna:public Fish
{
public:
   // override Fish::Swim
   void Swim()
   {
      cout << "Tuna swims!" << endl;
   }
};

class Carp:public Fish
{
public:
   // override Fish::Swim
   void Swim()
   {
      cout << "Carp swims!" << endl;
   }
};

void MakeFishSwim(Fish& InputFish)
{
   // calling Swim
   InputFish.Swim();
}

int main() 
{
   Tuna myDinner;
   Carp myLunch;

   // sending Tuna as Fish
   MakeFishSwim(myDinner);

   // sending Carp as Fish
   MakeFishSwim(myLunch);

   return 0;
}