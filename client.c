#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>

#define TRUE 1
#define FALSE 0

#define RFC1123FMT "%a, %d %b %Y %H:%M:%S GMT"
#define PRINT_WRONG_CMD_USAGE "Usage: client [-h] [-d <timeinterval>] <URL>\n"
#define PRINT_WRONG_INPUT "wrong input\n"
#define HEADERS_FLAG "-h"
#define DELAY_FLAG "-d"
#define TIME_INTERVAL_FORMAT "%2d:%2d:%2d"
// #define URL_FORMAT //TODO implement URL Format
#define NUM_OF_CMD_ARGS 2 //not including flags & options
#define DEFAULT_PORT 80

static int sDelayFlag = FALSE;
static int sDelayIndex = -1;
static int sHeadersFlag = FALSE;
static int sHeadersIndex = -1;
static int sURLIndex = -1;

//Method Declarations
int parseCMD(int, char**);
int checkFlags(int, char**);
int* checkTimeIntervalFormat(char*);
int verifyURL(char*);

//DEBUG Methods

/******************************************************************************/
/******************************************************************************/

int main(int argc, char* argv[]) {

        if(argc < 2) {
                printf(PRINT_WRONG_CMD_USAGE);
                exit(-1);
        }

        if(parseCMD(argc, argv)) {
                exit(-1);
        }

        //TODO Implement executeCMD();

        return 0;
}


/******************************************************************************/
/************************** Input Validation Methods **************************/
/******************************************************************************/

//divides user input into tokens
int parseCMD(int argc, char** argv) {

        if(argv == NULL) {
                printf("argv = NULL\n"); //DEBUG
                printf(PRINT_WRONG_CMD_USAGE);
                return -1;
        }

        int num_of_flags = checkFlags(argc, argv);

        //Number of expected arguments (according to flags found)
        int num_of_tokens = NUM_OF_CMD_ARGS + num_of_flags;
        printf("num of tokens = %d, argc = %d\n", num_of_tokens, argc); //DEBUG

        if(num_of_tokens != argc) {
                printf(PRINT_WRONG_CMD_USAGE);
                return -1;
        }

        int* interval = NULL;
        if(sDelayFlag) {

                interval = checkTimeIntervalFormat(argv[sDelayIndex + 1]);

                if(interval == NULL) {
                        printf("interval == NULL\n"); //DEBUG
                        printf(PRINT_WRONG_CMD_USAGE);
                        return -1;
                }

        }

        if(verifyURL(argv[sURLIndex])) {
                printf(PRINT_WRONG_INPUT);
                free(interval);
                return -1;
        }

        if(interval != NULL) {
                free(interval);
        }
        return 0;
}

/*********************************/
/*********************************/

//check flags and return how many addiotional arguments are expected
// param input - the command the user submitted
int checkFlags(int argc, char** argv) {

        int num_of_flags = 0;
        int i;

        for(i = 1; i < argc; i++) {

                if(!strcmp(argv[i], HEADERS_FLAG)) {
                        sHeadersFlag = TRUE;
                        sHeadersIndex = i;
                        printf("Header Flag Found, index = %d\n", sHeadersIndex); //DEBUG
                        num_of_flags += 1;

                } else if(!strcmp(argv[i], DELAY_FLAG)) {
                        sDelayFlag = TRUE;
                        sDelayIndex = i;
                        printf("Delay Flag Found, index = %d\n", sDelayIndex); //DEBUG
                        num_of_flags += 2;

                }
        }

        for(i = 1; i < argc; i++) {
                if(i != sDelayIndex && i != sDelayIndex+1 && i != sHeadersIndex)
                        sURLIndex = i;
        }
        printf("URL Index = %d\n", sURLIndex); //DEBUG

        return num_of_flags;
}

/*********************************/
/*********************************/

int* checkTimeIntervalFormat(char* interval_string) {

        if(interval_string == NULL)
                return NULL;

        int days, hours, mins;
        int assigned = sscanf(interval_string, TIME_INTERVAL_FORMAT, &days, &hours, &mins);
        printf("assigned = %d\n", assigned); //DEBUG
        printf("days = \"%d\"\thours = \"%d\"\t mins = \"%d\"\n", days, hours, mins); //DEBUG

        if(assigned == 3) {

                int* time_interval = (int*)calloc(3, sizeof(int));
                if(time_interval == NULL) {
                        return NULL;
                }

                time_interval[0] = days;
                time_interval[1] = hours;
                time_interval[2] = mins;

                return time_interval;
        }

        return NULL;
}

/*********************************/
/*********************************/

int verifyURL(char* url) {

        return 0;
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/


/****************************/
/****** DEBUG Methods *******/
/****************************/
