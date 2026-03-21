#include "doctest.h"

#include "utils/all"

TEST_CASE("queue"){
    util::SafeLinkQueue<int> queue;
    queue.push(1);
    queue.push(2);
    queue.push(3);
    CHECK(queue.size() == 3);

    auto val1 = queue.tryPop();
    CHECK(val1.has_value());
    CHECK(val1.value() == 1);

    auto val = queue.popWait();
    CHECK(val == 2);

    queue.push(4);
    CHECK(queue.size() == 2);

    queue.clear();
    CHECK(queue.size() == 0);

    try{
        queue.popWait(std::chrono::milliseconds(100));
    }catch(const util::TimeoutError & e){
        
    }
}