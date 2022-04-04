//
// Created by 陈一婷 on 31/3/22.
//

#ifndef ANIMAL
#define ANIMAL
#include <iostream>
#include <string>
using namespace std ;
enum COLOR { Green, Blue, White, Black, Brown } ;

class Animal {
public :
    Animal();
    Animal(string n, COLOR c);
    ~Animal();
    virtual void speak() const;
    virtual void move() const = 0;
protected :
    string _name;
    COLOR _color ;
};

class Mammal: public Animal{
public:
    Mammal():Animal(){ }
    Mammal(string n, COLOR c) : Animal(n,c) { }
    ~Mammal();
    void eat() const;
    virtual void move() const;
};

#endif //LAB5_SS3_CHENYITING_ANIMAL_H
