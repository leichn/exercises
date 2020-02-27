// also contains postfix increment and decrement

#include <iostream>
using namespace std;

class Date
{
private:
   int day, month, year;

public:
   Date (int inMonth, int inDay, int inYear)
        : month (inMonth), day(inDay), year (inYear) {};

   Date& operator ++ () // prefix increment
   {
      ++day;
      return *this;
   }

   Date& operator -- () // prefix decrement
   {
      --day;
      return *this;
   }

   Date operator ++ (int) // postfix increment
   {
      Date copy(month, day, year);
      ++day;
      return copy;
   }

   Date operator -- (int) // postfix decrement
   {
      Date copy(month, day, year);
      --day;
      return copy;
   }

   void DisplayDate()
   {
      cout << month << " / " << day << " / " << year << endl;
   }
};

int main ()
{
   Date holiday (12, 25, 2016); // Dec 25, 2016

   cout << "The date object is initialized to: ";
   holiday.DisplayDate ();

   ++holiday; // move date ahead by a day
   cout << "Date after prefix-increment is: ";
   holiday.DisplayDate ();

   --holiday; // move date backwards by a day
   cout << "Date after a prefix-decrement is: ";
   holiday.DisplayDate ();

   return 0;
}