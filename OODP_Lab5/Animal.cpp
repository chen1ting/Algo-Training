//
// Created by 陈一婷 on 31/3/22.
//

#include "Animal.h"
Animal::Animal() : _name("unknown") {
    cout << "default constructing Animal object "<< _name << endl ;
}

Animal::Animal(string n, COLOR c) : _name(n), _color(c) {
    cout << "overload constructing Animal object "<< _name << endl ;
}

Animal::~Animal() {
    cout << "destructing Animal object "<< _name << endl ;
}
void Animal::speak() const {
    cout << "Animal "<<_name<< " speaks "<< endl ;
}

Mammal::~Mammal() {
    cout << "destructing Mammal object "<< _name << endl ;
}

void Mammal::eat() const {
    cout << _name << ": Mammal eat " << endl ;
}

void Mammal::move() const{
    cout << _name << ": Mammal move" << endl;
}