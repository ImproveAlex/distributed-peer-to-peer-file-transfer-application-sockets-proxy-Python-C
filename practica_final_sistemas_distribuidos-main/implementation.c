#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "implementation.h"
#include "common.h"

char **users = NULL;    // Puntero a un array de Usuarios (strings)
int users_register = 0; // Number of de Usuarios registrados

Publication *publications = NULL;
int publish_count = 0;

USER_CONNECTED *users_connected = NULL;
int users_connected_count = 0;

/////////////////////////
// AUX FUNCTIONS
/////////////////////////

int check_string_length(char *value)
{
    // Check if value is within the allowed range
    size_t length = strnlen(value, MAX_STRING_LENGTH + 1);
    if (length == 0 || length > MAX_STRING_LENGTH)
    {
        return -1; // Return -1 if value length is invalid
    }
    return 0; // Return 0 if value length is valid
}

int check_ip_length(char *value)
{
    // Check if value is within the allowed range
    size_t length = strnlen(value, MAX_IP_LENGTH);
    if (length == 0 || length > MAX_IP_LENGTH)
    {
        return -1; // Return -1 if value length is invalid
    }
    return 0; // Return 0 if value length is valid
}

int check_user_register(char *user)
{
    // Recorrer el array de usuarios
    for (int i = 0; i < users_register; i++)
    {
        // Comparar el usuario actual con el usuario dado
        if (strcmp(users[i], user) == 0)
        {
            // Si encontramos el usuario, retornar -1
            return -1;
        }
    }
    // Si no encontramos el usuario, retornar 0
    return 0;
}

int check_port_range(int port)
{
    if (port < 1024 || port > 65535)
    {
        return -1; // Fuera del rango
    }
    else
    {
        return 0; // Dentro del rango
    }
}

int check_user_connected(char *user)
{
    for (int i = 0; i < users_connected_count; i++)
    {
        if (strcmp(users_connected[i].user_name, user) == 0)
        {
            return -1; // Usuario encontrado
        }
    }
    return 0; // Usuario no encontrado
}

int check_file_published(char *file)
{
    for (int i = 0; i < publish_count; i++)
    {
        if (strcmp(publications[i].file_name, file) == 0)
        {
            return -1; // Archivo encontrado
        }
    }
    return 0; // Archivo no encontrado
}

/////////////////////////
// MAIN FUNCTIONS
/////////////////////////

// Function to initialize the service
int init()
{
    if (users != NULL)
    {
        free(users);
        users = NULL;
        users_register = 0;
    }

    if (publications != NULL)
    {
        free(publications);
        publications = NULL;
        publish_count = 0;
    }

    if (users_connected != NULL)
    {
        free(users_connected);
        users_connected = NULL;
        users_connected_count = 0;
    }

    return SUCCESS;
}

enum RC register_user(char *user_name)
{
    // Check if user name is within the allowed range
    if (check_string_length(user_name) == -1)
    {
        return USER_ERROR_2;
    }

    // Check if the user already exists
    if (check_user_register(user_name) == -1)
    {
        return ERROR;
    }

    // Allocate memory for the new user name
    char *new_user = strdup(user_name);
    if (new_user == NULL)
    {
        return USER_ERROR_2; // Error allocating memory
    }

    // Add the new user to the users array
    char **temp = realloc(users, (users_register + 1) * sizeof(char *));
    if (temp == NULL)
    {
        free(new_user);      // Free memory if realloc fails
        return USER_ERROR_2; // Error allocating memory
    }
    users = temp;

    // Allocate memory for the user name in the users array
    users[users_register] = malloc(strlen(new_user) + 1);
    if (users[users_register] == NULL)
    {
        free(new_user);      // Free memory if malloc fails
        return USER_ERROR_2; // Error allocating memory
    }

    // Copy the user name into the users array
    strcpy(users[users_register], new_user);

    // Increment the users_register counter
    users_register++;

    // Free the memory allocated for new_user
    free(new_user);

    return SUCCESS;
}

// Unregister function
enum RC unregister_user(char *user_name)
{

    // Check if user name is within the allowed range
    if (check_string_length(user_name) == -1)
    {
        return USER_ERROR_2;
    }

    // Verificar si el usuario existe y obtener la posicion
    int user_index = -1;
    for (int i = 0; i < users_register; i++)
    {
        if (strcmp(users[i], user_name) == 0)
        {
            user_index = i;
            break;
        }
    }
    if (user_index == -1)
    {
        return ERROR; // El usuario no existe
    }

    // Liberar la memoria del usuario y reorganizar el array
    free(users[user_index]);
    for (int i = user_index; i < users_register - 1; i++)
    {
        users[i] = users[i + 1];
    }
    users = realloc(users, (users_register - 1) * sizeof(char *));
    if (users == NULL && users_register > 1)
    {
        return USER_ERROR_2; // Error al asignar memoria
    }

    users_register--;

    return SUCCESS;
}

enum RC connect_user(char *user_name, char *ip, int port)
{
    // Check if user name is within the allowed range
    if (check_string_length(user_name) == -1)
    {
        return OTHER_ERROR_3;
    }
    // Check if IP is within the allowed range
    if (check_ip_length(ip) == -1)
    {
        return OTHER_ERROR_3;
    }
    // Check if port is within the allowed range
    if (check_port_range(port) == -1)
    {
        return OTHER_ERROR_3;
    }
    // Check if user is registered
    if (check_user_register(user_name) == 0)
    {
        return ERROR;
    }
    // Check if user is already connected
    if (check_user_connected(user_name) == -1)
    {
        return USER_ERROR_2;
    }

    // Asignar memoria para un nuevo usuario conectado
    USER_CONNECTED *new_user = malloc(sizeof(USER_CONNECTED));
    if (new_user == NULL)
    {
        return OTHER_ERROR_3; // Error al asignar memoria
    }

    // Copiar los valores recibidos a la nueva estructura USER_CONNECTED
    strncpy(new_user->user_name, user_name, MAX_STRING_LENGTH);
    strncpy(new_user->ip, ip, MAX_IP_LENGTH);
    new_user->port = port;

    // Reasignar memoria para ampliar el array de usuarios conectados
    USER_CONNECTED *temp = realloc(users_connected, (users_connected_count + 1) * sizeof(USER_CONNECTED));
    if (temp == NULL)
    {
        free(new_user); // Liberar la memoria si falla la reasignación
        return OTHER_ERROR_3;
    }
    users_connected = temp;

    // Agregar el nuevo usuario conectado al array
    users_connected[users_connected_count] = *new_user;
    users_connected_count++;

    free(new_user);

    return SUCCESS;
}

// Disconnect function
enum RC disconnect_user(char *user_name)
{
    // Check if user name is within the allowed range
    if (check_string_length(user_name) == -1)
    {
        return OTHER_ERROR_3;
    }

    if (check_user_register(user_name) == 0)
    {
        return ERROR;
    }

    // Buscar la publicación en el array
    int i;
    for (i = 0; i < users_connected_count; i++)
    {
        if (strcmp(users_connected[i].user_name, user_name) == 0)
        {
            break; // Se encontró el usuario, salir del bucle
        }
    }
    // Si se llegó al final del array sin encontrar la publicación, devolver error
    if (i == users_connected_count)
    {
        return USER_ERROR_2; // Devolver código de error apropiado
    }

    // Eliminar el usuario moviendo las demás hacia atrás
    for (int j = i; j < users_connected_count - 1; j++)
    {
        users_connected[j] = users_connected[j + 1];
    }

    users_connected_count--; // Disminuir el contador de publicaciones

    // Reajustar el tamaño del array de publicaciones
    users_connected = realloc(users_connected, users_connected_count * sizeof(USER_CONNECTED));
    if (users_connected == NULL && users_connected_count > 0)
    {
        // Si falla la realocación pero aún quedan publicaciones, deja el estado del programa intacto
        return OTHER_ERROR_3;
    }

    return SUCCESS; // Devolver éxito si se eliminó el archivo correctamente
}

// Publish function
enum RC publish(char *user_name, char *fileName, char *description)
{

    // Verificar si alguno de los campos no cumple la longitud dentro del rango permitido
    if (check_string_length(user_name) == -1 ||
        check_string_length(fileName) == -1 ||
        check_string_length(description) == -1)
    {
        return OTHER_ERROR_4;
    }

    if (check_user_register(user_name) == 0)
    {
        return ERROR;
    }

    if (check_user_connected(user_name) == 0)
    {
        return USER_ERROR_2;
    }

    if (check_file_published(fileName) == -1)
    {
        return OTHER_ERROR_3;
    }

    // Asignar memoria dinámica para almacenar la publicación
    Publication *new_publication = malloc(sizeof(Publication));
    if (new_publication == NULL)
    {
        return OTHER_ERROR_4; // Retornar error si no se puede asignar memoria
    }

    // Copiar los datos a la nueva publicación
    strncpy(new_publication->user_name, user_name, MAX_STRING_LENGTH);
    strncpy(new_publication->file_name, fileName, MAX_STRING_LENGTH);
    strncpy(new_publication->description, description, MAX_STRING_LENGTH);

    // Reasignar memoria para ampliar el array de publicaciones
    publications = realloc(publications, (publish_count + 1) * sizeof(Publication));
    if (publications == NULL)
    {
        free(new_publication); // Liberar la memoria si falla la reasignación
        return OTHER_ERROR_4;
    }

    // Agregar la nueva publicación al array
    publications[publish_count] = *new_publication;
    publish_count++;

    return SUCCESS;
}

// Delete function
enum RC delete_file(char *user_name, char *fileName)
{
    // Verificar si el nombre de usuario y el nombre de archivo están dentro del rango permitido
    if (check_string_length(user_name) == -1 || check_string_length(fileName) == -1)
    {
        return OTHER_ERROR_4; // Devolver código de error apropiado para longitud de cadena no válida
    }

    // Verificar si el usuario está registrado y conectado
    if (check_user_register(user_name) == 0)
    {
        return ERROR; // Devolver código de error apropiado
    }
    if (check_user_connected(user_name) == 0)
    {
        return USER_ERROR_2; // Devolver código de error apropiado
    }

    // Buscar la publicación en el array
    int i;
    for (i = 0; i < publish_count; i++)
    {
        if (strcmp(publications[i].user_name, user_name) == 0 &&
            strcmp(publications[i].file_name, fileName) == 0)
        {
            break; // Se encontró la publicación, salir del bucle
        }
    }
    // Si se llegó al final del array sin encontrar la publicación, devolver error
    if (i == publish_count)
    {
        return OTHER_ERROR_3; // Devolver código de error apropiado
    }

    // Eliminar la publicación moviendo las demás hacia atrás
    for (int j = i; j < publish_count - 1; j++)
    {
        publications[j] = publications[j + 1];
    }

    publish_count--; // Disminuir el contador de publicaciones

    // Reajustar el tamaño del array de publicaciones
    publications = realloc(publications, publish_count * sizeof(Publication));
    if (publications == NULL && publish_count > 0)
    {
        // Si falla la realocación pero aún quedan publicaciones, deja el estado del programa intacto
        return OTHER_ERROR_4;
    }

    return SUCCESS; // Devolver éxito si se eliminó el archivo correctamente
}

// List users function
enum RC list_users(char *user_name, USER_CONNECTED **users_connected_for_listing, int *users_connected_for_listing_count)
{
    // Verificar si el nombre de usuario esta dentro del rango permitido
    if (check_string_length(user_name) == -1)
    {
        return OTHER_ERROR_3;
    }

    // Verificar si el usuario está registrado y conectado
    if (check_user_register(user_name) == 0)
    {
        return ERROR;
    }
    if (check_user_connected(user_name) == 0)
    {
        return USER_ERROR_2;
    }

    // Asigna memoria dinámica para el array de usuarios conectados
    *users_connected_for_listing = (USER_CONNECTED *)malloc(users_connected_count * sizeof(USER_CONNECTED));
    if (*users_connected_for_listing == NULL)
    {
        return OTHER_ERROR_3; // Error al asignar memoria
    }

    // Inicializa el array dinámico con ceros
    memset(*users_connected_for_listing, 0, users_connected_count * sizeof(USER_CONNECTED));

    // Copia todos los usuarios conectados en el array proporcionado
    for (int i = 0; i < users_connected_count; i++)
    {
        strcpy((*users_connected_for_listing)[i].user_name, users_connected[i].user_name);
        strcpy((*users_connected_for_listing)[i].ip, users_connected[i].ip);
        (*users_connected_for_listing)[i].port = users_connected[i].port;
    }

    // Actualiza el contador de usuarios conectados en el array proporcionado
    *users_connected_for_listing_count = users_connected_count;

    return SUCCESS;
}

enum RC list_content(char *user_name, char *user_for_content_request, Publication **publications_for_listing, int *publications_for_listing_count)
{
    // Verificar si el nombre de usuario está dentro del rango permitido
    if (check_string_length(user_name) == -1 || check_string_length(user_for_content_request) == -1)
    {
        return OTHER_ERROR_4;
    }

    // Verificar si el usuario que solicita está registrado y conectado
    if (check_user_register(user_name) == 0)
    {
        return ERROR; // Usuario solicitante no está registrado
    }
    if (check_user_connected(user_name) == 0)
    {
        return USER_ERROR_2; // Usuario solicitante no está conectado
    }

    int count = 0;

    // Contar la cantidad de publicaciones que coinciden con el usuario solicitado
    for (int i = 0; i < publish_count; i++)
    {
        if (strcmp(publications[i].user_name, user_for_content_request) == 0)
        {
            count++;
        }
    }

    *publications_for_listing_count = count;

    // Asignar memoria dinámica solo si hay publicaciones que coinciden con el usuario solicitado
    if (count > 0)
    {
        *publications_for_listing = (Publication *)malloc(count * sizeof(Publication));
        if (*publications_for_listing == NULL)
        {
            return OTHER_ERROR_4; // Error al asignar memoria
        }

        // Copiar las publicaciones en el array proporcionado
        for (int i = 0, j = 0; i < publish_count; i++)
        {
            if (strcmp(publications[i].user_name, user_for_content_request) == 0)
            {
                strcpy((*publications_for_listing)[j].user_name, publications[i].user_name);
                strcpy((*publications_for_listing)[j].file_name, publications[i].file_name);
                strcpy((*publications_for_listing)[j].description, publications[i].description);
                j++;
            }
        }
    }
    else
    {
        return OTHER_ERROR_3; // No hay publicaciones para el usuario solicitado
    }

    return SUCCESS;
}

/*
int main()
{
    init();
    // Caso 1: Registro exitoso
    enum RC result1 = register_user("user1");
    printf("Caso 1: Registro: Registro exitoso - Resultado esperado 0, resultado obtenido %d\n", result1);

    // Caso 1.1: Registro exitoso
    enum RC result1punto1 = register_user("user2");
    printf("Caso 1.1: Registro: Registro exitoso - Resultado esperado 0, resultado obtenido %d\n", result1punto1);

    // Caso 2: Usuario ya existe
    enum RC result2 = register_user("user1");
    printf("Caso 2: Registro: Usuario ya existe - Resultado esperado 1, resultado obtenido %d\n", result2);

    enum RC result3 = register_user("adxsowrtrezjeronanrglebdibfjlcubpicziqibiekxyqyebadqdmrudxoadyrruuoukaeriqapugokjdtzjmxwlnbkfopumhzgookgqbxytnmaqquppbtofftothkgkrcvruyuwlrzttwjxttoqxzzkopoerbvvslsocxcckwxzsevaefvygurvefesdhhvzwwvnmcprcoocunjggrhcnvibtlndkecejjvhfeyrddadelorysutzwwwtkszkvzkaaktywtewvarazagqxkdaqzyoryxrmjccgpiectjtcoxmdsypfeztmsfzvsgwljihqqodomwgnogxahqmbssakripdlxilfdudojtsasstumrqopvmovinpzielneuybrvrvjedtqyzxurxsprnxepqkeupxlvbhi");
    printf("Caso 3: Registro: Longitud del nombre de usuario superior a 256 - Resultado esperado 2, resultado obtenido %d\n", result3);

    printf("\n\n");
    // Caso 4: Eliminación exitosa del usuario
    enum RC result4 = unregister_user("user1");
    printf("Caso 4: Unregister: Eliminación exitosa del usuario - Resultado esperado 0, resultado obtenido %d\n", result4);

    // Caso 5: Usuario no existe
    enum RC result5 = unregister_user("user4");
    printf("Caso 5: Unregister: Usuario no existe - Resultado esperado 1, resultado obtenido %d\n", result5);

    // Caso 6: Longitud del nombre de usuario superior a 256
    enum RC result6 = unregister_user("adxsowrtrezjeronanrglebdibfjlcubpicziqibiekxyqyebadqdmrudxoadyrruuoukaeriqapugokjdtzjmxwlnbkfopumhzgookgqbxytnmaqquppbtofftothkgkrcvruyuwlrzttwjxttoqxzzkopoerbvvslsocxcckwxzsevaefvygurvefesdhhvzwwvnmcprcoocunjggrhcnvibtlndkecejjvhfeyrddadelorysutzwwwtkszkvzkaaktywtewvarazagqxkdaqzyoryxrmjccgpiectjtcoxmdsypfeztmsfzvsgwljihqqodomwgnogxahqmbssakripdlxilfdudojtsasstumrqopvmovinpzielneuybrvrvjedtqyzxurxsprnxepqkeupxlvbhi");
    printf("Caso 6: Unregister: Longitud del nombre de usuario superior a 256 - Resultado esperado 2, resultado obtenido %d\n", result6);

    printf("\n\n");

    // Caso 7: Caso de éxito
    enum RC result7 = connect_user("user2", "127.0.0.1", 8080);
    printf("Caso 7: Conexion: Caso de éxito - Resultado esperado 0, resultado obtenido %d\n", result7);

    // Caso 8: Longitud del nombre de usuario fuera del rango
    enum RC result8 = connect_user("adxsowrtrezjeronanrglebdibfjlcubpicziqibiekxyqyebadqdmrudxoadyrruuoukaeriqapugokjdtzjmxwlnbkfopumhzgookgqbxytnmaqquppbtofftothkgkrcvruyuwlrzttwjxttoqxzzkopoerbvvslsocxcckwxzsevaefvygurvefesdhhvzwwvnmcprcoocunjggrhcnvibtlndkecejjvhfeyrddadelorysutzwwwtkszkvzkaaktywtewvarazagqxkdaqzyoryxrmjccgpiectjtcoxmdsypfeztmsfzvsgwljihqqodomwgnogxahqmbssakripdlxilfdudojtsasstumrqopvmovinpzielneuybrvrvjedtqyzxurxsprnxepqkeupxlvbhi", "127.0.0.1", 8080);
    printf("Caso 8: Conexion: Longitud del nombre de usuario fuera del rango - Resultado esperado 3, resultado obtenido %d\n", result8);

    // Caso 9: Longitud de la IP fuera del rango
    enum RC result9 = connect_user("usuario", "192.168.1.1lllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllll", 8080);
    printf("Caso 9: Conexion: Longitud de la IP fuera del rango - Resultado esperado 3, resultado obtenido %d\n", result9);

    // Caso 10: Puerto fuera del rango
    enum RC result10 = connect_user("usuario", "127.0.0.1", 6553677);
    printf("Caso 10: Conexion: Puerto fuera del rango - Resultado esperado 3, resultado obtenido %d\n", result10);

    // Caso 11: Usuario no registrado
    enum RC result11 = connect_user("user3", "127.0.0.1", 8080);
    printf("Caso 11: Conexion: Usuario no registrado - Resultado esperado 1, resultado obtenido %d\n", result11);

    // Caso 12: Usuario ya conectado
    enum RC result12 = connect_user("user2", "127.0.0.1", 8080);
    printf("Caso 12: Conexion: Usuario ya conectado - Resultado esperado 2, resultado obtenido %d\n", result12);

    printf("\n\n");

    // Caso 13: Desconexión exitosa
    enum RC result13 = disconnect_user("user2");
    printf("Caso 13: Desconexión: exitosa - Resultado esperado 0, resultado obtenido %d\n", result13);

    // Caso 14: Nombre de usuario fuera del rango
    enum RC result14 = disconnect_user("usuario_demasiado_largo_para_ser_valido_llllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllll");
    printf("Caso 14: Desconexión: Nombre de usuario fuera del rango - Resultado esperado 3, resultado obtenido %d\n", result14);

    // Caso 15: Usuario no registrado
    enum RC result15 = disconnect_user("user3");
    printf("Caso 15: Desconexión: Usuario no registrado - Resultado esperado 1, resultado obtenido %d\n", result15);

    // Caso 17: Desconexión usuario no conectado
    enum RC result17 = disconnect_user("user2");
    printf("Caso 17: Desconexión: usuario no conectado - Resultado esperado 2, resultado obtenido %d\n", result17);

    printf("\n\n");

    enum RC user_for_publish = register_user("user_for_publish");
    enum RC user_for_publish_registerd_not_connected = register_user("user_for_publish_registerd_not_connected");
    enum RC connected_user_for_publish = connect_user("user_for_publish", "127.0.0.3", 8085);

    // Caso 18: Publicación exitosa
    enum RC result18 = publish("user_for_publish", "file1.txt", "Descripción de la publicación 1");
    printf("Caso 18: Publicación: exitosa - Resultado esperado 0, resultado obtenido %d\n", result18);

    // Caso 19: Nombre de usuario demasiado largo
    enum RC result19 = publish("usuario_demasiado_largo_para_ser_validollllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllll", "file2.txt", "Descripción de la publicación 2");
    printf("Caso 19: Publicación: Nombre de usuario demasiado largo - Resultado esperado 4, resultado obtenido %d\n", result19);

    // Caso 20: Nombre de archivo demasiado largo
    enum RC result20 = publish("user2", "adxsowrtrezjeronanrglebdibfjlcubpicziqibiekxyqyebadqdmrudxoadyrruuoukaeriqapugokjdtzjmxwlnbkfopumhzgookgqbxytnmaqquppbtofftothkgkrcvruyuwlrzttwjxttoqxzzkopoerbvvslsocxcckwxzsevaefvygurvefesdhhvzwwvnmcprcoocunjggrhcnvibtlndkecejjvhfeyrddadelorysutzwwwtkszkvzkaaktywtewvarazagqxkdaqzyoryxrmjccgpiectjtcoxmdsypfeztmsfzvsgwljihqqodomwgnogxahqmbssakripdlxilfdudojtsasstumrqopvmovinpzielneuybrvrvjedtqyzxurxsprnxepqkeupxlvbhi", "Descripción de la publicación 3");
    printf("Caso 20: Publicación: Nombre de archivo demasiado largo - Resultado esperado 4, resultado obtenido %d\n", result20);

    // Caso 21: Descripción demasiado larga
    enum RC result21 = publish("user3", "file3.txt", "");
    printf("Caso 21: Publicación: Descripción demasiado larga - Resultado esperado 4, resultado obtenido %d\n", result21);

    // Caso 22: Usuario no registrado
    enum RC result22 = publish("user_no_registrado", "file4.txt", "Descripción de la publicación 4");
    printf("Caso 22: Publicación: Usuario no registrado - Resultado esperado 1, resultado obtenido %d\n", result22);

    // Caso 23: Usuario no conectado
    enum RC result23 = publish("user_for_publish_registerd_not_connected", "file5.txt", "Descripción de la publicación 5");
    printf("Caso 23: Publicación: Usuario no conectado - Resultado esperado 2, resultado obtenido %d\n", result23);

    // Caso 24: Archivo ya publicado
    enum RC result24 = publish("user_for_publish", "file1.txt", "Descripción de la publicación 6dd");
    printf("Caso 24: Publicación: Archivo ya publicado - Resultado esperado 3, resultado obtenido %d\n", result24);

    printf("\n\n");

    // Caso 25: Eliminar archivo exitoso
    enum RC result25 = delete_file("user_for_publish", "file1.txt");
    printf("Caso 25: DeleteFile: Exitoso - Resultado esperado 0, resultado obtenido %d\n", result25);

    // Caso 26: Nombre de usuario demasiado largo
    enum RC result26 = delete_file("adxsowrtrezjeronanrglebdibfjlcubpicziqibiekxyqyebadqdmrudxoadyrruuoukaeriqapugokjdtzjmxwlnbkfopumhzgookgqbxytnmaqquppbtofftothkgkrcvruyuwlrzttwjxttoqxzzkopoerbvvslsocxcckwxzsevaefvygurvefesdhhvzwwvnmcprcoocunjggrhcnvibtlndkecejjvhfeyrddadelorysutzwwwtkszkvzkaaktywtewvarazagqxkdaqzyoryxrmjccgpiectjtcoxmdsypfeztmsfzvsgwljihqqodomwgnogxahqmbssakripdlxilfdudojtsasstumrqopvmovinpzielneuybrvrvjedtqyzxurxsprnxepqkeupxlvbhi", "file1.txt");
    printf("Caso 26: DeleteFile: Nombre de usuario demasiado largo - Resultado esperado 4, resultado obtenido %d\n", result26);

    // Caso 27: Nombre de archivo demasiado largo
    enum RC result27 = delete_file("user1", "adxsowrtrezjeronanrglebdibfjlcubpicziqibiekxyqyebadqdmrudxoadyrruuoukaeriqapugokjdtzjmxwlnbkfopumhzgookgqbxytnmaqquppbtofftothkgkrcvruyuwlrzttwjxttoqxzzkopoerbvvslsocxcckwxzsevaefvygurvefesdhhvzwwvnmcprcoocunjggrhcnvibtlndkecejjvhfeyrddadelorysutzwwwtkszkvzkaaktywtewvarazagqxkdaqzyoryxrmjccgpiectjtcoxmdsypfeztmsfzvsgwljihqqodomwgnogxahqmbssakripdlxilfdudojtsasstumrqopvmovinpzielneuybrvrvjedtqyzxurxsprnxepqkeupxlvbhi.txt");
    printf("Caso 27: DeleteFile: Nombre de archivo demasiado largo - Resultado esperado 4, resultado obtenido %d\n", result27);

    // Caso 28: Usuario no registrado
    enum RC result28 = delete_file("user_no_registrado", "file1.txt");
    printf("Caso 28: DeleteFile: Usuario no registrado - Resultado esperado 1, resultado obtenido %d\n", result28);

    // Caso 29: Usuario no conectado
    enum RC result29 = delete_file("user_for_publish_registerd_not_connected", "file1.txt");
    printf("Caso 29: DeleteFile: Usuario no conectado - Resultado esperado 2, resultado obtenido %d\n", result29);

    // Caso 30: Archivo no encontrado
    enum RC result30 = delete_file("user_for_publish", "file1.txt");
    printf("Caso 30: DeleteFile: Archivo no encontrado - Resultado esperado 3, resultado obtenido %d\n", result30);

    printf("\n\n");

    // Caso 31: Nombre de usuario dentro del rango, usuario registrado y conectado, listado de usuarios correcto
    USER_CONNECTED *users_list31;
    int users_count31;
    register_user("usuario_registrado_list_user");
    register_user("usuario_registrado_list_user_no_conectado");
    connect_user("usuario_registrado_list_user", "127.0.0.1", 8080);
    enum RC result31 = list_users("usuario_registrado_list_user", &users_list31, &users_count31);

    printf("Caso 31: List_User: listado de usuarios correcto - Resultado esperado 0, resultado obtenido %d\n", result31);

    // Verificar si la función se ejecutó correctamente
    if (result31 == SUCCESS)
    {
        // Imprimir la lista de usuarios conectados
        printf("Lista de usuarios conectados:\n");
        for (int i = 0; i < users_count31; i++)
        {
            printf("Usuario: %s, IP: %s, Puerto: %d\n", users_list31[i].user_name, users_list31[i].ip, users_list31[i].port);
        }

        // Liberar la memoria asignada dinámicamente
        free(users_list31);
    }

    // Caso 32: Nombre de usuario fuera del rango
    USER_CONNECTED *users_list32;
    int users_count32;
    enum RC result32 = list_users("adxsowrtrezjeronanrglebdibfjlcubpicziqibiekxyqyebadqdmrudxoadyrruuoukaeriqapugokjdtzjmxwlnbkfopumhzgookgqbxytnmaqquppbtofftothkgkrcvruyuwlrzttwjxttoqxzzkopoerbvvslsocxcckwxzsevaefvygurvefesdhhvzwwvnmcprcoocunjggrhcnvibtlndkecejjvhfeyrddadelorysutzwwwtkszkvzkaaktywtewvarazagqxkdaqzyoryxrmjccgpiectjtcoxmdsypfeztmsfzvsgwljihqqodomwgnogxahqmbssakripdlxilfdudojtsasstumrqopvmovinpzielneuybrvrvjedtqyzxurxsprnxepqkeupxlvbhi", &users_list32, &users_count32);
    printf("Caso 32: List_User: Nombre de usuario fuera del rango - Resultado esperado 3, resultado obtenido %d\n", result32);

    // Caso 33: Usuario no registrado
    USER_CONNECTED *users_list33;
    int users_count33;
    enum RC result33 = list_users("user3", &users_list33, &users_count33);
    printf("Caso 33: List_User: Usuario no registrado - Resultado esperado 1, resultado obtenido %d\n", result33);

    // Caso 34: Usuario no conectado
    USER_CONNECTED *users_list34;
    int users_count34;
    enum RC result34 = list_users("usuario_registrado_list_user_no_conectado", &users_list34, &users_count34);
    printf("Caso 34: Usuario no conectado - Resultado esperado 2, resultado obtenido %d\n", result34);

    printf("\n\n");

    // Caso 35: Nombre de usuario dentro del rango, usuario registrado y conectado, publicaciones encontradas
    register_user("user_for_list_content");
    register_user("user_for_list_content_not_connected");
    connect_user("user_for_list_content", "127.0.0.3", 8085);
    publish("user_for_list_content", "file_list_content.txt", "Descripción de la publicación");
    publish("user_for_list_content", "file_list_content2.txt", "Descripción de la publicación");
    publish("user_for_list_content", "file_list_conten3.txt", "Descripción de la publicación");

    Publication *publications_list35;
    int publications_count35;
    enum RC result35 = list_content("user_for_publish", "user_for_list_content", &publications_list35, &publications_count35);
    printf("Caso 35: ListContent: Nombre de usuario dentro del rango, usuario registrado y conectado, publicaciones encontradas - Resultado esperado 0, resultado obtenido %d\n", result35);
    if (result35 == SUCCESS)
    {
        printf("Publicaciones encontradas:\n");
        for (int i = 0; i < publications_count35; i++)
        {
            printf("Usuario: %s, Archivo: %s, Descripción: %s\n", publications_list35[i].user_name, publications_list35[i].file_name, publications_list35[i].description);
        }
        free(publications_list35);
    }

    // Caso 36: Nombre de usuario fuera del rango
    Publication *publications_list36;
    int publications_count36;
    enum RC result36 = list_content("adxsowrtrezjeronanrglebdibfjlcubpicziqibiekxyqyebadqdmrudxoadyrruuoukaeriqapugokjdtzjmxwlnbkfopumhzgookgqbxytnmaqquppbtofftothkgkrcvruyuwlrzttwjxttoqxzzkopoerbvvslsocxcckwxzsevaefvygurvefesdhhvzwwvnmcprcoocunjggrhcnvibtlndkecejjvhfeyrddadelorysutzwwwtkszkvzkaaktywtewvarazagqxkdaqzyoryxrmjccgpiectjtcoxmdsypfeztmsfzvsgwljihqqodomwgnogxahqmbssakripdlxilfdudojtsasstumrqopvmovinpzielneuybrvrvjedtqyzxurxsprnxepqkeupxlvbhi", "user2", &publications_list36, &publications_count36);
    printf("Caso 36: ListContent: Nombre de usuario fuera del rango - Resultado esperado 4, resultado obtenido %d\n", result36);

    // Caso 37: Usuario no registrado
    Publication *publications_list37;
    int publications_count37;
    enum RC result37 = list_content("user_not_connected", "user_for_list_content", &publications_list37, &publications_count37);
    printf("Caso 37: ListContent: Usuario no registrado - Resultado esperado 1, resultado obtenido %d\n", result37);

    // Caso 38: Usuario no conectado
    Publication *publications_list38;
    int publications_count38;
    enum RC result38 = list_content("user_for_list_content_not_connected", "user_for_list_content", &publications_list38, &publications_count38);
    printf("Caso 38: ListContent: Usuario no conectado - Resultado esperado 2, resultado obtenido %d\n", result38);

    // Caso 40: No hay publicaciones encontradas para el usuario solicitado
    Publication *publications_list40;
    int publications_count40;
    enum RC result40 = list_content("user_for_list_content", "user_no_files", &publications_list40, &publications_count40);
    printf("Caso 40: ListContent: No hay publicaciones encontradas para el usuario solicitado - Resultado esperado 3, resultado obtenido %d\n", result40);

    printf("\n\n");

    return 0;
}
*/