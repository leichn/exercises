#include<iostream>
using namespace std;

class Shape
{
public:
   virtual double Area() = 0;
   virtual void Print() = 0;
};

class Circle: public Shape
{
   double Radius;
public:
   Circle(double inputRadius) : Radius(inputRadius) {}

   double Area() override
   {
      return 3.1415 * Radius * Radius;
   }
   
   void Print() override
   {
      cout << "Circle says hello!" << endl;
   }
};

class Triangle: public Shape
{
   double Base, Height;
public:
   Triangle(double inputBase, double inputHeight) : Base(inputBase), Height(inputHeight) {}

   double Area() override
   {
      return 0.5 * Base * Height;
   }
   
   void Print() override
   {
      cout << "Triangle says hello!" << endl;
   }
};

int main()
{
   Circle myRing(5);
   Triangle myWarningTriangle(6.6, 2);

   cout << "Area of circle: " << myRing.Area() << endl; 
   cout << "Area of triangle: " << myWarningTriangle.Area() << endl; 

   myRing.Print();
   myWarningTriangle.Print();

   return 0;
}