#include "my_any.h"
#include <iostream>

class A {
    public:
        A(int a):_val(new int(a)) {fprintf(stderr,"constructor!\n");}
        ~A() {fprintf(stderr,"destroctor!\n");delete _val;}
        A(const A &other):_val(new int(*other._val)) {
            fprintf(stderr,"copy constructor\n"); 
        }
        A & operator=(const A &other) {
            fprintf(stderr,"copy operator\n");
            int *tmp = new int(*other._val);
            delete _val;
            _val = tmp;
            return *this;
        }

        int GetVal() {return *_val;}

        // 定义移动构造函数
        A(A &&other):_val(other._val) {
            fprintf(stderr,"move constuctor\n");
            other._val = nullptr;
        }
        A & operator==(A &&other) {
            fprintf(stderr,"move operator\n");
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
    std1::Any a = A(10);
    //std::cerr << "any_cast = " << std1::any_cast<A>(a).GetVal() << std::endl;

    fprintf(stderr,"==========================\n");
    //std1::Any b(10);
    fprintf(stderr,"==========================\n");

    //A d = GetA(10);
}
