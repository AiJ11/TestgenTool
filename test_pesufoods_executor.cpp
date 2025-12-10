#include "config/executors/PesuFoodsExecutor.hpp" 
#include <iostream>

int main() {
    std::cout << "=== Testing PESU Foods Executor ===" << std::endl;
    
    PesuFoodsExecutor::setPort(5001);
    
    // 1. Health check
    if (!PesuFoodsExecutor::checkHealth()) {
        std::cerr << "Backend not running!" << std::endl;
        return 1;
    }
    
    // 2. Reset database
    PesuFoodsExecutor::resetTestData();
    
    // 3. Signup
    auto signupResp = PesuFoodsExecutor::signup("Test User", "test@pesu.com", "password123");
    std::cout << "Signup: " << (signupResp.success ? "SUCCESS" : "FAILED") << std::endl;
    
    // 4. Get users
    std::string users = PesuFoodsExecutor::getGlobalU();
    std::cout << "Users in DB: " << users << std::endl;
    
    // 5. Login
    auto loginResp = PesuFoodsExecutor::login("test@pesu.com", "password123");
    std::cout << "Login: " << (loginResp.success ? "SUCCESS" : "FAILED") << std::endl;
    
    // 6. Get account
    auto accountResp = PesuFoodsExecutor::getAccount();
    std::cout << "Account: " << (accountResp.success ? "SUCCESS" : "FAILED") << std::endl;
    
    return 0;
}
