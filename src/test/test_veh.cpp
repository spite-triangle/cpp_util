#include <iostream>
#include <vector>
#include <thread>


#include "veh/utilVeh.h"

class TestClass {
public:
    void testMethod() {
        std::cout << "Test method called."<< this->a << this->b << std::endl;
    }

    int a;
    int b;
};

inline void test_access_null(){
    TestClass* obj = nullptr;
    obj->testMethod(); // This will cause an access violation and trigger the exception filter.
}

inline void test_range_error(){
    std::vector<int> vec = {1, 2, 3};
    int value = vec.at(5); // This will cause a range error and trigger the exception filter.
}


inline void vulnerable(const char *str) {
   char buffer[10];
   strcpy(buffer, str); // overrun buffer !!!

   // use a secure CRT function to help prevent buffer overruns
   // truncate string to fit a 10 byte buffer
   // strncpy_s(buffer, _countof(buffer), str, _TRUNCATE);
}

inline void test_overrun_buffer(){
    char large_buffer[] = "This string is longer than 10 characters!!";
    vulnerable(large_buffer);
}

inline void test_divide_by_zero(){
    int a = 0;
    int result = 10 / a; // This will cause a division by zero and trigger the exception filter.
}

inline void test_throw(){
    throw std::runtime_error("test exception"); // This will trigger the failure handler.
}

inline void run(){
    util::CoreDump::instance()->registerExceptionFilter([](const _EXCEPTION_POINTERS *pInfo) {
        std::cout << "xxxx" << std::endl;
    });

    util::CoreDump::instance()->registerFailureHandler([](const util::CoreDump::FAILURE_ERROR_E &enType) {
        std::cout << "yyyy" << std::endl;
    });

    
    auto t = std::thread([](){
        
        test_divide_by_zero();
    });
    t.join();

}