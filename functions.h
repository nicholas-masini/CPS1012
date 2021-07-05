#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <limits.h>

#define LEN 20
#define SHELL_VARIABLES_LIMIT 30

// Shell variables struct
typedef struct Shell_Variables {
    char *name;
    char *val;
} shell;

// Function to check certain ranges of ASCII characters, used mostly in input parsing for alphabet, numbers etc.
bool in_range(char input, int low, int high) {
    if(((int) input) >= low && ((int) input) <= high) return true;
    else return false;
}

// Function to define delimiter statement
bool delim(char character) {
    if(character == '|' || character == ';' || character == '<' || character == '>' || character == ' ' || character == '\t') return true;
    else return false;
}

// Function is called on startup to initialise shell variables
void init_shellV(shell *shellV, char *var_name) {  
        shellV->name = var_name;
        shellV->val = getenv(var_name); 
}

// Function used to get shell variable value based on name
char *get_shellV_val(char *var_name, shell *shellV) {
    for(int i = 0; shellV[i].name != NULL; i++) {
        if(i > SHELL_VARIABLES_LIMIT) {
            return NULL;
        } else if(strcmp(shellV[i].name, var_name) == 0) {
            return shellV[i].val;
        }
    }
    return NULL;
}

// Function to assign variable name to value
bool var_assign(char *var_name, char *var_value, shell *shellV, int *var_count) {
    // Checking if variable is already initiliased
    int l = 0;
    bool exist_var = false;
    bool limit_var = false;
    // Iterating through all shell variables
    while(shellV[l].name != NULL) {
        if(l > SHELL_VARIABLES_LIMIT) {
            limit_var = true;
            break;
        }
        // Assinging variable new value
        if((strcmp(shellV[l].name, var_name)) == 0) {
            strcpy(shellV[l].val, var_value);
            exist_var = true;
            return true;
        }
        l++;
    }
    // If variable does not exist
    if(!(exist_var)) {
        // Checking if limit of shell variables is reached
        if(limit_var) {
            return false;
        } 
        // Creating new variable
        else {
            shellV[l].name = realloc(shellV[l].name, (strlen(var_name) + 1));
            shellV[l].val = realloc(shellV[l].val, (strlen(var_value) + 1));
            strcpy(shellV[l].name, var_name);
            strcpy(shellV[l].val, var_value);
            (*var_count)++;
            return true;
        }
    }
}

// Function to expand variables inside quoted tokens
bool q_expand(char *quoted_token, shell *shellV) {
    char *new_token = malloc(sizeof(char) * 100);
    bool var_exists;
    int j = 0;
    int k, l;

    char temp_var[100];
    char var_val[100];

    *new_token = '\0';
    
    for(int i = 0; i < strlen(quoted_token); i++) {
        if(quoted_token[i] == '$') {
            var_exists = false;
            memset(temp_var, 0, 100);
            memset(var_val, 0, 100);
            k = 0;
            l = 0;
            i++;
            // Checking if first character is number
            if(in_range(quoted_token[i], 48, 57)) return false;
            // Reading variable name until unacceptable character is encountered
            while((in_range(quoted_token[i], 48, 57) || in_range(quoted_token[i], 65, 90) || in_range(quoted_token[i], 97, 122) || quoted_token[i] == '_')) {
                temp_var[k] = quoted_token[i];
                k++;
                i++;
            }
            temp_var[k+1] = '\0';
            while(shellV[l].name != NULL) {
                if(strcmp(shellV[l].name, temp_var) == 0) {
                    strcpy(var_val, shellV[l].val);
                    var_exists = true;
                    break;
                }
                l++;
            }
            if(!var_exists) return false;
            strcat(new_token, var_val);
            j = (strlen(new_token)+1);
            strncat(new_token, &quoted_token[i], 1);
        } else {
            strncat(new_token, &quoted_token[i], 1);
            j++;
        }
    }
    strcpy(quoted_token, new_token);
    free(new_token);
    return true;
}

// Function to expand variable
void var_expand(char tokens[20][100], char *var_name, shell *shellV, int i) {
    // Expanding variable
    int m = 0;
    while(shellV[m].name != NULL) {
        if(strcmp(shellV[m].name, var_name) == 0) {
            strcpy(tokens[i], shellV[m].val);
            return;
        }
        m++;
    }
    return;
}

// Function for pushd command
int pushd(char *new_dir, char **dir_stack, int *d_pointer) {
    // Checking if directory exists
    DIR *dir = opendir(new_dir);
    if(dir) {
        (*d_pointer)++;
        strcpy(dir_stack[*d_pointer], new_dir);
        return 0;
    } else if(ENOENT == errno) return 1;
    else return 2;
}

// Function for popd command
bool popd(char **dir_stack, int *d_pointer) {
    // Checking if directory stack contains only one element
    if(*d_pointer == 0) return false;
    else {
        strcpy(dir_stack[*d_pointer], "");
        (*d_pointer)--;
        return true;
    }
}