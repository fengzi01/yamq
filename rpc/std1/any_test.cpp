#include "my_any.h"
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <vector>
using std::vector;

class A {
    public:
        A():_val(nullptr) {
            fprintf(stderr,"default constructor! val = %p\n",_val);
        }
        A(int a):_val(new int(a)) {
            fprintf(stderr,"constructor! val = %d\n",*_val);
        }

        ~A() {
//            fprintf(stderr,"destroctor! val = %p\n",_val);
            delete _val;
        }
        A(const A &other):_val(new int(*other._val)) {
            fprintf(stderr,"copy constructor val = %d\n",*_val); 
        }
        A & operator=(const A &other) {
            fprintf(stderr,"copy operator val = %d\n",*other._val);
            int *tmp = new int(*other._val);
            delete _val;
            _val = tmp;
            return *this;
        }

        int GetVal() {return *_val;}

        // 定义移动构造函数
        A(A &&other):_val(other._val) {
            fprintf(stderr,"move constuctor val = %d\n",*_val);
            other._val = nullptr;
        }

        A & operator=(A &&other) {
            fprintf(stderr,"move operator val = %d\n",*other._val);
            if (this != &other) {
                _val = other._val;
                other._val = nullptr;
            }
            return *this;
        }
    private:
        int *_val;
};

A GetA(int val) { return A(val); }

int main() {
    A c(20); // 构造函数
    A d = 90; // 构造函数
    A f = c; // 复制构造函数
    A e = std::move(c); // 移动构造函数
    A x,y,z;
    y = e; // 赋值运算符
    z = std::move(y); // 移动赋值运算符

    fprintf(stderr,"==========================\n");
    //std1::Any b(10);
    fprintf(stderr,"==========================\n");
    //std1::Any l(i);
    std1::Any m(A(10));

    // test

    std::cout << std1::any_cast<A>(std::move(m)).GetVal() << std::endl;
}
