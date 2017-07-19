#pragma once
#include <stormancer.h>

#ifdef __cplusplus
extern "C" {
#endif
    
/// Run the test suite, and block indefinitely on stdin.
void run_all_tests();
    
/// Same as run_all_tests(), but returns immediately.
/// Use tests_done() and tests_passed() to check the tests' status.
void run_all_tests_nonblocking();
    
/// Check whether the tests are done (NOT if they're succesful).
bool tests_done();
    
/// Check whether or not the tests were succsful.
/// You need to check that the tests have finished running with tests_done() before calling this function.
bool tests_passed();

#ifdef __cplusplus
}
#endif
