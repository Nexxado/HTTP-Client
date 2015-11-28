#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>


#define RFC1123FMT "%a, %d %b %Y %H:%M:%S GMT"
#define PRINT_WRONG_CMD_USAGE "Usage: client [-h] [-d <timeinterval>] <URL>\n"
#define PRINT_WRONG_INPUT "wrong input\n"
#define HEADERS_FLAG "-h"
#define DELAY_FLAG "-d"
#define NUM_OF_CMD_ARGS 2 //not including flags & their options
#define CMD_PREFIX "client"

//Method Declarations
void initClient();
char* getCMD();
char** parseCMD(char*);
int checkFlags(char*);
int checkTimeIntervalFormat(char*, char*);
void destroy_tokens(char**);

//DEBUG Methods
void printTokens(char**);

/******************************************************************************/
/******************************************************************************/

int main() {

        initClient();

        return 0;
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

void initClient() {

        char* input;
        char** cmdTokens = NULL;

        do {

                destroy_tokens(cmdTokens);
                printf("cmd: ");
                input = getCMD();
                // printf("input = %s\n", input); //DEBUG

        } while((cmdTokens = parseCMD(input)) == NULL);

        // printTokens(cmdTokens); //DEBUG



        free(input);
        destroy_tokens(cmdTokens);
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/


//Method to handle user input of unknown length while avoiding buffer overflow
char* getCMD() {

        char* input;
        int ch;
        int size = 64; //initial size
        int size_increment = 16; //allocate this much additional size
        int length = 0;

        input = realloc(NULL, size * sizeof(char));
        if(input == NULL) {
                perror("realloc");
                exit(-1);
        }

        while(EOF != (ch = fgetc(stdin)) && ch != '\n') {

                input[length++] = ch;

                if(length == size) {

                        input = realloc(input, (size += size_increment) * sizeof(char));
                        if(input == NULL) {
                                perror("realloc");
                                exit(-1);
                        }

                }
        }

        input[length++] = '\0';

        return realloc(input, length * sizeof(char));
}

/*********************************/
/*********************************/

//divides user input into tokens
char** parseCMD(char* input) {

        if(input == NULL) {
                printf(PRINT_WRONG_CMD_USAGE);
                return NULL;
        }

        int num_of_flags = checkFlags(input);
        if(num_of_flags == -1) {
                printf(PRINT_WRONG_CMD_USAGE);
                free(input);
                return NULL;
        }

        //Number of expected arguments (according to flags found)
        int num_of_tokens = NUM_OF_CMD_ARGS + num_of_flags;
        printf("num of tokens = %d\n", num_of_tokens); //DEBUG

        char** cmdTokens = (char**)calloc((num_of_tokens + 1), sizeof(char*));
        if(cmdTokens == NULL) {
                perror("calloc");
                exit(-1);
        }

        char* delimiters = " \n";
        char* temp = strtok(input, delimiters);
        int i = 0;

        //check if first arg isnt 'client'
        if(strcmp(temp, CMD_PREFIX)) {
                printf(PRINT_WRONG_CMD_USAGE);
                free(cmdTokens);
                free(input);
                return NULL;
        }

        //extract cmd args as tokens
        while(temp != NULL && i < num_of_tokens) {

                cmdTokens[i] = (char*)calloc((strlen(temp) + 1), sizeof(char));
                if(cmdTokens[i] == NULL) {
                        perror("calloc");
                        exit(-1);
                }

                strcpy(cmdTokens[i], temp);
                cmdTokens[i][strlen(temp)] = '\0';
                temp = strtok(NULL, delimiters); //move temp to the next token

                i++;
        }

        cmdTokens[i] = NULL; //tokens ends in NULL

        printf("i = %d\n", i); //DEBUG
        //check if too few\many arguments were passed
        if(i != num_of_tokens || temp != NULL) {
                printf(PRINT_WRONG_CMD_USAGE);
                destroy_tokens(cmdTokens);
                free(input);
                return NULL;
        }

        //TODO verify URL 

        return cmdTokens;
}

/*********************************/
/*********************************/

//check flags and return how many addiotional arguments are expected
int checkFlags(char* input) {

        int num_of_flags = 0;

        char* flag = strstr(input, HEADERS_FLAG);
        if(flag != NULL) {

                //check for whitespaces before & after flag
                if(flag[-1] == ' ' && flag[2] == ' ') {
                        num_of_flags += 1;
                }
        }

        flag = strstr(input, DELAY_FLAG);
        if(flag != NULL) {

                if(flag[-1] == ' ' && flag[2] == ' ') {

                        if(!checkTimeIntervalFormat(input, flag)) {

                                num_of_flags += 2;
                        } else {
                                return -1;
                        }
                }
        }
        return num_of_flags;
}

/*********************************/
/*********************************/

int checkTimeIntervalFormat(char* input, char* flag_ptr) {

        char interval_string[8];
        strncpy(interval_string, flag_ptr + 3, 8);
        printf("interval string = \"%s\"\n", interval_string); //DEBUG

        //TODO find way to verify format using return value without assigning it to variables
        int days, hours, mins;
        int assigned = sscanf(interval_string, "%2d:%2d:%2d", &days, &hours, &mins);
        printf("assigned = %d\n", assigned); //DEBUG
        printf("days = \"%d\"\thours = \"%d\"\t mins = \"%d\"\n", days, hours, mins); //DEBUG
        return assigned == 3 ? 0 : -1;
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

void destroy_tokens(char** cmdTokens) {
        // printf("Destroying tokens\n"); //DEBUG
        if(cmdTokens == NULL)
                return;

        int i = 0;

        while(cmdTokens[i] != NULL) {
                // printf("token[%d]\n", i); //DEBUG
                free(cmdTokens[i]);
                i++;
        }
        free(cmdTokens);
}

/****************************/
/****** DEBUG Methods *******/
/****************************/

void printTokens(char** cmdTokens) {
        printf("Tokens: ");
        if (cmdTokens == NULL)
                return;

        int i = 0;

        while(cmdTokens[i] != NULL) {
                printf("%s\t", cmdTokens[i++]);
        }

        printf("\n");

}
