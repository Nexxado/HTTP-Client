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
    printf("input = %s\n", input); //DEBUG

  } while((cmdTokens = parseCMD(input)) == NULL);

  printTokens(cmdTokens); //DEBUG




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

  int num_of_tokens = NUM_OF_CMD_ARGS + checkFlags(input);

  char** cmdTokens = (char**)malloc((num_of_tokens + 1) * sizeof(char*));
  if(cmdTokens == NULL) {
    perror("malloc");
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

  while(temp != NULL) {

    //check if too many arguments were passed
    if(i == num_of_tokens) {
      printf(PRINT_WRONG_CMD_USAGE);
      destroy_tokens(cmdTokens);
      free(input);
      return NULL;
    }

    cmdTokens[i] = (char*)malloc((strlen(temp) + 1) * sizeof(char));
    if(cmdTokens[i] == NULL) {
      perror("malloc");
      exit(-1);
    }

    strcpy(cmdTokens[i], temp);
    cmdTokens[i][strlen(temp)] = '\0';
    temp = strtok(NULL, delimiters); //move temp to the next token

    i++;
  }

  cmdTokens[i] = NULL; //tokens ends in NULL

  if(i < num_of_tokens) {
    printf(PRINT_WRONG_CMD_USAGE);
    destroy_tokens(cmdTokens);
    free(input);
    return NULL;
  }

  free(input);
  return cmdTokens;
}

/*********************************/
/*********************************/


int checkFlags(char* input) {

  int num_of_flags = 0;

  char* flag = strstr(input, HEADERS_FLAG);
  if(flag != NULL) {
    num_of_flags = 1;
  }

  flag = strstr(input, DELAY_FLAG);
  if(flag != NULL) {

    if(num_of_flags > 0) {
      num_of_flags = 3;

    } else {

      num_of_flags = 2;

    }
  }
  return num_of_flags;
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

void destroy_tokens(char** cmdTokens) {
  printf("Destroying tokens\n"); //DEBUG
  if(cmdTokens == NULL)
    return;

  int i;

  while(cmdTokens[i] != NULL) {
    printf("token[%d]\n", i); //DEBUG
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
