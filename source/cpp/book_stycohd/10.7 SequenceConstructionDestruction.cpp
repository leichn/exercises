#include <iostream>
using namespace std; 

class FishDummyMember
{
public:
   FishDummyMember()
   {
      cout << "FishDummyMember constructor" << endl;
   }

   ~FishDummyMember()
   {
      cout << "FishDummyMember destructor" << endl;
   }
};

class Fish
{
protected:
   FishDummyMember dummy;

public:
   // Fish constructor
   Fish()
   {
      cout << "Fish constructor" << endl;
   }

   ~Fish()
   {
      cout << "Fish destructor" << endl;
   }
};

class TunaDummyMember
{
public:
   TunaDummyMember()
   {
      cout << "TunaDummyMember constructor" << endl;
   }

   ~TunaDummyMember()
   {
      cout << "TunaDummyMember destructor" << endl;
   }
};

class Tuna: public Fish
{
private:
   TunaDummyMember dummy;

public:
   Tuna()
   {
      cout << "Tuna constructor" << endl;
   }
   ~Tuna()
   {
      cout << "Tuna destructor" << endl;
   }

};
   
int main()
{
   Tuna myDinner;
}