#include<map>
#include<iostream>
#include<string>
using namespace std;

template<typename T>
void DisplayContents(const T& cont)
{
	for (auto element = cont.cbegin();
         element != cont.cend();
         ++ element)
      cout<< element->first<< " -> "<< element->second<< endl;

   cout<< endl;
}

int main()
{
   multimap<int, string> mmapIntToStr;

   // Insert key-value pairs into the multimap
   mmapIntToStr.insert(make_pair(3, "Three"));
   mmapIntToStr.insert(make_pair(45, "Forty Five"));
   mmapIntToStr.insert(make_pair(-1, "Minus One"));
   mmapIntToStr.insert(make_pair(1000, "Thousand"));

   // Insert duplicates into the multimap
   mmapIntToStr.insert(make_pair(-1, "Minus One"));
   mmapIntToStr.insert(make_pair(1000, "Thousand"));

   cout<< "The multimap contains "<< mmapIntToStr.size();
   cout<< " key-value pairs. "<< "They are: "<< endl;
   DisplayContents(mmapIntToStr);

   // Erasing an element with key as -1 from the multimap
   auto numPairsErased = mmapIntToStr.erase(-1);
   cout<< "Erased " << numPairsErased << " pairs with -1 as key."<< endl;

   // Erase an element given an iterator from the multimap
   auto pair = mmapIntToStr.find(45);
   if(pair != mmapIntToStr.end())
   {
      mmapIntToStr.erase(pair);
      cout<< "Erased a pair with 45 as key using an iterator"<< endl;
   }

   // Erase a range from the multimap...
   cout << "Erasing the range of pairs with 1000 as key." << endl;
   mmapIntToStr.erase(mmapIntToStr.lower_bound(1000),
                      mmapIntToStr.upper_bound(1000) );

   cout<< "The multimap now contains "<< mmapIntToStr.size();
   cout<< " key-value pair(s)."<< "They are: "<< endl;
   DisplayContents(mmapIntToStr);

   return 0;
}