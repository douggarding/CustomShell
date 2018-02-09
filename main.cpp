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

#define READ_END 0
#define WRITE_END 1

int main(int argc, const char *argv[])
{

    std::string userCommand;

    // Running loop that allows for user to input their commands
    while (std::getline(std::cin, userCommand))
    {

        // Turn the user input into command(s)
        std::vector<std::string> tokens = tokenize(userCommand);
        std::vector<Command> commands = getCommands(tokens);

        // Check for exit command
        if ("exit" == commands[0].exec)
        {
            return 0;
        }

        // Check for a directory change
        if ("cd" == commands[0].exec){
            // If no arguments passed with "cd" command, return to home directory
            if(commands[0].argv.size() == 2){ // size 2 because of NULL at end
                chdir(getenv("HOME"));
            }
            else{
                chdir(commands[0].argv[1]);
            }
            // Proceed to collect next commands
            continue;
        }

        // Process each of the commands
        for (int i = 0; i < commands.size(); i++){
            // TODO: check if a real command exists. Then fork.
            int pid = fork();

            if (pid < 0)
            {
                std::cout << "fork() failed. Exiting shell program.\n";
                return 0;
            }

            else if (pid == 0)
            { // CHILD PROCESS

                // Set up correct file descriptors.
                dup2(commands[i].fdStdin, READ_END);
                dup2(commands[i].fdStdout, WRITE_END);

                int working = 0;
                working = execvp(commands[i].argv[0], const_cast<char *const *>(commands[i].argv.data()));

                // Check for errors with exec command
                if (working < 0)
                {
                    std::cout << "execvp failed with error: " << errno << "\n";
                    exit(errno);
                }
            }
            else
            { // PARENT PROCESS

                // Close read end of pipe if used
                if(commands[i].fdStdin > 0){
                    close(commands[i].fdStdin);
                }
                // Close write end of pipe if used
                if(commands[i].fdStdout > 1){
                    close(commands[i].fdStdout);
                }

                wait(NULL); 
            }
        }
    }

    std::cout << "PROGRAM TERMINATED!\n";
    return 0;
}
