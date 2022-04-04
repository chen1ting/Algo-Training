#include "Animal.h"
#include "ChildAnimal.h"

#include <vector>

int main() {
    /*
    //Animal a;
    //Animal b("Cat", COLOR::Black);
    Mammal c("Human", COLOR::White);
    Dog d("wangcai",COLOR::Brown, "laowang");
    Dog *e = new Dog("wangcai_Reference",COLOR::Brown, "laowang_Reference");
    Animal *animalPtr = new Dog("Lassie", White, "Andy");
    Dog dogi("Lassie", White, "Andy");
    Mammal *aniPtr = &dogi ;
    Mammal &aniRef = dogi ;
    Mammal aniVal = dogi ;
    aniPtr->speak() ;
    aniRef.speak() ;
    aniVal.speak() ;
    //a.speak();
    c.speak();
    d.speak();
    e->speak();
    animalPtr->speak();
    d.move();
    e->move();
    animalPtr->move();
    cout << "Program exiting …. "<< endl;
    system("Pause");
    return 0;
    delete e, animalPtr;
    */
    //below are for Q2:
    vector<Mammal*> zoo_mammal;
    Mammal *tmp;
    while(true){
        cout<<"Select the animal to send to Zoo :\n"<<
            "(1) Dog (2) Cat (3) Lion (4) Move all animals (5) Quit"<<endl;
        int choice;
        cin>>choice;
        switch(choice){
            case 1:
                tmp = new Dog((string)"dog_ptr"+=to_string(zoo_mammal.size()), COLOR::Black, "Zoo");
                zoo_mammal.push_back(tmp);
                break;
            case 2:
                tmp = new Cat((string)"cat_ptr"+=to_string(zoo_mammal.size()), COLOR::White);
                zoo_mammal.push_back(tmp);
                break;
            case 3:
                tmp = new Lion((string)"lion_ptr"+=to_string(zoo_mammal.size()), COLOR::Brown);
                zoo_mammal.push_back(tmp);
                break;
            case 4:
                for(size_t i = 0; i < zoo_mammal.size(); i++){
                    zoo_mammal[i]->move();
                    zoo_mammal[i]->speak();
                    zoo_mammal[i]->eat();
                }
            default:
                for(size_t i = 0; i < zoo_mammal.size(); i++)
                    delete zoo_mammal[i];
                return 0;
        }
    }
}
