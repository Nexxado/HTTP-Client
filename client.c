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
#define URL_FORMAT "%4s://%s"//format = Protocol://Host[:port]/Filepath
#define NUM_OF_CMD_ARGS 2 //not including flags & options
#define DEFAULT_PORT 80

static int sDelayFlag = FALSE;
static int sDelayIndex = -1;
static int sHeadersFlag = FALSE;
static int sHeadersIndex = -1;
static int sURLIndex = -1;
static int* sTimeInterval = NULL;

//Method Declarations
int parseCMD(int, char**);
int checkFlags(int, char**);
int* getTimeInterval(char*);
int verifyURL(char*);
int verifyPort(char*);

//DEBUG Methods

/******************************************************************************/
/******************************************************************************/

int main(int argc, char* argv[]) {

        printf("\n********************\n**** Main START ****\n********************\n");

        if(argc < 2) {
                printf(PRINT_WRONG_CMD_USAGE);
                exit(-1);
        }

        if(parseCMD(argc, argv)) {
                exit(-1);
        }

        //TODO Implement executeCMD();

        printf("\n********************\n***** Main END *****\n********************\n");
        return 0;
}


/******************************************************************************/
/*************************** CMD Execution Methods ****************************/
/******************************************************************************/


/******************************************************************************/
/************************** Input Validation Methods **************************/
/******************************************************************************/

//Parse & validate passed arguments
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

        if(sDelayFlag) {

                sTimeInterval = getTimeInterval(argv[sDelayIndex + 1]);

                if(sTimeInterval == NULL) {
                        printf("interval == NULL\n"); //DEBUG
                        printf(PRINT_WRONG_CMD_USAGE);
                        return -1;
                }

        }

        if(verifyURL(argv[sURLIndex])) {
                printf(PRINT_WRONG_INPUT);
                free(sTimeInterval);
                return -1;
        }

        // if(interval != NULL) {
        //         free(interval);
        // }
        return 0;
}

/*********************************/
/*********************************/

//check arguments for flags and return how many additional arguments are expected
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

//get and validate time interval arguement
//used only if the Delay flag is found
int* getTimeInterval(char* interval_string) {

        if(interval_string == NULL)
                return NULL;

        int days, hours, mins;
        int assigned = sscanf(interval_string, TIME_INTERVAL_FORMAT, &days, &hours, &mins);
        printf("assigned = %d\n", assigned); //DEBUG
        printf("days = \"%d\"\thours = \"%d\"\t mins = \"%d\"\n", days, hours, mins); //DEBUG

        //if interval_string matches format return int array with days, hours, mins.
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

//Verify that passed URL argument matches format
int verifyURL(char* url) {

        char protocol[4];
        char host_path[strlen(url)-4];
        int port = DEFAULT_PORT;
        int assigned = sscanf(url, URL_FORMAT, protocol, host_path);

        printf("Url = %s\nURL Format - assigned = %d\n", url, assigned); //DEBUG
        printf("protocol = \"%s\"\thost_path = \"%s\"\n", protocol, host_path); //DEBUG
        if(assigned != 2 || strcmp(protocol, "http"))
                return -1;

        //check & verify port
        char* port_ptr;
        if((port_ptr = strchr(host_path, ':')) != NULL) {

                port = verifyPort(port_ptr);
                if(port == -1) {
                        return -1;
                }
        }

        printf("port = %d\n", port); //DEBUG

        return 0;
}

/*********************************/
/*********************************/

//if port was found, verify its format
int verifyPort(char* port_ptr) {

        int i;
        for(i = 0; port_ptr[i] != '/' && port_ptr[i] != '\0'; i++); //find end of port

        int port_length = i-1;
        //port is a maximum of 4 digits
        if(port_length > 4)
                return -1;


        char port_string[port_length];
        strncpy(port_string, port_ptr + 1, port_length);
        printf("i-1 = %d\tport_string = %s\n", i-1, port_string); //DEBUG

        //check port_string containts only digits
        int assigned = strspn(port_string, "0123456789");
        printf("span = %d\n", assigned); //DEBUG
        if(assigned != port_length)
                return -1;

        return atoi(port_string);
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/


/****************************/
/****** DEBUG Methods *******/
/****************************/
