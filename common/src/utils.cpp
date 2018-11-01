//
// Created by lotus mile on 31/10/2018.
//

#include "utils.hpp"
#include "config.hpp"


using namespace milecsa::explorer;

int milecsa::explorer::random(int min, int max){
    static bool first = true;
    if (first)
    {
        srand( time(0) );
        first = false;
    }
    return min + rand() % (( max + 1 ) - min);
}
