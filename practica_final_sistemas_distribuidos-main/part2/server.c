#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h> /* For mode constants */
#include <signal.h>
#include <pthread.h>

#include "common.h"
#include "implementation.h"

// Declarar un mutex global
int busy = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_attr_t attr;

ssize_t send_response(int sockfd, int status_code)
{
    int ret = sendMessage(sockfd, (char *)&status_code, sizeof(int));
    if (ret == -1)
    {
        printf("Error sending response\n");
        return -1;
    }

    return 0; // Success
}

ssize_t send_response_publications(int sockfd, Publication* publications, int count)
{
    int buffer_size = MAX_STRING_LENGTH + MAX_STRING_LENGTH + 2; // 2 =  1 SPACE BETWEEN + NULL CHARACTER
    for (int i = 0; i < count; i++)
    {
        char* message = (char*)malloc(buffer_size * sizeof(char));
        if (message == NULL) {
            perror("Error allocating memory for message");
            return -1;
        }

        snprintf(message, buffer_size, "%s %s", publications[i].file_name, publications[i].description);

        int ret = sendMessage(sockfd, message, strlen(message) + 1); // +1 for null terminator
        free(message); // Free dynamically allocated memory for message
        if (ret == -1) {
            printf("Error sending users[%d]\n", i);
            return -1;
        }
    }

    return 0; // Success
}

ssize_t send_response_users(int sockfd, USER_CONNECTED* users, int count) {
    int buffer_size = MAX_STRING_LENGTH + MAX_IP_LENGTH + 8; // 8 = MAX_IP_LENGTH + 2 SPACES BETWEEN + NULL CHARACTER

    for (int i = 0; i < count; i++) {
        char* message = (char*)malloc(buffer_size * sizeof(char));
        if (message == NULL) {
            perror("Error allocating memory for message");
            return -1;
        }

        snprintf(message, buffer_size, "%s %s %d", users[i].user_name, users[i].ip, users[i].port);

        int ret = sendMessage(sockfd, message, strlen(message) + 1); // +1 for null terminator
        free(message); // Free dynamically allocated memory for message
        if (ret == -1) {
            printf("Error sending users[%d]\n", i);
            return -1;
        }
    }

    return 0; // Success
}

int receive_args(int sockfd, char args[MAX_ARGS_LENGTH][MAX_STRING_LENGTH], int args_count, int max_length) {
    int ret;
    for (int i = 0; i < args_count; ++i) {
        ret = readLine(sockfd, args[i], max_length);
        if (ret == -1) {
            printf("Error receiving args[%d]\n", i);
            return -1;
        }
    }
    return 0;
}

ssize_t receive_operation(int sockfd, client_operation *operation)
{
    ssize_t ret;

    ret = readLine(sockfd, operation->key, MAX_STRING_LENGTH * 2);
    if (ret == -1)
    {
        printf("Error receiving operation\n");
        return -1;
    }

    ret = readLine(sockfd, operation->time, MAX_STRING_LENGTH * 2);
    if (ret == -1)
    {
        printf("Error receiving time\n");
        return -1;
    }


    ret = readLine(sockfd, operation->user_name, MAX_STRING_LENGTH * 2);
    if (ret == -1)
    {
        printf("Error receiving user_name\n");
        return -1;
    }

    return 0; // Success
}

void *process_request(void *sc)
{
    int s_local;

    /// avisar copiado argumentos ////
    pthread_mutex_lock(&mutex);
    s_local = (*(int *)sc);
    busy = 0;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
    /////////////////////

    int ret;
    client_operation *request;
    request = (client_operation *)malloc(sizeof(client_operation));
    if (request == NULL)
    {
        perror("Error al asignar memoria para request");
        return NULL;
    }

    //pthread_mutex_lock(&mutex);

    ret = receive_operation(s_local, request);
    if (ret < 0)
    {
        printf("Error\n");
        return NULL;
    }

    printf("s > OPERATION FROM %s, time: %s \n" , request->user_name, request->time);

    int result;

    if (strcmp(request->key, "REGISTER") == 0) {
    result = send_response(s_local, register_user(request->user_name));
    if (result == -1) {
        closeSocket(s_local);
        pthread_exit(NULL);
    }
    }
    else if (strcmp(request->key, "UNREGISTER") == 0) {
        result = send_response(s_local, unregister_user(request->user_name));
        if (result == -1) {
            closeSocket(s_local);
            pthread_exit(NULL);
        }
    }
    else if (strcmp(request->key, "CONNECT") == 0) {
        if (receive_args(s_local, request->args, 2, MAX_STRING_LENGTH * 2) == -1) {
            closeSocket(s_local);
            pthread_exit(NULL);
        }
        result = send_response(s_local, connect_user(request->user_name, request->args[0], atoi(request->args[1])));
        if (result == -1) {
            closeSocket(s_local);
            pthread_exit(NULL);
        }
    }
    else if (strcmp(request->key, "DISCONNECT") == 0) {
        result = send_response(s_local, disconnect_user(request->user_name));
        if (result == -1) {
            closeSocket(s_local);
            pthread_exit(NULL);
        }
    }
    else if (strcmp(request->key, "PUBLISH") == 0) {
        if (receive_args(s_local, request->args, 2, MAX_STRING_LENGTH * 2) == -1) {
            closeSocket(s_local);
            pthread_exit(NULL);
        }
        result = send_response(s_local, publish(request->user_name, request->args[0], request->args[1]));
        if (result == -1) {
            closeSocket(s_local);
            pthread_exit(NULL);
        }
    }
    else if (strcmp(request->key, "DELETE") == 0) {
        if (receive_args(s_local, request->args, 1, MAX_STRING_LENGTH * 2) == -1) {
            closeSocket(s_local);
            pthread_exit(NULL);
        }
        result = send_response(s_local, delete_file(request->user_name, request->args[0]));
        if (result == -1) {
            closeSocket(s_local);
            pthread_exit(NULL);
        }
    }
    else if (strcmp(request->key, "LIST_USERS") == 0) {
        USER_CONNECTED* users = NULL;
        int users_count = 0;
        int res = list_users(request->user_name, &users, &users_count);
        result = send_response(s_local, res);
        if (result == -1) {
            closeSocket(s_local);
            free(users);  // Free allocated memory for users
            pthread_exit(NULL);
        }
        send_response_users(s_local, users, users_count);

        free(users); // Free memory for the array of user structs
    }

    else if (strcmp(request->key, "LIST_CONTENT") == 0) {
        if (receive_args(s_local, request->args, 1, MAX_STRING_LENGTH * 2) == -1) {
            closeSocket(s_local);
            pthread_exit(NULL);
        }
        Publication *publications = NULL;
        int publications_count = 0;
        int res = list_content(request->user_name, request->args[0], &publications, &publications_count);
        result = send_response(s_local, res);
        if (result == -1) {
            closeSocket(s_local);
            pthread_exit(NULL);
        }
        send_response_publications(s_local, publications, publications_count);
    }
    else
    {
        printf("Invalid operation.\n");
    }

    //pthread_mutex_unlock(&mutex);
    free(request);
    closeSocket(s_local);
    pthread_exit(NULL);
    return NULL;
}

int do_exit = 0;

void sigHandler(int signo)
{
    do_exit = 1;
}

int main(int argc, char *argv[])
{

    if (argc != 2)
    {
        printf("Uso: %s <puerto>\n", argv[0]);
        return -1;
    }

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    int sd, sc, port;
    struct sigaction new_action, old_action;

    char *endptr;

    port = strtol(argv[1], &endptr, 10);

    // Check for conversion errors
    if (*endptr != '\0')
    {
        printf("Invalid port number: %s\n", argv[1]);
        return -1;
    }

    // crear socket
    sd = serverSocket(INADDR_ANY, port, SOCK_STREAM);
    if (sd < 0)
    {
        printf("SERVER: Error en serverSocket\n");
        return 0;
    }

    // si se presiona Ctrl-C el bucle termina
    new_action.sa_handler = sigHandler;
    sigemptyset(&new_action.sa_mask);
    new_action.sa_flags = 0;
    sigaction(SIGINT, NULL, &old_action);
    if (old_action.sa_handler != SIG_IGN)
    {
        sigaction(SIGINT, &new_action, NULL);
    }

    printf("s > init server 127.0.0.1:%d\ns >\n", port);

    while (0 == do_exit)
    {
        // aceptar conexión con cliente
        sc = serverAccept(sd);
        if (sc < 0)
        {
            printf("Error en serverAccept\n");
            continue;
        }

        // procesar petición
        pthread_t thread_id;
        pthread_create(&thread_id, &attr, process_request, (void *)&sc);

        /// Esperar copia de argumentos ///
        pthread_mutex_lock(&mutex);
        while (busy == 1)
        {
            pthread_cond_wait(&cond, &mutex);
        }
        busy = 1;
        pthread_mutex_unlock(&mutex);
        /////////////////////
    }

    closeSocket(sd);
    return 0;
}