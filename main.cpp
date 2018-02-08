//
//  main.cpp
//  Assignmen02_Shell
//
//  Created by Doug on 1/29/18.
//  Copyright © 2018 Doug. All rights reserved.
//

#include <iostream>
#include <unistd.h> // for execvp
#include "shelpers.hpp"

int main(int argc, const char * argv[]) {
    
    std::string userCommand;
    
    // Running loop that allows for user to input their commands
    while(std::getline(std::cin, userCommand)){
        
        // Turn the user input into command(s)
        std::vector<std::string> tokens = tokenize(userCommand);
        std::vector<Command> commands = getCommands(tokens);

        // Check for exit command
        if("exit" == commands[0].exec){
            return 0;
        }

        // TODO: check if a real command exists. Then fork.
        int pid = fork();
        
        if(pid < 0){
            std::cout << "fork() failed. Exiting shell program.\n";
            return 0;
        }

        else if(pid == 0) { // CHILD PROCESS
            int working = 0;
            working = execvp(commands[0].argv[0], const_cast<char* const*>(commands[0].argv.data()));

            // Check for errors with exec command
            if (working < 0){
                std::cout << "execvp failed with error: " << errno << "\n";
                exit(errno);
            }
        
        }
        else { // PARENT PROCESS
            wait(NULL);
        }

        
    }
    

    std::cout << "PROGRAM TERMINATED!\n";
    return 0;
}
