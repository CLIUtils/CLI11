#include "CLI.hpp"


int main (int argc, char** argv) {

    std::vector<std::string> test_strings = {"a,boo", ",coo", "d,", "Q,this-is", "s", "single"};

    for(std::string name : test_strings) {
        std::string one;
        std::string two;
        
        std::tie(one, two) = CLI::split(name);
        std::cout << one << ", " << two << std::endl;
    }

    std::vector<std::string> test_fails= {"a,,boo", "a,b,c", "ssd,sfd", "-a", "", ",", "one two"};

    for(std::string name : test_fails) {
        std::string one;
        std::string two;
        
        try {
        std::tie(one, two) = CLI::split(name);
        std::cout << "Failed to catch: " << name << std::endl;
        return 1;
        } catch (const CLI::BadNameString &e) {
            std::cout << "Hooray! Caught: " << name << std::endl;
        }
    }

    
}
