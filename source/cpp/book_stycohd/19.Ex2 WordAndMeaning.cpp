#include <set>
#include <iostream>
#include <string>

using namespace std;
   
struct PAIR_WORD_MEANING
{
   string word;
   string meaning;

   PAIR_WORD_MEANING(const string& sWord, const string& sMeaning)
      : word(sWord), meaning(sMeaning) {}

   bool operator< (const PAIR_WORD_MEANING& pairAnotherWord) const
   {
      return (word < pairAnotherWord.word);
   }

   bool operator== (const string& key)
   {
      return (key == this->word);
   }
};

int main()
{
   multiset <PAIR_WORD_MEANING> msetDictionary;
   PAIR_WORD_MEANING word1("C++", "A programming language");
   PAIR_WORD_MEANING word2("Programmer", "A geek!");

   msetDictionary.insert(word1);
   msetDictionary.insert(word2);

   cout << "Enter a word you wish to find the meaning off" << endl;
   string input;
   getline(cin, input);
   auto element = msetDictionary.find(PAIR_WORD_MEANING(input, ""));
   if (element != msetDictionary.end())
      cout << "Meaning is: " << (*element).meaning << endl;

   return 0;
}