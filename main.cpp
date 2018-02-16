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

    std::vector<pid_t> backgroundKids;

    // Running loop that allows for user to input their commands
    while (std::getline(std::cin, userCommand))
    {

        // Check for exit command
        if("exit" == userCommand){
            return 0;
        }

        // check for and alert of finished background commands
        for(int i = 0; i < backgroundKids.size(); i++){
            pid_t status = waitpid(backgroundKids[i], NULL, WNOHANG);
            // background process is finished
            if(status == backgroundKids[i]){
                std::cout << "the following process was completed: " << status << "\n";
                backgroundKids.erase(std::remove(backgroundKids.begin(), backgroundKids.end(), backgroundKids[i]), backgroundKids.end());
            }
            // background process still running
            else if(status == 0){
                // std::cout << "background process still running.\n";
            }
        }

        std::vector<pid_t> waitingKids;

        // Check for nothing
        if("" == userCommand){
            continue;
        }

        // Turn the user input into command(s)
        std::vector<std::string> tokens = tokenize(userCommand);
        std::vector<Command> commands = getCommands(tokens);

        // Check for a directory change
        if ("cd" == commands[0].exec){
            // If no arguments passed with "cd" command, return to home directory
            if(commands[0].argv.size() == 2){ // size 2 because of NULL at end
                chdir(getenv("HOME"));
            }
            else{
                chdir(commands[0].argv[1]);
            }

            continue;
        }

        // Process each of the commands
        for (int i = 0; i < commands.size(); i++){

            pid_t pid = fork();

            // Add child to appropriate background/nonbackground list
            if(commands[i].background){
                backgroundKids.push_back(pid);
            }
            else{
                waitingKids.push_back(pid);
            }

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

                // Close open file descriptors not used by this process
                for(int j = 0; j < commands.size(); j++){
                    // Skup current command
                    if(i == j){
                        continue;
                    }

                    if(commands[j].fdStdin != READ_END){
                        close(commands[j].fdStdin);
                    }
                    if(commands[j].fdStdout != WRITE_END){
                        close(commands[j].fdStdout);
                    }
                }

                int working = 0;
                working = execvp(commands[i].argv[0], const_cast<char *const *>(commands[i].argv.data()));

                // Check for errors with exec command
                if (working < 0)
                {
                    std::cout << "Your command (exec) failed with error: " << errno << "\n";
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

                // Wait for children to complete in order
                for(int i = 0; i < waitingKids.size(); i++){
                    waitpid(waitingKids[i], NULL, 0);
                }
               
            } // END PARENT
        } // END PROCESSING COMMANDS
    } // END WHILE LOOP FOR PROGRAM

    std::cout << "PROGRAM TERMINATED!\n";
    return 0;
}
