#include <iostream>
#include <memory>
using namespace std;

class Fish
{
public:
    Fish() {cout << "Fish: Constructed!" << endl;}
    ~Fish() {cout << "Fish: Destructed!" << endl;}

    void Swim() const {cout << "Fish swims in water" << endl;}
};

void MakeFishSwim(const unique_ptr<Fish>& inFish)
{
   inFish->Swim();
}

int main()
{
   unique_ptr<Fish> smartFish (new Fish);

   smartFish->Swim();
   MakeFishSwim(smartFish); // OK, as MakeFishSwim accepts reference

   unique_ptr<Fish> copySmartFish;
   // copySmartFish = smartFish; // error: operator= is private

   unique_ptr<Fish> sameFish (std::move(smartFish)); 
   // smartFish is empty henceforth

   return 0;
}