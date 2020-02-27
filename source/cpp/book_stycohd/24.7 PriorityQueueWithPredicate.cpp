#include <queue>
#include <iostream>
#include <functional>

int main ()
{
   using namespace std;

   // Define a priority_queue object with greater <int> as predicate
   priority_queue <int, vector <int>, greater <int> > numsInPrioQ;

   cout << "Inserting {10, 5, -1, 20} into the priority queue" << endl;
   numsInPrioQ.push (10);
   numsInPrioQ.push (5);
   numsInPrioQ.push (-1);
   numsInPrioQ.push (20);

   cout << "Deleting " << numsInPrioQ.size () << " elements" << endl;
   while (!numsInPrioQ.empty ())
   {
      cout << "Deleting topmost element " << numsInPrioQ.top () << endl;
      numsInPrioQ.pop ();
   }

   return 0;
}