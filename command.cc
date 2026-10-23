
/*
 * 
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <glob.h>
#include <wordexp.h>
#include <vector>
#include <string>
#include <cctype>
#include <iostream>
#include <sstream>
#include "command.h"
#include "tokenizer.h"


void parse(std::vector<Token> &tokens)
{
    Command currentCommand;
    SimpleCommand *currentSimpleCommand = new SimpleCommand();

    bool errorFlag = false;

    for (int i = 0; i < (int)tokens.size(); i++)
    {
        // End of input
        if (tokens[i].type == TOKEN_EOF){
            break;
        }

        // Normal command or argument
        if (tokens[i].type == TOKEN_COMMAND || tokens[i].type == TOKEN_ARGUMENT){
            currentSimpleCommand->insertArgument(strdup(tokens[i].value.c_str()));
        }

        // Pipe |
        else if (tokens[i].type == TOKEN_PIPE){
            if (currentSimpleCommand->_numberOfArguments == 0){
                fprintf(stderr, "syntax error: missing command before pipe\n");
                errorFlag = true;
                break;
            }

            currentCommand.insertSimpleCommand(currentSimpleCommand);
            currentSimpleCommand = new SimpleCommand();
        }

        // Background &
        else if (tokens[i].type == TOKEN_BACKGROUND){
            if (currentSimpleCommand->_numberOfArguments == 0){
                fprintf(stderr, "syntax error: missing command before &\n");
                errorFlag = true;
                break;
            }

            // & must be the last token before EOF
            if (i + 1 < (int)tokens.size() && tokens[i + 1].type != TOKEN_EOF){
                fprintf(stderr, "syntax error: & must be at the end\n");
                errorFlag = true;
                break;
            }

            currentCommand._background = 1;
        }

        // Output, append, input, or error redirection
        else if (tokens[i].type == TOKEN_REDIRECT ||
                 tokens[i].type == TOKEN_APPEND ||
                 tokens[i].type == TOKEN_INPUT ||
                 tokens[i].type == TOKEN_ERROR ||
                 tokens[i].type == TOKEN_REDIRECT_AND_ERROR)
        {
            // Check that a filename exists
            if (i + 1 >= (int)tokens.size() ||
                tokens[i + 1].type == TOKEN_EOF ||
                tokens[i + 1].type == TOKEN_PIPE ||
                tokens[i + 1].type == TOKEN_BACKGROUND ||
                tokens[i + 1].type == TOKEN_REDIRECT ||
                tokens[i + 1].type == TOKEN_APPEND ||
                tokens[i + 1].type == TOKEN_INPUT ||
                tokens[i + 1].type == TOKEN_ERROR ||
                tokens[i + 1].type == TOKEN_REDIRECT_AND_ERROR)
            {
                fprintf(stderr, "syntax error: missing filename\n");
                errorFlag = true;
                break;
            }

            std::string fileName = tokens[i + 1].value;

            if (tokens[i].type == TOKEN_REDIRECT){
                free(currentCommand._outFile);
                currentCommand._outFile = strdup(fileName.c_str());
                currentCommand._append = 0;
            }
            else if (tokens[i].type == TOKEN_APPEND){
                free(currentCommand._outFile);
                currentCommand._outFile = strdup(fileName.c_str());
                currentCommand._append = 1;
            }
            else if (tokens[i].type == TOKEN_INPUT){
                free(currentCommand._inputFile);
                currentCommand._inputFile = strdup(fileName.c_str());
            }
            else if (tokens[i].type == TOKEN_ERROR){
                free(currentCommand._errFile);
                currentCommand._errFile = strdup(fileName.c_str());
            }
            else if (tokens[i].type == TOKEN_REDIRECT_AND_ERROR){
                free(currentCommand._outFile);
                free(currentCommand._errFile);

                currentCommand._outFile = strdup(fileName.c_str());
                currentCommand._errFile = strdup(fileName.c_str());

                currentCommand._append = 1;
                currentCommand._out_error = 1;
            }

            // Skip the filename token
            i++;
        }
    }

    // Handle syntax errors and clean up
    if (errorFlag)
    {
        delete currentSimpleCommand;
        currentCommand.clear();
        return;
    }

    // Check whether the command after the last pipe is empty
    if (currentSimpleCommand->_numberOfArguments == 0)
    {
        if (currentCommand._numberOfSimpleCommands > 0 ||
            currentCommand._outFile != NULL ||
            currentCommand._inputFile != NULL ||
            currentCommand._errFile != NULL){
            fprintf(stderr, "syntax error: missing command\n");
        }

        delete currentSimpleCommand;
        currentCommand.clear();
        return;
    }

    // Add the final simple command
    currentCommand.insertSimpleCommand(currentSimpleCommand);

    // Execute the complete command
    currentCommand.execute();
}
// constructor -- to save simple command
SimpleCommand::SimpleCommand()
{
    _numberOfAvailableArguments = 5;
    _numberOfArguments = 0;
    _arguments = (char **)malloc(_numberOfAvailableArguments * sizeof(char *));
}

void SimpleCommand::insertArgument(char *argument)
{
    if (_numberOfAvailableArguments == _numberOfArguments + 1)
    {
        _numberOfAvailableArguments *= 2;
        _arguments = (char **)realloc(_arguments,
                                      _numberOfAvailableArguments * sizeof(char *));
    }

    _arguments[_numberOfArguments] = argument;
    _arguments[_numberOfArguments + 1] = NULL;

    _numberOfArguments++;
}

Command::Command()
{
    _numberOfAvailableSimpleCommands = 1;
    _simpleCommands = (SimpleCommand **)
        malloc(_numberOfAvailableSimpleCommands * sizeof(SimpleCommand *));

    _numberOfSimpleCommands = 0;
    _outFile = 0;
    _inputFile = 0;
    _errFile = 0;
    _background = 0;
	_out_error = 0;
	_background = 0;
	_append = 0;
}

void Command::insertSimpleCommand(SimpleCommand *simpleCommand)
{
    if (_numberOfAvailableSimpleCommands == _numberOfSimpleCommands)
    {
        _numberOfAvailableSimpleCommands *= 2;
        _simpleCommands = (SimpleCommand **)realloc(_simpleCommands,
                                                    _numberOfAvailableSimpleCommands * sizeof(SimpleCommand *));
    }

    _simpleCommands[_numberOfSimpleCommands] = simpleCommand;
    _numberOfSimpleCommands++;
}

void Command::clear()
{
    for (int i = 0; i < _numberOfSimpleCommands; i++)
    {
        for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++)
        {
            free(_simpleCommands[i]->_arguments[j]);
        }

        free(_simpleCommands[i]->_arguments);
        free(_simpleCommands[i]);
    }

    if (_outFile)
    {
        free(_outFile);
    }

    if (_inputFile)
    {
        free(_inputFile);
    }

    if (_errFile)
    {
        free(_errFile);
    }

    _numberOfSimpleCommands = 0;
    _outFile = 0;
    _inputFile = 0;
    _errFile = 0;
    _background = 0;
    //added
    _append = 0;
    _out_error = 0;
}

void Command::print()
{
    printf("\n\n");
    printf("              COMMAND TABLE                \n");
    printf("\n");
    printf("  #   Simple Commands\n");
    printf("  --- ----------------------------------------------------------\n");

    for (int i = 0; i < _numberOfSimpleCommands; i++)
    {
        printf("  %-3d ", i);
        for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++)
        {
            printf("\"%s\" \t", _simpleCommands[i]->_arguments[j]);
        }
    }

    printf("\n\n");
    printf("  Output       Input        Error        Err&Out       Background\n");
    printf("  ------------ ------------ ------------ ------------ ------------\n");
    printf("  %-12s %-12s %-12s %-12s %-12s\n", _outFile ? _outFile : "default",
           _inputFile ? _inputFile : "default", _errFile ? _errFile : "default", _out_error == 1 ? _errFile : "default"
           ,_background ? "YES" : "NO");
    printf("\n\n");
}

void Command::execute()
{
    print();

    if (_numberOfSimpleCommands == 0)
    {
        clear();
        return;
    }

    int defaultin = dup(STDIN_FILENO);
    int defaultout = dup(STDOUT_FILENO);
    int defaulterr = dup(STDERR_FILENO);

    int inputfd = defaultin;
    int outputfd = defaultout;
    int errorfd = defaulterr;

    if (_inputFile)
    {
        inputfd = open(_inputFile, O_RDONLY);
        if (inputfd < 0)
        {
            perror("open input");
            close(defaultin);
            close(defaultout);
            close(defaulterr);
            return;
        }
    }

    if (_outFile)
    {
        int flags = O_WRONLY | O_CREAT | (_append ? O_APPEND : O_TRUNC);
        outputfd = open(_outFile, flags, 0666);
        if (outputfd < 0)
        {
            perror("open output");
            if (inputfd != defaultin) close(inputfd);
            close(defaultin);
            close(defaultout);
            close(defaulterr);
            return;
        }
    }

    if (_errFile)
    {
        int flags = O_WRONLY | O_CREAT | (_append ? O_APPEND : O_TRUNC);
        errorfd = open(_errFile, flags, 0666);
        if (errorfd < 0)
        {
            perror("open error");
            if (inputfd != defaultin) close(inputfd);
            if (outputfd != defaultout) close(outputfd);
            close(defaultin);
            close(defaultout);
            close(defaulterr);
            return;
        }
    }

    for (int i = 0; i < _numberOfSimpleCommands; i++)
    {
        SimpleCommand *simple = _simpleCommands[i];
        if (simple == NULL || simple->_numberOfArguments == 0)
        {
            continue;
        }

        pid_t pid = fork();
        if (pid == -1)
        {
            perror("fork");
            break;
        }
        else if (pid == 0)
        {
            if (i == 0 && _inputFile)
            {
                dup2(inputfd, STDIN_FILENO);
                close(inputfd);
            }

            if (i == _numberOfSimpleCommands - 1 && _outFile)
            {
                dup2(outputfd, STDOUT_FILENO);
                close(outputfd);
            }

            if (i == _numberOfSimpleCommands - 1 && _errFile)
            {
                dup2(errorfd, STDERR_FILENO);
                close(errorfd);
            }

            execvp(simple->_arguments[0], simple->_arguments);
            perror(simple->_arguments[0]);
            _exit(1);
        }

        int status = 0;
        waitpid(pid, &status, 0);
    }

    dup2(defaultin, STDIN_FILENO);
    dup2(defaultout, STDOUT_FILENO);
    dup2(defaulterr, STDERR_FILENO);

    close(defaultin);
    close(defaultout);
    close(defaulterr);

    if (inputfd != defaultin) close(inputfd);
    if (outputfd != defaultout) close(outputfd);
    if (errorfd != defaulterr) close(errorfd);

    clear();
}
// first 
void Command::prompt()
{
    printf("myshell>");
    fflush(stdout);
    std::string input;
    while (true)
    {
        std::getline(std::cin, input); 
        std::vector<Token> tokens = tokenize(input);
        parse(tokens);
    }
}

Command Command::_currentCommand;
SimpleCommand *Command::_currentSimpleCommand;

int main()
{
    Command::_currentCommand.prompt();
    return 0;
}
