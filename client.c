#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h> // for closing socket
#include <time.h>
#include <ctype.h>

#define TRUE 1
#define FALSE 0

#define RFC1123FMT "%a, %d %b %Y %H:%M:%S GMT" //If-Modified-Since format
#define PRINT_WRONG_CMD_USAGE "Usage: client [-h] [-d <timeinterval>] <URL>\n"
#define PRINT_WRONG_INPUT "Wrong input\n"
#define HEADERS_FLAG "-h"
#define DELAY_FLAG "-d"
#define TIME_INTERVAL_FORMAT "%d:%d:%d"
#define URL_FORMAT "%4s://%s" //format = Protocol://(Host[:port]/Filepath)
#define NUM_OF_CMD_ARGS 2 //not including flags & options
#define DEFAULT_PORT 80
#define MAX_PORT 65535

#define BUFFER_SIZE 512
#define REQUEST_SIZE 1024
#define RESPONSE_SIZE 1024

//Static Variables
static int sDelayFlag = FALSE;
static int sDelayIndex = -1;
static int sHeaderFlag = FALSE;
static int sHeadersIndex = -1;
static int sURLIndex = -1;
static int* sTimeInterval = NULL;
static char* sHost = NULL;
static char* sFilePath = NULL;
static int sPort = DEFAULT_PORT;

//Method Declarations
void parseArguments(int, char**);
int checkFlags(int, char**);
int* getTimeInterval(char*);
int verifyURL(char*);
int verifyPort(char*);
void executeCMD(char*);
void establishConnection(int*);
char* constructRequest(char*);
void sendRequest(int*, char*);
int getResponse(char**, int, int*);
void destroy();


/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

int main(int argc, char* argv[]) {

        if(argc < 2) {
                printf(PRINT_WRONG_CMD_USAGE);
                exit(-1);
        }

        parseArguments(argc, argv);

        executeCMD(argv[sURLIndex]);

        destroy();
        return 0;
}


/******************************************************************************/
/******************************* Main Methods *********************************/
/******************************************************************************/

//Parse & validate passed arguments
void parseArguments(int argc, char** argv) {

        if(argv == NULL) {
                printf(PRINT_WRONG_CMD_USAGE);
                exit(-1);
        }

        int num_of_flags = checkFlags(argc, argv);

        //Number of expected arguments (according to flags found)
        int num_of_tokens = NUM_OF_CMD_ARGS + num_of_flags;

        if(num_of_tokens != argc) {
                printf(PRINT_WRONG_CMD_USAGE);
                exit(-1);
        }

        if(sDelayFlag) {

                sTimeInterval = getTimeInterval(argv[sDelayIndex + 1]);
                if(sTimeInterval == NULL) {
                        printf(PRINT_WRONG_INPUT);
                        destroy();
                        exit(-1);
                }
        }

        if(verifyURL(argv[sURLIndex])) {
                printf(PRINT_WRONG_INPUT);
                destroy();
                exit(-1);
        }
}

/*********************************/
/*********************************/
/*********************************/

//Execute HTTP Request according to URL & arguments
void executeCMD(char* url) {

        int sockfd = 0;
        establishConnection(&sockfd);

        sendRequest(&sockfd, constructRequest(url));

        char* response = (char*)calloc(RESPONSE_SIZE, sizeof(char));
        if(response == NULL) {
                perror("calloc");
                destroy();
                exit(-1);
        }

        int read = getResponse(&response, RESPONSE_SIZE, &sockfd);

        printf("\n%s\n", response);
        printf("\n Total received response bytes: %d\n", read);

        free(response);
        close(sockfd);
}

/******************************************************************************/
/************************ executeCMD() Helper Methods *************************/
/******************************************************************************/

//Initialize socket structs and establish connection to server.
void establishConnection(int* sockfd) {

        if(((*sockfd) = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
                perror("socket");
                destroy();
                exit(-1);
        }
        struct sockaddr_in server_addr;
        memset(&server_addr, 0, sizeof(server_addr));

        struct hostent *hp;

        hp = gethostbyname(sHost);
        if(hp == NULL) {
                herror("gethostbyname");
                destroy();
                exit(-1);
        }

        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = ((struct in_addr*) hp->h_addr)->s_addr;
        server_addr.sin_port = htons(sPort);

        if(connect((*sockfd), (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
                perror("connect");
                destroy();
                exit(-1);
        }
}

/*********************************/
/*********************************/
/*********************************/

//construct HTTP request to be sent to server.
char* constructRequest(char* url) {

        const char* METHOD;
        char modified_since[256] = "If-Modified-Since: ";
        char port[10];

        char* request = (char*)calloc(REQUEST_SIZE + strlen(url), sizeof(char));
        if(request == NULL) {
                perror("calloc");
                destroy();
                exit(-1);
        }

        //get Port
        sprintf(port, ":%d", sPort);

        //Determine method
        if(sHeaderFlag)
                METHOD = "HEAD";
        else
                METHOD = "GET";

        //Construct If-Modified-Since time format
        if(sDelayFlag) {

                time_t now;
                char timebuf[128];
                now = time(NULL);
                // sTimeInterval = {days, hours, mins}
                now = now - (sTimeInterval[0] * 24 * 3600 + sTimeInterval[1] * 3600 + sTimeInterval[2] * 60);
                //timebuf will hold the correct format of the time
                strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&now));
                strcat(modified_since, timebuf);
                strcat(modified_since, "\r\n");
        }

        //Construct the request
        // sprintf(request, "%s /%s HTTP/1.0\r\nHost: %s%s\r\n%s\r\n",
        //         METHOD,
        //         sFilePath != NULL ? sFilePath : "",
        //         sHost,
        //         sPort != DEFAULT_PORT ? port : "",
        //         sDelayFlag ? modified_since : "");

        sprintf(request, "%s %s HTTP/1.0\r\n%s\r\n",
                METHOD,
                url,
                sDelayFlag ? modified_since : "");

        return request;
}

/*********************************/
/*********************************/
/*********************************/

//Send constructed request to server
void sendRequest(int* sockfd, char* request) {

        int request_length = strlen(request);
        int bytes_written = 0;
        int nBytes;

        printf("HTTP request =\n%s\nLEN = %d\n", request, request_length);

        while(bytes_written < request_length) {

                if((nBytes = write((*sockfd), request, strlen(request))) < 0) {
                        perror("write");
                        destroy();
                        exit(-1);
                }

                bytes_written += nBytes;
        }
        free(request);
}

/*********************************/
/*********************************/
/*********************************/

//read response from server & count total bytes read.
int getResponse(char** response, int response_length, int* sockfd) {

        int nBytes;
        int bytes_read = 0;
        char buffer[BUFFER_SIZE];
        memset(&buffer, 0, sizeof(buffer));

        char* temp;

        //Reading server response
        while ((nBytes = read((*sockfd), buffer, sizeof(buffer))) > 0) {

                if(nBytes < 0) {
                        perror("read");
                        destroy();
                        exit(-1);
                }
                //count total bytes
                bytes_read += nBytes;

                //if not enough space in Response, allocate more memory
                if(nBytes >= (response_length - bytes_read)) {

                        temp = (char*)realloc((*response), (response_length *= 2));
                        if(temp == NULL) {
                                perror("realloc");
                                destroy();
                                exit(-1);
                        }
                        (*response) = temp;

                }
                strncat((*response), buffer, nBytes);
        }
        return bytes_read;
}


/******************************************************************************/
/********************** parseArguments() Helper Methods ***********************/
/******************************************************************************/


//check arguments for flags and return how many additional arguments are expected
int checkFlags(int argc, char** argv) {

        int num_of_flags = 0;
        int i;

        for(i = 1; i < argc; i++) {

                if(!strcmp(argv[i], HEADERS_FLAG)) {
                        sHeaderFlag = TRUE;
                        sHeadersIndex = i;
                        num_of_flags += 1;

                } else if(!strcmp(argv[i], DELAY_FLAG)) {
                        sDelayFlag = TRUE;
                        sDelayIndex = i;
                        num_of_flags += 2;

                }
        }

        for(i = 1; i < argc; i++) {
                if(i != sDelayIndex && i != sDelayIndex + 1 && i != sHeadersIndex)
                        sURLIndex = i;
        }

        return num_of_flags;
}

/*********************************/
/*********************************/
/*********************************/

//get and validate time interval arguement
//used only if the Delay flag is found
int* getTimeInterval(char* interval_string) {

        if(interval_string == NULL)
                return NULL;

        int days, hours, mins;
        int assigned = sscanf(interval_string, TIME_INTERVAL_FORMAT, &days, &hours, &mins);

        //if interval_string matches format return int array with days, hours, mins.
        if(assigned == 3) {

                int* time_interval = (int*)calloc(3, sizeof(int));
                if(time_interval == NULL) {
                        perror("calloc");
                        destroy();
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
/*********************************/

//Verify that passed URL argument matches format
int verifyURL(char* url) {

        char protocol[4];
        char host_path[strlen(url)-4];
        //seperate protocol & host+path
        int assigned = sscanf(url, URL_FORMAT, protocol, host_path);

        char* path_ptr = strchr(host_path, '/');

        //convert protocol to lowercase to support uppercase\mixedcase input
        int i;
        for(i = 0; protocol[i] != '\0'; i++)
                protocol[i] = tolower(protocol[i]);

        //check & verify URL format & correct protocol
        if(assigned != 2 || strcmp(protocol, "http"))
                return -1;

        //check & verify port
        //Determine length of Host string.
        char* port_ptr;
        int hostLength = 0;
        if((port_ptr = strchr(host_path, ':')) != NULL) {

                sPort = verifyPort(port_ptr);
                if(sPort == -1)
                        return -1;

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

        //get file Path
        if(path_ptr != NULL) {

                int filePathLength = strlen(path_ptr + 1);
                sFilePath = (char*)calloc(filePathLength + 1, sizeof(char));
                strncpy(sFilePath, path_ptr + 1, filePathLength);

        }

        return 0;
}

/*********************************/
/*********************************/
/*********************************/

//if port was found, verify its format
int verifyPort(char* port_ptr) {

        int i;
        for(i = 0; port_ptr[i] != '/' && port_ptr[i] != '\0'; i++) ;  //find end of port

        int port_length = i-1;
        //port is a maximum of 5 digits (max = 65535)
        if(port_length <= 0 || port_length > 5)
                return -1;


        char* port_string = (char*)calloc(port_length + 1, sizeof(char));
        if(port_string == NULL) {
                perror("calloc");
                destroy();
                exit(-1);
        }

        strncpy(port_string, port_ptr + 1, port_length);

        //check port_string containts only digits
        int assigned = strspn(port_string, "0123456789");

        if(assigned != port_length) {
                free(port_string);
                return -1;
        }

        int port = atoi(port_string);

        if(port > MAX_PORT) {
                free(port_string);
                return -1;
        }

        free(port_string);
        return port;
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

void destroy() {

        free(sTimeInterval);
        free(sHost);
        free(sFilePath);

}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
