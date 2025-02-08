//Primitive http

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <regex.h>
#include <netdb.h>
#include <unistd.h>

#include "./globalConfig.h"

#define PORT                4200
#define BUFFER_SIZE         1024
#define MAX_ROUTE_LEN       128  
#define MAX_PAGES           1  
#define PRIVATE_IP_ADDRESS  "192.168.0.17"

typedef char* (*EndpointFunc)(void*);
typedef void* (*ExtractQueryParams)(char*, short);

int server_sock;
int client_sock;

typedef struct {
    char route[MAX_ROUTE_LEN];
    EndpointFunc endpointFunc;
    ExtractQueryParams extractQueryParams;
} Endpoint;

Endpoint* endpoints;

Endpoint* allocEndpoints() {
    Endpoint *endpoints = malloc(MAX_PAGES * sizeof(Endpoint));
    if (!endpoints) {
        perror("Failed to allocate memory for Endpoint");
        return NULL;
    }
    return endpoints;
}

int calcParamCount(char* query, int queryLength, char charToCount) {
    int appearanceCount = 1;
    for (int i = 0; i < queryLength; i++) {
        if (query[i] == charToCount) {
            appearanceCount++;
        }
    }
    return appearanceCount;
}

short matchEndpoint(char* definedRoute, char* query, int queryLength) {
    regex_t regex;
    int ret = regcomp(&regex, definedRoute, REG_EXTENDED);

    if (ret) {
        printf("Could not compile regex\n");
        return 1;
    }

    ret = regexec(&regex, query, 0, NULL, 0);

    regfree(&regex);

    return ret;
}

void closeHttpConnection() {
    printf("\nSignal received, closing server socket...\n");
    if (server_sock != -1) {
        close(client_sock);
        close(server_sock);
        printf("Server socket closed.\n");
    }
}

void handle_request(int client_sock) {
    char buffer[BUFFER_SIZE];

    int read_size = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
    if (read_size < 0) {
        perror("recv failed");
        return;
    }
    buffer[read_size] = '\0';

    char method[16], http_version[16];
    char * route;
    if (sscanf(buffer, "%s %s %s", method, route, http_version) != 3) {
        printf("Failed to parse request\n");
        return;
    }

    printf("Method: %s, Route: %s, HTTP Version: %s\n", method, route, http_version);

    int pageFound = 0;
    for (int i = 0; i < MAX_PAGES; i++) {
        Endpoint endpoint = endpoints[i];
        
        short length = strlen(route);

        if (matchEndpoint(endpoint.route, route, length) == 0) {
            void* queryParam = endpoint.extractQueryParams(route, length);
            char* response = endpoint.endpointFunc(queryParam);

            if (response) {
                send(client_sock, response, strlen(response), 0);
                free(response);
            } else {
                char* buffer = "[]";
                send(client_sock, buffer, 2, 0);
            }
            
            pageFound = 1;
            break;
        }
    }

    if (!pageFound) {
        const char *response =
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: text/plain\r\n"
            "Connection: close\r\n"
            "\r\n"
            "404 Not Found\n";
        send(client_sock, response, strlen(response), 0);
    }

    close(client_sock);
}

short runServer(Endpoint* endpointsParam) {
    endpoints = endpointsParam;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) {
        perror("Socket creation failed");
        return ERROR;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(PRIVATE_IP_ADDRESS); 
    server_addr.sin_port = htons(PORT);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_sock);
        return ERROR;
    }

    if (listen(server_sock, 3) < 0) {
        perror("Listen failed");
        close(server_sock);
        return ERROR;
    }

    short retry = 0;

    while (1) {
        if (retry > 4) {
            return ERROR;
        }

        client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_sock < 0) {
            retry++;
            perror("Accept failed");
            continue;
        }
        handle_request(client_sock);
    }

    close(server_sock);
    return SUCCESS;
}

