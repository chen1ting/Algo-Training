//
// Created by 陈一婷 on 31/3/22.
//

#ifndef CHILDANIMAL
#define CHILDANIMAL
#include "Animal.h"

class Dog: public Mammal{
public:
    Dog():Mammal(), _owner("unknown"){ }
    Dog(string n, COLOR c, string owner): Mammal(n,c), _owner(owner){ }
    ~Dog();
    void speak() const;
protected:
    string _owner;
};

class Cat:public Mammal{
public:
    Cat():Mammal(){ }
    Cat(string n, COLOR c): Mammal(n,c){ }
    ~Cat();
    void move() const;
    void speak() const;
};

class Lion:public Mammal{
public:
    Lion():Mammal(){ }
    Lion(string n, COLOR c): Mammal(n,c){ }
    ~Lion();
    void move() const;
    void speak() const;
};

#endif //LAB5_SS3_CHENYITING_CHILDANIMAL_H
