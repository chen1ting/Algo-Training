//
// Created by 陈一婷 on 31/3/22.
//

#include "ChildAnimal.h"

Dog::~Dog() {
    cout << "destructing Dog object "<< _name << endl ;
}

void Dog::speak() const {
    cout << _name << ": Dog Woof" << endl;
}

Cat::~Cat() {
    cout << "destructing Cat object "<< _name << endl ;
}

void Cat::move() const {
    cout << _name << ": Cat move" << endl;
}

void Cat::speak() const{
        cout << _name << ": Cat Meow" << endl;
}

Lion::~Lion(){
cout << "destructing Lion object "<< _name << endl ;
}

void Lion::move() const {
    cout << _name << ": Lion move" << endl;
}

void Lion::speak() const{
    cout << _name << ": Lion Roar" << endl;
}