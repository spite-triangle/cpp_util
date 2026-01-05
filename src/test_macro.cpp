#include "doctest.h"

#include "util.hpp"


TEST_CASE("defer"){
    printf("begin\n");
    RAII_DEFER(
        printf("defer\n");
    );
    printf("end\n");

    /*
    打印顺序:
      begin
      end
      defer
    */
}