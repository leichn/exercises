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
   // 引用形式
   Fish myFish;
   Tuna myTuna;
   Carp myCarp;
   Fish &rFish = myFish;
   Fish &rTuna = myTuna;
   Fish &rCarp = myCarp;
   rFish.Swim();
   rTuna.Swim();
   rCarp.Swim();

   // 指针形式
   Fish *pFish = new Fish();
   Fish *pTuna = new Tuna();
   Fish *pCarp = new Carp();
   pFish->Swim();
   pTuna->Swim();
   pCarp->Swim();

   return 0;
}
