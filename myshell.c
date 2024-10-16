#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <glob.h>
#include <fcntl.h>

#define MAX_INPUT_SIZE 1024
#define MAX_ARGS 64

char* PATHS[] = { "/usr/local/bin/", "/usr/bin/", "/bin/", NULL };
int is_prev_executed = 0;

void TogglePrevExecuted(int status)
{
    if (WIFEXITED(status)) 
    {
        int exit_status = WEXITSTATUS(status);
        if (exit_status != 0) 
        {
            fprintf(stderr, "Child process did not execute successfully.\n");
        }
        else
        {
            is_prev_executed = 1;
        }
    }
}
int CheckFileExists(const char* directory, const char* file) 
{
    char full_path[MAX_INPUT_SIZE];
    snprintf(full_path, sizeof(full_path), "%s%s", directory, file);

    return access(full_path, X_OK);
}

void ExecutePipeCommand(char* args1[], char* args2[])
{
    int pipe_fds[2];
    pid_t pid1, pid2;

    if (pipe(pipe_fds) == -1) 
    {
        perror("pipe: Pipe cannot be created");
        exit(1);
    }
    
    pid1 = fork();
    if(pid1 == -1)
    {
        perror("fork");
    }
    else if (pid1 == 0) 
    {
        char full_path[MAX_INPUT_SIZE];
    	for(int i = 0; PATHS[i] != NULL; ++i)
    	{
    	    if(CheckFileExists(PATHS[i], args1[0]) == 0)
    	    {
    	        snprintf(full_path, sizeof(full_path), "%s%s", PATHS[i], args1[0]);
    	        break;
    	    }
    	}
    	
        close(pipe_fds[0]);
        dup2(pipe_fds[1], STDOUT_FILENO);
        close(pipe_fds[1]);
        
        if(execv(full_path, args1) == -1)
        {
            perror("execv: Failed execution");
            exit(EXIT_FAILURE);
        }
    }
    

    pid2 = fork();
    if(pid2 == -1)
    {
        perror("fork");
    }
    else if (pid2 == 0) 
    {
        char full_path[MAX_INPUT_SIZE];
    	for(int i = 0; PATHS[i] != NULL; ++i)
    	{
    	    if(CheckFileExists(PATHS[i], args2[0]) == 0)
    	    {
    	        snprintf(full_path, sizeof(full_path), "%s%s", PATHS[i], args2[0]);
    	        break;
    	    }
    	}
    	
        close(pipe_fds[1]);
        dup2(pipe_fds[0], STDIN_FILENO);
        close(pipe_fds[0]);
        
        if(execv(full_path, args2) == -1)
        {
            perror("execv: Failed execution");
            exit(EXIT_FAILURE);
        }
    }
    
    int status1, status2;
    close(pipe_fds[0]);
    close(pipe_fds[1]);
    waitpid(pid1, &status1, 0);
    waitpid(pid2, &status2, 0);
    TogglePrevExecuted(status1);
    TogglePrevExecuted(status2);
}

void ExecuteCommand(char* tokens[])
{
    if(tokens[0][0] == '/')
    {
        pid_t pid = fork();
        if(pid == -1)
        {
            perror("fork");
        }
        if(pid == 0)
        {
            if(execv(tokens[0], tokens) == -1)
            {
                perror("execv");
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            int status;
            waitpid(pid, &status, 0);
            TogglePrevExecuted(status);
        }
    }
    else if (strcmp(tokens[0], "cd") == 0) 
    {
    	is_prev_executed = 1;
        if (tokens[1] == NULL || tokens[2] != NULL) 
        {
            is_prev_executed = 0;
            fprintf(stderr, "cd: Number of parameters is not correct.\n");
        }
        else if (chdir(tokens[1]) != 0) 
        {
            is_prev_executed = 0;
            perror("cd: ");
        }
    }
    else if (strcmp(tokens[0], "pwd") == 0) 
    {
    	is_prev_executed = 0;
        char cwd[MAX_INPUT_SIZE];
        if (getcwd(cwd, sizeof(cwd)) != NULL) 
        {
            is_prev_executed = 1;
            printf("%s\n", cwd);
        }
        else 
        {
            perror("pwd: Cannot display current directory.\n");
        }
    }
    else if (strcmp(tokens[0], "which") == 0) 
    {
    	is_prev_executed = 0;
        if (tokens[1] == NULL || tokens[2] != NULL) 
        {
            return;
        }
        else
        {
            char cwd[MAX_INPUT_SIZE];
            if(getcwd(cwd, sizeof(cwd)) == NULL)
            {
            	return;
            }
            char temp_token[MAX_INPUT_SIZE];
            strcpy(temp_token, tokens[1]);
            temp_token[0] = temp_token[1];
            if(CheckFileExists(cwd, temp_token) == 0)
            {
            	is_prev_executed = 1;
                printf("%s\n", tokens[1]);
            }
            else
            {
                for (int i = 0; PATHS[i] != NULL; i++) 
                {
                    if (CheckFileExists(PATHS[i], tokens[1]) == 0) 
                    {
                    	is_prev_executed = 1;
                        printf("%s%s\n", PATHS[i], tokens[1]);
                        break;
                    }
                }
            }

        }
    }
    else 
    {
    	int found = 0;
    	char full_path[MAX_INPUT_SIZE];
    	for(int i = 0; PATHS[i] != NULL; ++i)
    	{
    	    if(CheckFileExists(PATHS[i], tokens[0]) == 0)
    	    {
    	        snprintf(full_path, sizeof(full_path), "%s%s", PATHS[i], tokens[0]);
    	        found = 1;
    	        break;
    	    }
    	}
    	
        if (!found) 
        {
            fprintf(stderr, "Command '%s' not found.\n", tokens[1]);
            return;
        }
        
        pid_t pid = fork();
        if (pid == -1) 
        {
            perror("fork");
        }
        else if (pid == 0) 
        {
            if (execv(full_path, tokens) == -1) 
            {
                perror("execv");
                exit(EXIT_FAILURE);
            }
        }
        else 
        {
            int status;
            waitpid(pid, &status, 0);
            TogglePrevExecuted(status);
        }
    }
}

void Wildcards(char* arg, char** argv, int* argc)
{
    glob_t glob_result;

    if (glob(arg, GLOB_NOCHECK | GLOB_TILDE, NULL, &glob_result) == 0) 
    {
        for (int i = 0; i < glob_result.gl_pathc && *argc < MAX_ARGS - 1; i++) 
        {
            argv[(*argc)++] = strdup(glob_result.gl_pathv[i]);
        }
    }
    globfree(&glob_result);
}

void Redirection(char *tokens[])
{
    int input_fd = -1, output_fd = -1;

    for (int i = 0; tokens[i] != NULL; i++) 
    {
        if (strcmp(tokens[i], "<") == 0) 
        {
            input_fd = open(tokens[i + 1], O_RDONLY);
            if (input_fd == -1) 
            {
                perror("Input redirection error");
                exit(1); // Set exit status to 1
            }

            pid_t pid = fork();
            if(pid == -1)
            {
                perror("fork");
            }
            else if(pid == 0)
            {
                char full_path[MAX_INPUT_SIZE];
    	        for(int i = 0; PATHS[i] != NULL; ++i)
    	        {
    	            if(CheckFileExists(PATHS[i], tokens[0]) == 0)
    	            {
    	                snprintf(full_path, sizeof(full_path), "%s%s", PATHS[i], tokens[0]);
    	                break;
    	            }
    	        }
                dup2(input_fd, STDIN_FILENO); 
                tokens[i] = NULL; 
                tokens[i + 1] = NULL;
                if(execv(full_path, tokens) == -1)
                {
                    perror("execv");
                    exit(EXIT_FAILURE);
                }
            }
            else if(pid > 0)
            {
                int status;
                waitpid(pid, &status, 0);
		TogglePrevExecuted(status);
                close(input_fd);
                return;
            }
        } 
        else if (strcmp(tokens[i], ">") == 0) 
        {
            output_fd = open(tokens[i + 1], O_WRONLY | O_CREAT | O_TRUNC , 0640);
            if (output_fd == -1) 
            {
                perror("Output redirection error");
                exit(1);
            }

            pid_t pid = fork();
            if(pid == -1)
            {
                perror("fork");
            }
            else if(pid == 0)
            {
                char full_path[MAX_INPUT_SIZE];
    		for(int i = 0; PATHS[i] != NULL; ++i)
    		{
    		    if(CheckFileExists(PATHS[i], tokens[0]) == 0)
    		    {
    	  	      snprintf(full_path, sizeof(full_path), "%s%s", PATHS[i], tokens[0]);
    	  	      break;
    	 	   }
    		}
                dup2(output_fd, STDOUT_FILENO); 
                tokens[i] = NULL; 
                tokens[i + 1] = NULL;
                
                if(execv(full_path, tokens) == -1)
                {
                    perror("execv");
                    exit(EXIT_FAILURE);
                }
            }
            else if(pid > 0)
            {
                int status;
                waitpid(pid, &status, 0);
                TogglePrevExecuted(status);
                close(output_fd);
                return;
            }
        }
    }
}

void Tokenize(char* input, char* tokens[MAX_ARGS])
{
    char* args_pipe[MAX_ARGS];
    int token_count = 0;
    int pipe_count = 0;
    int pipe_found = 0;
    int is_executed = 0;

    int length = strlen(input);
    char modifiedString[length * 2];
    int j = 0;

    for (int i = 0; i < length; i++) 
    {
        if ((input[i] == '<' || input[i] == '|' ||input[i] == '>') && i > 0 && (input[i - 1] != ' ' || input[i + 1] != ' ')) 
        {
            modifiedString[j++] = ' ';
            modifiedString[j++] = input[i];
            modifiedString[j++] = ' ';
        } 
        else 
        {
            modifiedString[j++] = input[i];
        }
    }
    modifiedString[j] = '\0';
    input = modifiedString;

    char* token = strtok(input, " ");
    if (strcmp(token, "then") == 0)
    {
        if(is_prev_executed == 0)
        {
            return;
        }
        token = strtok(NULL, " ");
    }    
    else if (strcmp(token, "else") == 0)
    {
    	if(is_prev_executed == 1)
    	{
    	    is_prev_executed = 0;
    	    return;
    	}
    	token = strtok(NULL, " ");
    }

    
    while (token != NULL && token_count < MAX_ARGS - 1) 
    {
        if (strchr(token, '|'))
        {
            pipe_found = 1;
            break;
        }
        else if (strchr(token, '*')) 
        {
            Wildcards(token, tokens, &token_count);
        }
        else if(strchr(token, '<'))
        {
            while(token != NULL && token_count < MAX_ARGS - 1)
            {
                tokens[token_count++] = token;
            	token = strtok(NULL, " ");
            }
            Redirection(tokens);
            is_executed = 1;
        }
        else if(strchr(token, '>'))
        {

            while(token != NULL && token_count < MAX_ARGS - 1)
            {
                tokens[token_count++] = token;
            	token = strtok(NULL, " ");
            }
            Redirection(tokens);
            is_executed = 1;
        }
        else 
        {
            tokens[token_count++] = token;
        }
        token = strtok(NULL, " ");
    }
    tokens[token_count] = NULL;
    if (pipe_found) 
    {
        token = strtok(NULL, " ");
        while (token != NULL && pipe_count < MAX_ARGS - 1) 
        {
            args_pipe[pipe_count++] = token;
            token = strtok(NULL, " ");
        }
        args_pipe[pipe_count] = NULL;

        ExecutePipeCommand(tokens, args_pipe);
    }
    else if (is_executed == 0)
    {
        ExecuteCommand(tokens);
    }
}

void InteractiveMode()
{
    char input[MAX_INPUT_SIZE];
    char* tokens[MAX_ARGS];

    while (1)
    {
        printf("mysh> ");

        if (!fgets(input, MAX_INPUT_SIZE, stdin)) 
        {
            break;
        }

        input[strcspn(input, "\n")] = 0;
        if (strcmp(input, "exit") == 0)
        {
       	    printf("mysh: exiting\n");
            break;
        }
        Tokenize(input, tokens);
    }
}

void BatchMode(const char* file_name)
{
    char input[MAX_INPUT_SIZE];
    char* args[MAX_ARGS];

    FILE* file = fopen(file_name, "r");
    if (file == NULL)
    {
        perror("File not found");
        exit(EXIT_FAILURE);
    }

    while (fgets(input, MAX_INPUT_SIZE, file) != NULL)
    {
        input[strcspn(input, "\n")] = 0;
        if(strcmp(input, "exit") == 0)
        {
            printf("mysh: exiting\n");
            break;
        }
        Tokenize(input, args);
    }

    fclose(file);
}

int main(int argc, char* argv[])
{
    if (argc == 2)
    {
        BatchMode(argv[1]);
    }
    else if (argc == 1)
    {
        printf("Welcome to my shell!\n");
        InteractiveMode();
    }
    else
    {
        fprintf(stderr, "Invalid arguments. Usage: %s [batch_file]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    return 0;
}