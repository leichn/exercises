#include <memory>
#include <iostream>
using namespace std;

class Fish
{
public:
   Fish() {cout << "Fish: Constructed!" << endl;}
   ~Fish() {cout << "Fish: Destructed!" << endl;}

   void Swim() const {cout << "Fish swims in water" << endl;}
};

class Carp: public Fish
{
};

void MakeFishSwim(const unique_ptr<Fish>& inFish)
{
   inFish->Swim();
}

int main ()
{
   unique_ptr<Fish> myCarp (new Carp);
   MakeFishSwim(myCarp);

   return 0;
}