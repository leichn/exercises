#include <iostream>
using namespace std; 

class FishDummy1Member
{
public:
    FishDummy1Member() { cout << "Fish.m_dummy1 constructor" << endl; }
    ~FishDummy1Member() { cout << "Fish.m_dummy1 destructor" << endl; }
};

class FishDummy2Member
{
public:
    FishDummy2Member() { cout << "Fish.m_dummy2 constructor" << endl; }
    ~FishDummy2Member() { cout << "Fish.m_dummy2 destructor" << endl; }
};

class Fish
{
private:
    FishDummy1Member m_dummy1;
    FishDummy2Member m_dummy2;

public:
    Fish() { cout << "Fish constructor" << endl; }
    ~Fish() { cout << "Fish destructor" << endl; }
};

class TunaDummyMember
{
public:
    TunaDummyMember() { cout << "Tuna.m_dummy constructor" << endl; }
    ~TunaDummyMember() { cout << "Tuna.m_dummy destructor" << endl; }
};

class Tuna: public Fish
{
private:
    TunaDummyMember m_dummy;

public:
    Tuna() { cout << "Tuna constructor" << endl; }
    ~Tuna() { cout << "Tuna destructor" << endl; }
};

int main()
{
    Tuna tuna;
}