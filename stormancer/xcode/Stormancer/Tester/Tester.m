//
//  Tester.m
//  Tester
//
//  Created by Matthieu RICHARD on 28/04/2017.
//  Copyright © 2017 Stormancer. All rights reserved.
//

#import <XCTest/XCTest.h>
#import <unistd.h>
#import "test.h"

@interface Tester : XCTestCase

@end

@implementation Tester

- (void)setUp {
    [super setUp];
    // Put setup code here. This method is called before the invocation of each test method in the class.
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    [super tearDown];
}

- (void)testExample {
    // This is an example of a functional test case.
    // Use XCTAssert and related functions to verify your tests produce the correct results.
    run_all_tests_nonblocking();
    
    while (!tests_done()) {
        sleep(1);
    }
    XCTAssertTrue(tests_passed(), @"Test suite failed.");
}

- (void)testPerformanceExample {
    // This is an example of a performance test case.
    [self measureBlock:^{
        // Put the code you want to measure the time of here.
    }];
}

@end
