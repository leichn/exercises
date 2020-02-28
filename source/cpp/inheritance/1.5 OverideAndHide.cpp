#include <iostream>
using namespace std; 

class Fish
{
private:
    bool isFreshWaterFish;

public:
    Fish(bool IsFreshWater) : isFreshWaterFish(IsFreshWater){}

    void Swim()                 // 1.1 此方法被派生类中的方法覆盖
    {
        if (isFreshWaterFish)
            cout << "Fish::Swim() Fish swims in lake" << endl;
        else
            cout << "Fish::Swim() Fish swims in sea" << endl;
    }

    void Swim(bool freshWater)  // 1.3 此方法被派生类中的方法隐藏，因为派生类中的覆盖函数隐藏了基类中 Swim 的所有重载版本
    {
        if (freshWater)
            cout << "Fish::Swim(bool) Fish swims in lake" << endl;
        else
            cout << "Fish::Swim(bool) Fish swims in sea" << endl;
    }

    void Fly()
    {
        cout << "Fish::Fly() Joke? A fish can fly?" << endl;
    }
};

class Tuna: public Fish
{
public:
    Tuna(): Fish(false) {}

    // using Fish::Swim;        // 4.2 解除对基类中所有 Swim() 重载版本的隐藏

    void Swim()                 // 1.2 覆盖基类中的方法
    {
        cout << "Tuna::Swim() Tuna swims real fast" << endl;
    }
};

class Carp: public Fish
{
public:
    Carp(): Fish(true) {}

    void Swim()                 // 1.2 覆盖基类中的方法
    {
        cout << "Carp::Swim() Carp swims real slow" << endl;
        Fish::Swim();           // 3.2 在派生类中调用基类方法(继承得到)
        Fish::Fly();            // 5.2 在派生类中调用基类方法(继承得到)
    }
    
    /*
    void Swim(bool freshWater)  // 4.3 覆盖基类中 Swim(bool) 方法
    {
        Fish::Swim(freshWater); // 4.3 调用基类中被覆盖的 Swim(bool) 方法
    }
    */
};

int main()
{
    Carp carp;
    Tuna tuna;

    carp.Swim();                // 2.1 调用派生类中的覆盖方法
    tuna.Swim();                // 2.2 调用派生类中的覆盖方法
    tuna.Fish::Swim();          // 3.1 调用基类中被覆盖的方法
    tuna.Fish::Swim(false);     // 4.1 调用基类中被隐藏的方法
    tuna.Fly();                 // 5.1 调用基类中的其他方法(继承得到)

    return 0;
}