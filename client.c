#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h> // for close
#include <arpa/inet.h> //DEBUG

#define TRUE 1
#define FALSE 0

#define RFC1123FMT "%a, %d %b %Y %H:%M:%S GMT"
#define PRINT_WRONG_CMD_USAGE "Usage: client [-h] [-d <timeinterval>] <URL>\n"
#define PRINT_WRONG_INPUT "wrong input\n"
#define HEADERS_FLAG "-h"
#define DELAY_FLAG "-d"
#define TIME_INTERVAL_FORMAT "%2d:%2d:%2d"
#define URL_FORMAT "%4s://%s" //format = Protocol://Host[:port]/Filepath
#define NUM_OF_CMD_ARGS 2 //not including flags & options
#define DEFAULT_PORT 80

#define BUFFER_SIZE 512
#define REQUEST_SIZE 1024
#define RESPONSE_SIZE 1024

static int sDelayFlag = FALSE;
static int sDelayIndex = -1;
static int sHeadersFlag = FALSE;
static int sHeadersIndex = -1;
static int sURLIndex = -1;
static int* sTimeInterval = NULL;
static char* sHost = NULL;
static char* sFilePath = NULL;
static int sPort = DEFAULT_PORT;

//Method Declarations
int parseArguments(int, char**);
int checkFlags(int, char**);
int* getTimeInterval(char*);
int verifyURL(char*);
int verifyPort(char*);
int executeCMD(int, char**);

//DEBUG Methods

//TODO add support for HTTPS urls
//TODO fix issue when url contains ampersand
//TODO add If-Modified-Since header

/******************************************************************************/
/******************************************************************************/

int main(int argc, char* argv[]) {

        printf("\n********************\n**** Main START ****\n********************\n"); //DEBUG

        if(argc < 2) {
                printf(PRINT_WRONG_CMD_USAGE);
                exit(-1);
        }

        if(parseArguments(argc, argv)) {
                exit(-1);
        }

        int result = executeCMD(argc, argv); //DEBUG - remove assignment
        printf("executeCMD result = %d\n", result); //DEBIG


        free(sTimeInterval);
        free(sHost);
        free(sFilePath);
        printf("\n********************\n***** Main END *****\n********************\n"); //DEBUG
        return 0;
}


/******************************************************************************/
/*************************** CMD Execution Methods ****************************/
/******************************************************************************/


int executeCMD(int argc, char** argv) {
        printf("\n\n\n"); //DEBUG
        int sockfd;
        if((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
                perror("socket");
                exit(-1);
        }
        struct sockaddr_in server_addr;
        memset(&server_addr, 0, sizeof(server_addr));

        struct hostent *hp;
        hp = gethostbyname(sHost);
        if(hp == NULL)
                return -1;

        printf("hostnent IP address = %s\n", inet_ntoa(*(struct in_addr*)hp->h_addr_list[0])); //DEBUG
        printf("**********HP - name = %s\taddressType = %d\n**********length = %d\n", hp->h_name, hp->h_addrtype, hp->h_length); //DEBUG

        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = ((struct in_addr*) hp->h_addr)->s_addr;
        printf("server_addr IP address = %s\n", inet_ntoa(server_addr.sin_addr)); //DEBUG
        server_addr.sin_port = htons(sPort);
        printf("address = %u, port = %u\n", server_addr.sin_addr.s_addr, server_addr.sin_port); //DEBUG

        if(connect(sockfd, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
                perror("connect");
                exit(-1);
        }

        int nBytes;
        char buffer[BUFFER_SIZE];
        memset(&buffer, 0, sizeof(buffer));
        char request[REQUEST_SIZE];
        memset(request, 0, sizeof(request));

        int response_length = RESPONSE_SIZE;
        int bytes_read = 0;
        char* response = (char*)calloc(response_length, sizeof(char));
        if(response == NULL) {
                perror("calloc");
                exit(-1);
        }



        //Constructing HTTP Request
        // char* request = (char*)calloc(request_length, sizeof(char));
        if(sHeadersFlag)
                strcat(request, "HEAD ");
        else
                strcat(request, "GET ");

        strcat(request, argv[sURLIndex]);
        strcat(request, " HTTP/1.0\r\n\r\n");

        // printf("request = \n%s, length = %d\n", request, (int)strlen(request)); //DEBUG


        //Sending Request
        printf("HTTP request =\n%s\nLEN = %d\n", request, (int)strlen(request)); //THIS IS NOT(!!!) DEBUG
        if( write(sockfd , request , strlen(request)) < 0) {
                perror("write");
                exit(-1);
        }


        //Reading server response
        while ((nBytes = read(sockfd, buffer, sizeof(buffer))) > 0) {
                printf("Im Reading...\n"); //DEBUG
                if(nBytes < 0) {
                        perror("read");
                        exit(-1);
                }
                bytes_read += nBytes;
                printf("nBytes(bytes read now) = %d, bytes_read(total) = %d\n", nBytes, bytes_read); //DEBUG
                // printf("buffer = %s\n", buffer); //DEBUG

                // printf("buffer = %s\n", buffer); //DEBUG
                printf("free space = %d\n", response_length - bytes_read); //DEBUG
                if(nBytes >= (response_length - bytes_read)) {
                        printf("Reallocating Reponse - inc size to %d\n", response_length * 2); //DEBUG
                        response = realloc(response, (response_length *= 2));
                        if(response == NULL) {
                                perror("realloc");
                                exit(-1);
                        }

                }
                strncat(response, buffer, nBytes);
                // strcat(response, buffer);

        }


        printf("\n********************\n****** RESPONSE ******\n********************\n\n%s\n\n********************\n\n", response); //DEBUG
        free(response);
        close(sockfd);
        return 0;
}


/******************************************************************************/
/************************** Input Validation Methods **************************/
/******************************************************************************/

//Parse & validate passed arguments
int parseArguments(int argc, char** argv) {

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
                        printf(PRINT_WRONG_INPUT);
                        return -1;
                }

        }

        if(verifyURL(argv[sURLIndex])) {
                printf(PRINT_WRONG_INPUT);
                free(sTimeInterval);
                return -1;
        }

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
                if(i != sDelayIndex && i != sDelayIndex + 1 && i != sHeadersIndex)
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
                        perror("calloc");
                        exit(-1);
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
        //seperate protocol & host+path
        int assigned = sscanf(url, URL_FORMAT, protocol, host_path);

        char* path_ptr = strchr(host_path, '/');

        printf("Url = %s, length = %d\nURL Format - assigned = %d\n", url, (int)strlen(url), assigned); //DEBUG
        printf("protocol = \"%s\"\thost_path = \"%s\"\n", protocol, host_path); //DEBUG
        //check & verify URL format & correct protocol
        if(assigned != 2 || strcmp(protocol, "http"))
                return -1;

        //check & verify port
        char* port_ptr;
        int hostLength = 0;
        if((port_ptr = strchr(host_path, ':')) != NULL) {

                sPort = verifyPort(port_ptr);
                if(sPort == -1)
                        return -1;

                printf("port_ptr = %s, its length = %d\n", port_ptr, (int)strlen(port_ptr)); //DEBUG
                hostLength = strlen(host_path) - strlen(port_ptr);

        } else {

                if(path_ptr == NULL) {
                        hostLength = strlen(host_path);

                } else {
                        hostLength = strlen(host_path) - strlen(path_ptr);
                }

        }

        //get Host address
        sHost = (char*)calloc(hostLength + 1, sizeof(char));
        if(sHost == NULL)
                return -1;

        strncpy(sHost, host_path, hostLength);
        printf("sHost = %s, length = %d\n", sHost, (int)strlen(sHost)); //DEBUG

        //get file Path
        if(path_ptr != NULL) {

                int filePathLength = strlen(path_ptr + 1);
                sFilePath = (char*)calloc(filePathLength + 1, sizeof(char));
                strncpy(sFilePath, path_ptr + 1, filePathLength);
                printf("sFilePath = %s, length = %d\n", sFilePath, (int)strlen(sFilePath)); //DEBUG

        }

        printf("port = %d\n", sPort); //DEBUG

        return 0;
}

/*********************************/
/*********************************/

//if port was found, verify its format
int verifyPort(char* port_ptr) {

        int i;
        for(i = 0; port_ptr[i] != '/' && port_ptr[i] != '\0'; i++) ;  //find end of port

        int port_length = i-1;
        //port is a maximum of 4 digits
        if(port_length > 4)
                return -1;


        char* port_string = (char*)calloc(port_length + 1, sizeof(char));
        if(port_string == NULL) {
                perror("calloc");
                exit(-1);
        }

        strncpy(port_string, port_ptr + 1, port_length);
        printf("i-1 = %d\tport_string = %s\n", i-1, port_string); //DEBUG

        //check port_string containts only digits
        int assigned = strspn(port_string, "0123456789");
        printf("span = %d\n", assigned); //DEBUG
        if(assigned != port_length)
                return -1;

        int port = atoi(port_string);
        free(port_string);
        return port;
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/


/****************************/
/****** DEBUG Methods *******/
/****************************/
