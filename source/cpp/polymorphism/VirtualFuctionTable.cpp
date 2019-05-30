#include <iostream>
using namespace std;

class Fish
{
public:
   virtual void Swim() { cout << "Fish swims!" << endl; }
};

class Tuna : public Fish
{
public:
   void Swim() { cout << "Tuna swims!" << endl; }
};

class Carp:public Fish
{
public:
   void Swim() { cout << "Carp swims!" << endl; }
};
int main() 
{
   Fish *pFish = NULL;

   pFish = new Fish();
   pFish->Swim();

   pFish = new Tuna();
   pFish->Swim();

   pFish = new Carp();
   pFish->Swim();

   return 0;
}