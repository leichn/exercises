#include <iostream>
using namespace std;

class Fish
{
public:
   void Swim()
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

void MakeFishSwim(Fish& InputFish)
{
   // calling Fish::Swim
   InputFish.Swim();
}

int main() 
{
   Tuna myDinner;

   // calling Tuna::Swim
   myDinner.Swim();

   // sending Tuna as Fish
   MakeFishSwim(myDinner);

   return 0;
}