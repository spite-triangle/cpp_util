#include "doctest.h"


#include "utils/all"

namespace
{

int fcn3(int a, int b){
    if( a == 0){
        throw util::ValueError("xxx");
    }
    return b / a;
}

int fcn2(int a, int b){
    int c = fcn3(10, b);
    int d = fcn3(a, 10);

    return fcn3(c,d);
}

void fcn1(int a, int b){
    if ( b > 0){
        fcn2(a,b);
    }else{
        fcn2(a,b + 1);
    }
}
    
} // namespace
 


TEST_CASE("stack_exception"){

    try{
        fcn1(0,2);
    }catch(const util::ValueError & e){
        CHECK(std::string(e.what()) == "xxx");

        printf("%s", e.detail().c_str());
    }
}