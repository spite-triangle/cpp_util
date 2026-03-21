#include "doctest.h"

#include "utils/all"


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