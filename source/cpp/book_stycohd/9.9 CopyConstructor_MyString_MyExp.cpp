#include <iostream>
#include <string.h>
#include <stdio.h>

using namespace std;

class MyString
{
private:
    char *m_buffer;

public:
    MyString(const char *initString)        // Constructor
    {
        m_buffer = NULL;
        cout << "constructor" << endl;
        if (initString != NULL)
        {
            m_buffer = new char[strlen(initString) + 1];
            strcpy(m_buffer, initString);
        }
    }

    MyString(const MyString &copySource)    // Copy constructor
    {
        m_buffer = NULL;
        cout << "copy constructor" << endl;
        if (copySource.m_buffer != NULL)
        {
            m_buffer = new char[strlen(copySource.m_buffer) + 1];
            strcpy(m_buffer, copySource.m_buffer);
        }
    }

    ~MyString()                             // Destructor
    {
        cout << "destructor, delete m_buffer " << hex << (unsigned int *)m_buffer << dec << endl;
        delete[] m_buffer;
    }

    void PrintAddress(const string &prefix)
    {
        cout << prefix << hex << (unsigned int *)m_buffer << dec << endl;
    }
};

void UseMyString(MyString str1, MyString &str2)
{
    str1.PrintAddress("str1.m_buffer addr: ");
    str2.PrintAddress("str2.m_buffer addr: ");
}

int main()
{
    MyString test1("12345678901234567890");     // 直接初始化，执行构造函数 MyString(const char* initString)
    UseMyString(test1, test1);                  // 第一个参数是拷贝初始化，执行拷贝构造函数；第二个参数不会调用拷贝构造函数
    MyString test2 = test1;                     // 拷贝初始化，执行拷贝构造函数 MyString(const MyString& copySource)，深复制
    MyString test3("abcdefg");                  // 直接初始化，执行构造函数 MyString(const char* initString)
    test3 = test1;                              // 赋值，执行合成拷贝赋值运算符，因未显式定义赋值运算符，因此是浅复制
    test1.PrintAddress("test1.m_buffer addr: ");
    test2.PrintAddress("test2.m_buffer addr: ");
    test3.PrintAddress("test3.m_buffer addr: ");

    return 0;
}
