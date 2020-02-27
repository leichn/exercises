#include <iostream>
using namespace std;

class Fish
{
public:
   Fish()
   {
      cout << "Constructed Fish" << endl;
   }
   virtual ~Fish()   // virtual destructor!
   {
      cout << "Destroyed Fish" << endl;
   }
};

class Tuna:public Fish
{
public:
   Tuna()
   {
      cout << "Constructed Tuna" << endl;
   }
   ~Tuna()
   {
      cout << "Destroyed Tuna" << endl;
   }
};

void DeleteFishMemory(Fish* pFish)
{
   delete pFish;
}

int main() 
{
   cout << "Allocating a Tuna on the free store:" << endl;
   Tuna* pTuna = new Tuna;
   cout << "Deleting the Tuna: " << endl;
   DeleteFishMemory(pTuna);

   cout << "Instantiating a Tuna on the stack:" << endl;
   Tuna myDinner;
   cout << "Automatic destruction as it goes out of scope: " << endl;

   return 0;
}