#include <iostream>
using namespace std; 
  
class Fish
{
public:
   virtual void Swim()
   {
      cout << "Fish swims in water" << endl;
   }

   // base class should always have virtual destructor
   virtual ~Fish() {}   
};

class Tuna: public Fish
{
public:
   void Swim()
   {
      cout << "Tuna swims real fast in the sea" << endl;
   }

   void BecomeDinner()
   {
      cout << "Tuna became dinner in Sushi" << endl;
   }
};

class Carp: public Fish
{
public:
   void Swim()
   {
      cout << "Carp swims real slow in the lake" << endl;
   }

   void Talk()
   {
      cout << "Carp talked crap" << endl;
   }
};

void DetectFishType(Fish* InputFish)
{
   Tuna* pIsTuna = dynamic_cast <Tuna*>(InputFish);
   if (pIsTuna)
   {
      cout << "Detected Tuna. Making Tuna dinner: " << endl;
      pIsTuna->BecomeDinner();   // calling Tuna::BecomeDinner
   }

   Carp* pIsCarp = dynamic_cast <Carp*>(InputFish);
   if(pIsCarp)
   {
      cout << "Detected Carp. Making carp talk: " << endl;
      pIsCarp->Talk();  // calling Carp::Talk
   }

   cout << "Verifying type using virtual Fish::Swim: " << endl;
   InputFish->Swim(); // calling virtual function Swim
}

int main()
{
   Fish* pFish = new Tuna;
   Tuna* pTuna = static_cast<Tuna*>(pFish);

   // Tuna::BecomeDinner will work only using valid Tuna* 
   pTuna->BecomeDinner();   
   
   // virtual destructor in Fish ensures invocation of ~Tuna()
   delete pFish;

   return 0;
}