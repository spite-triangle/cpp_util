#include "doctest.h"

#include <thread>
#include <iostream>

#include "utils/all"

TEST_CASE("cancellation_norm"){
    auto tokenSource = std::make_shared<util::CancellationTokenSource>();

    auto token = tokenSource->token();
    CHECK(token->isCancellationRequested() == false);

    bool callbackCalled = false;
    token->onCancellationRequested([&](){
        callbackCalled = true;
    });

    tokenSource->cancel();
    CHECK(token->isCancellationRequested() == true);
    CHECK(callbackCalled == true);
}

TEST_CASE("cancellation_dispose"){
    auto tokenSource = std::make_shared<util::CancellationTokenSource>();

    auto token = tokenSource->token();
    CHECK(token->isCancellationRequested() == false);

    int callbackCount = 0;
    auto dispose1 = token->onCancellationRequested([&](){
        callbackCount = 1;
        std::cout << "Callback called 1" << std::endl;
    });

    auto dispose2 = token->onCancellationRequested([&](){
        callbackCount = 2;
        std::cout << "Callback called 2" << std::endl;
    });
    
    {
        auto dispose3 = token->onCancellationRequested([&](){
            callbackCount = 3;
            std::cout << "Callback called 3" << std::endl;
        }, true);
        
        dispose2->dispose();
    }

    tokenSource->cancel();
    CHECK(token->isCancellationRequested() == true);

    CHECK(callbackCount == 1); // dispose2 已经被销毁，不会被调用
}

TEST_CASE("cancellation_parent"){
    auto parentSource = std::make_shared<util::CancellationTokenSource>();
    auto childSource = std::make_shared<util::CancellationTokenSource>(parentSource->token());

    CHECK(parentSource->token()->isCancellationRequested() == false);
    CHECK(childSource->token()->isCancellationRequested() == false);

    parentSource->cancel();

    CHECK(parentSource->token()->isCancellationRequested() == true);
    CHECK(childSource->token()->isCancellationRequested() == true);
}

TEST_CASE("cancellation_callback_after_cancel"){
    auto tokenSource = std::make_shared<util::CancellationTokenSource>();

    auto token = tokenSource->token();
    CHECK(token->isCancellationRequested() == false);

    tokenSource->cancel();
    CHECK(token->isCancellationRequested() == true);

    bool callbackCalled = false;
    token->onCancellationRequested([&](){
        callbackCalled = true;
    });

    CHECK(callbackCalled == true);
}

TEST_CASE("cancellation_pool"){
    
    auto tokenSource1 = std::make_shared<util::CancellationTokenSource>();
    auto tokenSource2 = std::make_shared<util::CancellationTokenSource>();

    
    util::CancellationTokenPool pool;

    pool.add(tokenSource1->token());
    pool.add(tokenSource2->token());

    CHECK(tokenSource1->token()->isCancellationRequested() == false);
    CHECK(tokenSource2->token()->isCancellationRequested() == false);

    bool callback1Called = false;
    pool.token()->onCancellationRequested([&](){
        std::cout << "Cancellation requested in pool" << std::endl;
        callback1Called = true;
    });

    tokenSource1->cancel();
    tokenSource2->cancel();

    CHECK(callback1Called == true);
}

TEST_CASE("cancellation_parallel"){
    auto tokenSource = std::make_shared<util::CancellationTokenSource>();

    auto t1 = std::thread([tokenSource](){
        auto token = tokenSource->token();

        token->onCancellationRequested([&](){
            std::cout << "Cancellation requested in thread 1" << std::endl;
        });

        while (!token->isCancellationRequested()) {
            std::cout << "Thread 1 is running..." << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });

    auto t2 = std::thread([tokenSource](){
        auto token = tokenSource->token();
        while (!token->isCancellationRequested()) {
            std::cout << "Thread 2 is running..." << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });

    tokenSource->token()->onCancellationRequested([&](){
        std::cout << "Cancellation requested in main thread" << std::endl;
    });

    std::this_thread::sleep_for(std::chrono::seconds(1));
    tokenSource->cancel();

    t1.join();
    t2.join();
}