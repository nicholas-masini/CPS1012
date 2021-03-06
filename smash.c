// =================================== CPS1012 Assignment: smash ===================================
// Nicholas Masini 0021102H

#include "linenoise.h"
#include "functions.h"

int main(int argc, char **argv, char **env) {
  
    int i, j, k, l, m, n, o, var_count, q_count, id_count, tc_char, arguments, d_count;
    char *line, *temp1, *temp2;
    char buffer1[50];
    char buffer2[2];
    bool dquotes, var_fail, exist_var, limit_var, expand, assigning_dquotes, redirect, append;
    bool t = false;

    // Directory Stack
    char *dir_stack[20];
    // Initialising the directory stack
    for(int i = 0; i < 20; i++) {
        dir_stack[i] = malloc(200);
        strcpy(dir_stack[i], "");
    }

    // Shell variables are initialised
    shell *shellV = malloc(sizeof(shell) * SHELL_VARIABLES_LIMIT);
    FILE *fp1;
    FILE *fp2;
    FILE *out_fp;

    init_shellV(&shellV[0], "PATH");

    shellV[1].name = "PROMPT";
    shellV[1].val = "smash> ";

    shellV[2].name = "CWD";
    char curwd[PATH_MAX];  
    if(getcwd(curwd, PATH_MAX) == NULL) {
        printf("getcwd() error\n\n");
    } else {
        shellV[2].val = curwd;
    }

    // Adding CWD to directory stack
    strcpy(dir_stack[0], shellV[2].val);
    int d_pointer = 0;

    init_shellV(&shellV[3], "USERNAME");
    shellV[3].name = "USER";
    
    init_shellV(&shellV[4], "HOME");

    init_shellV(&shellV[5], "SHELL");

    shellV[6].name = "TERMINAL";
    fp1 = popen("/bin/tty", "r");
    if(fp1 == NULL) {
        printf("Failed to run command tty\n");
        exit(EXIT_FAILURE);
    }
    while((fread(buffer1, 1, sizeof(buffer1), fp1)) != 0);
    shellV[6].val = buffer1;
    strtok(shellV[6].val, "\n");
    pclose(fp1);

    shellV[7].name = "EXITCODE";
    fp2 = popen("/bin/echo $?", "r");
    if(fp2 == NULL) {
        printf("Failed to run command echo $?\n");
        exit(EXIT_FAILURE);
    }
    while((fread(buffer2, 1, sizeof(buffer2), fp2)) != 0);
    shellV[7].val = buffer2;
    strtok(shellV[7].val, "\n");
    pclose(fp2);

    var_count = 8;

    // Getting user input
    while((line = linenoise(shellV[1].val)) != NULL) {

        // Tokenise and parse command line
        dquotes = false;
        redirect = false;
        append = false;
        var_fail = false;
        i = 0;
        j = 0;
        k = 0;
        n = 0;
        arguments = 0;
        id_count = 0;
        q_count = 0;
        d_count = 0;
        char tokens[20][100] = {};
        char delimiters[20] = {};
        char *buffer3[100] = {};
        char filename[50] = {};
        char quoted_tokens[20][100] = {};
        int qt_ids[20];

        while(line[i] != '\0' && line[i] != '>') {
            tc_char = (int) line[i];
            // Checking if current text is in double quotes
            if(!dquotes) {
                // Checking for variable assignment
                if(line[i] == '=') {
                    expand = false;
                    var_fail = false;
                    assigning_dquotes = false;
                    // Checking if first character of shell variable name is a number
                    if(in_range(tokens[j][0], 48, 57)) var_fail = true;
                    // Checking for valid characters in shell variable name (ranges of numbers, uppercase alphabet, lowercase alphabet and underscore character)
                    else {
                        for(int z = 0; z < k; z++) {
                            if(!(in_range(tokens[j][z], 48, 57) || in_range(tokens[j][z], 65, 90) || in_range(tokens[j][z], 97, 122) || tokens[j][z] == '_')) var_fail = true;
                        }
                    }
                    buffer3[n] = tokens[j];
                    n++;
                    j++;
                    arguments++;
                    k = 0;
                    i++;
                    dquotes = false;
                    // Checking variable assignment errors
                    if(line[i] == '\0' || delim(line[i])) var_fail = true;
                    else {
                        while(line[i] != '\0' && var_fail != true) {
                            tc_char = (int) line[i];
                            // Checking for double quotes
                            if(tc_char == 34) {
                                if(dquotes == true) {
                                    // Checking double quote/var assignment error
                                    if(line[i+1] != '\0') {
                                        var_fail = true;
                                    } 
                                    // If all is good 
                                    else {
                                        dquotes = false;
                                        assigning_dquotes = true;
                                        break;
                                    }
                                }
                                dquotes = true;
                            } 
                            // Checking for '\' character
                            else if(tc_char == 92) {
                                i++;
                                tokens[j][k] = line[i];
                                k++;
                            }
                            // Checking for delimiters
                            else if(delim(line[i]) && !(dquotes)) {
                                var_fail = true;
                            }
                            // Inserting character
                            else {
                                tokens[j][k] = line[i];
                                k++;
                            } 
                            i++;
                        }
                        if(assigning_dquotes) {
                            strcpy(quoted_tokens[q_count], tokens[j]);
                            qt_ids[id_count] = j;
                            q_count++; 
                            id_count++;
                        }
                        buffer3[n] = tokens[j];
                        n++;
                    }
                }
                // Checking for delimeters
                else if(delim(line[i])) {
                    delimiters[d_count] = line[i];
                    d_count++;
                    j++;
                    arguments++;
                    k = 0;
                } // Checking for '\' character
                else if(tc_char == 92) {
                    i++;
                    tokens[j][k] = line[i];
                    k++;
                } // Checking for double quotes
                else if(tc_char == 34) {
                    dquotes = true;
                }  
                // Inserting character 
                else {
                    tokens[j][k] = line[i];
                    k++;
                }
            } else {
                // Checking for double quotes
                if(tc_char == 34) {
                    dquotes = false;
                    strcpy(quoted_tokens[q_count], tokens[j]);
                    qt_ids[id_count] = j;
                    q_count++;
                    id_count++;
                } // Checking for '\' character
                else if(tc_char == 92) {
                    i++;
                    tokens[j][k] = line[i];
                    k++;
                } 
                // Inserting character 
                else {
                    tokens[j][k] = line[i];
                    k++;
                }
            }
            i++; 
        }
        arguments++;

        // Checking if double quotes were not enclosed
        if(dquotes) {
            printf("Error occurred during parsing: Double quotes were not enclosed.\n");
        } else {
            // Expanding variables
            for(int i = 0; tokens[i][0] != '\0'; i++) {
                if(tokens[i][0] == '$') {
                    // Checking for variables enclosed in {}
                    if(tokens[i][1] == '{') {
                        char temp3[30] = {0};
                        char buffer4[30] = {0};
                        int z = 2;
                        int ct = 0;
                        while(tokens[i][z] != '}') {
                            strncat(temp3, &tokens[i][z], 1);
                            z++;
                        }
                        char *var_name = temp3;
                        z++;
                        while(tokens[i][z] != '\0') {
                            buffer4[ct] = tokens[i][z];
                            ct++;
                            z++;
                        }
                        var_expand(tokens, var_name, shellV, i);
                        strcat(tokens[i], buffer4);
                    } else {
                        char *var_name = strtok(tokens[i], "$"); 
                        var_expand(tokens, var_name, shellV, i);
                        // strcpy(var_name, temp3);
                    }
                }
            }
        
            int temp_count = 0;

            // Expanding variables inside quotes
            for(int i = 0; quoted_tokens[i][0] != '\0'; i++) {    
                if(!(q_expand(quoted_tokens[i], shellV))) {
                    printf("Error occured when expanding variables inside quotes\n");
                    break;
                } else {
                    strcpy(tokens[(qt_ids[temp_count])], quoted_tokens[i]);
                    temp_count++;
                }
            }

            // Checking for variable assignment
            if(var_fail) {
                printf("Syntax error in variable assignment.\n");
            } else {
                for(int i = 0; buffer3[i] != NULL; i+=2) {
                    if(!(var_assign(buffer3[i], buffer3[i+1], shellV, &var_count))) printf("Cannot add new variable, limit of shell variables is reached\n");
                }
            }

            // Checking for output redirection
            if(line[i] == '>') {
                redirect = true;
                t = true;
                // Checking for append operator
                if(line[i+1] == '>') {
                    append = true;
                    i++;
                }
                i++;
                for(int x = 0; line[i] != '\0'; x++) {
                    filename[x] = line[i];
                    i++;
                }
            }

        }

        // Free allocated memory
        linenoiseFree(line);

        // Checking for Output redirection
        if(redirect) {
            // Checking for append operator
            if(append) out_fp = freopen(filename, "a", stdout);
            else out_fp = freopen(filename, "w", stdout);
        }

        // Checking for internal commands

        char *cmd;
        cmd = tokens[0];
        
        // exit
        if((strcmp(cmd, "exit")) == 0) {
            // Checking for exit code
            if(arguments == 2) {
                // Free allocated memory
                free(shellV);
                if(t) fclose(out_fp);
                for(int i = 0; i < 20; i++) free(dir_stack[i]);
                printf("Exiting with exit code %d\n", atoi(tokens[1]));
                exit(atoi(tokens[1]));
            }

            // Checking for invalid no. of arguments
            else if(arguments > 2) {
                printf("Error: Too many arguments");
            }
            else {
                // Free allocated memory
                free(shellV);
                if(t) fclose(out_fp);
                for(int i = 0; i < 20; i++) free(dir_stack[i]);
                printf("Exiting with exit code %d\n", EXIT_SUCCESS);
                exit(EXIT_SUCCESS);
            }
        }
        // echo
        else if((strcmp(cmd, "echo")) == 0) {
            // Printing tokens out
            for(int i = 1; tokens[i][0] != '\0'; i++) {
                printf("%s", tokens[i]);
                printf("%c", delimiters[i]);
            }
            printf("\n");
        }
        // cd
        else if((strcmp(cmd, "cd")) == 0) {
            if(arguments == 2) {
                if(chdir(tokens[1]) != 0) perror("chdir() failed");
                else {
                    // Changing CWD variable
                    if(getcwd(curwd, PATH_MAX) == NULL) printf("getcwd() error");
                    if(!(var_assign("CWD", curwd, shellV, &var_count))) printf("Error when assigning value to variable");
                    else strcpy(dir_stack[d_pointer], get_shellV_val("CWD", shellV));
                }
            } else if(arguments > 2) {
                printf("Error: Too many arguments\n");
            }
        }
        // showvar
        else if((strcmp(cmd, "showvar")) == 0) {
            if(arguments == 1) {
                for(int i = 0; i < var_count; i++) {
                    printf("%s=%s\n", shellV[i].name, shellV[i].val);
                }
            } 
            // Checking for specific shell variable
            else if(arguments == 2) {
                if(get_shellV_val(tokens[1], shellV) == NULL) printf("Error: Variable not found\n");
                else printf("%s=%s\n", tokens[1], get_shellV_val(tokens[1], shellV));  
            } else {
                printf("Error: Too many arguments\n");
            }
        }
        // export
        else if((strcmp(cmd, "export")) == 0) {
            if(arguments > 2) {
                printf("Error: Too many arguments\n");
            } else {
                if(arguments == 2) {
                    char *var_value = get_shellV_val(tokens[1], shellV);
                    // Checking if shell variable exists
                    if(var_value == NULL) printf("Error: Variable not found\n");
                    // Checking if variable exists in the environment
                    else if(getenv(tokens[1]) != NULL) setenv(tokens[1], var_value, 1);
                    // If variable does not exist in the environment
                    else setenv(tokens[1], var_value, 0);
                }
            }
        }
        // unset
        else if((strcmp(cmd, "unset")) == 0) {
            if(arguments > 2) printf("Error: Too many arguments\n");
            else if(arguments == 2) unsetenv(tokens[1]);
        }
        // showenv
        else if((strcmp(cmd, "showenv")) == 0) {
            if(arguments > 2) printf("Error: Too many arguments\n");
            else if(arguments == 2) {
                // Checking if environment variable exists
                if(getenv(tokens[1]) == NULL) printf("Error: Environment variable not found\n");
                else printf("%s=%s\n", tokens[1], getenv(tokens[1]));
            } else {
                for(int i = 0; env[i] != NULL; i++) printf("%s\n", env[i]);
            }
        }
        // pwd
        else if((strcmp(cmd, "pwd")) == 0) printf("%s\n", dir_stack[d_pointer]);
        // pushd
        else if((strcmp(cmd, "pushd")) == 0) {
            if(arguments == 1) printf("Error: Specify file to push in directory stack\n");
            else if(arguments > 2) printf("Error: Too many arguments\n");
            else {
                int tmp = pushd(tokens[1], dir_stack, &d_pointer);
                // pushd succeeds
                if(tmp == 0) {
                    for(int i = d_pointer; i >= 0; i--) printf("%s ", dir_stack[i]);
                    printf("\n");
                    // Changing directories and CWD value
                    var_assign("CWD", dir_stack[d_pointer], shellV, &var_count);
                    if(chdir(dir_stack[d_pointer]) != 0) perror("chdir() failed");
                }
                else if(tmp == 1) printf("Error: Directory does not exist\n");
                else if(tmp == 2) printf("Error: pushd failed\n");
            }
        }
        // popd
        else if((strcmp(cmd, "popd")) == 0) {
            if(arguments > 1) printf("Error: Too many arguments\n");
            else if(popd(dir_stack, &d_pointer)) {
                for(int i = d_pointer; i >= 0; i--) printf("%s ", dir_stack[i]);
                printf("\n");
                // Changing directories and CWD value
                var_assign("CWD", dir_stack[d_pointer], shellV, &var_count);
                if(chdir(dir_stack[d_pointer]) != 0) perror("chdir() failed");
            } else printf("Error: Cannot remove CWD from directory stack\n");
        }
        // dirs
        else if((strcmp(cmd, "dirs")) == 0) {
            if(arguments > 1) printf("Error: Too many arguments\n");
            else {
                for(int i = d_pointer; i >= 0; i--) printf("%s ", dir_stack[i]);
                printf("\n");
            }
        }
        // source
        else if((strcmp(cmd, "source")) == 0) {
            if(arguments == 1) printf("Error: Specify file to read\n");
            else if(arguments > 2) printf("Error: Too many arguments\n");
            else {
                // Opening file
                FILE *fp3;
                fp3 = fopen(tokens[1], "r");
                if(!fp3) printf("Error: File not found in CWD\n");
                else {
                    // Executing commands from file
                    char content[1000];
                    while(fgets(content, 1000, fp3)!=NULL) system(content);
                    fclose(fp3);
                }
            }
        }
        // Searching for External Commands
        else if(buffer3[0] == NULL && buffer3[1] == NULL) {
            // Forking new child process
            pid_t pid = fork();
            if(pid == -1) perror("fork() failed\n");
            else if(pid == 0) {
                // Child
                char *args[20];
                for(int i = 0; tokens[i][0] != '\000'; i++) args[i] = tokens[i];
                if(execvp(args[0], args) == -1) {
                    perror("execv() failed\n");
                    exit(EXIT_FAILURE);
                }
            } else {
                // Parent
                int status;
                if(waitpid(pid, &status, 0) == -1) {
                    perror("wait() failed\n");
                    exit(EXIT_FAILURE);
                }
            }
        }

        if(redirect) fclose(out_fp);

    }

}
