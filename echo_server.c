#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAX_BUFFER 2048

void *handle_client(void *arg) {
    int client_socket = *(int *)arg;
    free(arg);
    char buffer[MAX_BUFFER];
    while (1) {
        ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        buffer[bytes_received] = '\0';
        printf("Received: %s", buffer); 
        send(client_socket, buffer, bytes_received, 0);
    }
    close(client_socket);
    return NULL;
}

int main(int argc, char *argv[]) {
    int port = 0, server_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            port = atoi(argv[++i]);
        }
    }
    if (port <= 0) {
        fprintf(stderr, "Usage: %s -p <port>\n", argv[0]);
        return EXIT_FAILURE;
    }
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_socket, 1);
    printf("Echo server is listening on port %d\n", port);
    while (1) {
        int *client_socket = malloc(sizeof(int));
        if ((*client_socket = accept(server_socket, NULL, NULL)) < 0) {
            perror("Accept failed");
            free(client_socket);
            continue;
        }
        printf("Client connected\n");
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, handle_client, client_socket);
        pthread_detach(thread_id);
    }
    close(server_socket);
    return EXIT_SUCCESS;
}