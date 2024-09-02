#include "common.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

int serverSocket(unsigned int addr, int port, int type)
{
    struct sockaddr_in server_addr; // Estructura para almacenar la dirección del servidor
    int sd, ret;                    // Descriptores de socket y variable de retorno

    // Crear socket
    sd = socket(AF_INET, type, 0); // Crear un socket IPv4 con el tipo especificado
    if (sd < 0)
    {                       // Verificar si la creación del socket fue exitosa
        perror("socket: "); // Imprimir un mensaje de error con perror()
        return 0;           // Retornar 0 indicando un error
    }

    // Opción de reusar dirección
    int optval = 1;
    if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
    {
        perror("setsockopt failed");
        return -1;
    }

    // Dirección
    bzero((char *)&server_addr, sizeof(server_addr)); // Inicializar server_addr con ceros
    server_addr.sin_family = AF_INET;                 // Familia de direcciones IPv4
    server_addr.sin_addr.s_addr = INADDR_ANY;         // Dirección IP del servidor (aceptar conexiones en todas las interfaces)
    server_addr.sin_port = htons(port);               // Puerto del servidor en el formato de red

    // Bind
    ret = bind(sd, (const struct sockaddr *)&server_addr, sizeof(server_addr)); // Asociar el socket a la dirección y puerto
    if (ret == -1)
    {                     // Verificar si bind() fue exitoso
        perror("bind: "); // Imprimir un mensaje de error con perror()
        return -1;        // Retornar -1 indicando un error
    }

    // Listen
    ret = listen(sd, SOMAXCONN); // Marcar el socket como pasivo, listo para aceptar conexiones
    if (ret == -1)
    {                       // Verificar si listen() fue exitoso
        perror("listen: "); // Imprimir un mensaje de error con perror()
        return -1;          // Retornar -1 indicando un error
    }

    return sd; // Retornar el descriptor de socket, indicando éxito
}

int serverAccept(int sd)
{

    int sc;
    struct sockaddr_in client_addr;
    socklen_t size;

    size = sizeof(client_addr);
    sc = accept(sd, (struct sockaddr *)&client_addr, (socklen_t *)&size);
    if (sc < 0)
    {
        perror("accept: ");
        return -1;
    }

    return sc;
}

int clientSocket(char *remote, int port)
{
    struct sockaddr_in server_addr; // Estructura para almacenar la dirección del servidor
    struct hostent *hp;             // Puntero a estructura para información del host remoto
    int sd, ret;                    // Descriptores de socket y variable de retorno

    // Crear socket
    sd = socket(AF_INET, SOCK_STREAM, 0); // Crear un socket IPv4 de tipo TCP
    if (sd < 0)
    {                       // Verificar si la creación del socket fue exitosa
        perror("socket: "); // Imprimir un mensaje de error con perror()
        return -1;          // Retornar -1 indicando un error
    }

    // Obtener información del host remoto
    hp = gethostbyname(remote); // Obtener la información del host a partir del nombre remoto
    if (hp == NULL)
    {                                       // Verificar si gethostbyname() fue exitoso
        printf("Error en gethostbyname\n"); // Imprimir un mensaje de error
        return -1;                          // Retornar -1 indicando un error
    }

    // Configurar la dirección del servidor
    bzero((char *)&server_addr, sizeof(server_addr));                  // Inicializar server_addr con ceros
    memcpy(&(server_addr.sin_addr), hp->h_addr_list[0], hp->h_length); // Copiar la dirección del host remoto
    server_addr.sin_family = AF_INET;                                  // Familia de direcciones IPv4
    server_addr.sin_port = htons(port);                                // Puerto del servidor en el formato de red

    // Establecer la conexión
    ret = connect(sd, (struct sockaddr *)&server_addr, sizeof(server_addr)); // Conectar al servidor remoto
    if (ret < 0)
    {                        // Verificar si connect() fue exitoso
        perror("connect: "); // Imprimir un mensaje de error con perror()
        return -1;           // Retornar -1 indicando un error
    }
    return sd; // Retornar el descriptor de socket, indicando éxito
}

int closeSocket(int sd)
{
    int ret;

    ret = close(sd);
    if (ret < 0)
    {
        perror("close: ");
        return -1;
    }

    return ret;
}

double htond(double value)
{
    uint64_t val;
    double result;

    val = *(uint64_t *)&value;
    val = htobe64(val);
    result = *(double *)&val;

    return result;
}

double ntohd(double value)
{
    uint64_t val;
    double result;

    val = *(uint64_t *)&value;
    val = be64toh(val);
    result = *(double *)&val;

    return result;
}

int sendMessage(int socket, char *buffer, int len)
{
    int r;
    int l = len;

    do
    {
        r = write(socket, buffer, l);
        if (r < 0)
        {
            return -1; // error
        }
        l -= r;
        buffer += r;

    } while (l > 0 && r >= 0);

    return 0;
}

int recvMessage(int socket, char *buffer, int len)
{
    int r;
    int l = len;

    do
    {
        r = read(socket, buffer, l);
        if (r < 0)
        {
            return -1; // error
        }
        l -= r;
        buffer += r;

    } while (l > 0 && r >= 0);

    return 0;
}

ssize_t writeLine(int fd, char *buffer)
{
    return sendMessage(fd, buffer, strlen(buffer) + 1);
}

ssize_t readLine(int fd, char *buffer, size_t n)
{
    ssize_t numRead; /* num of bytes fetched by last read() */
    size_t totRead;  /* total bytes read so far */
    char *buf;
    char ch;

    if (n <= 0 || buffer == NULL)
    {
        errno = EINVAL;
        return -1;
    }

    buf = buffer;
    totRead = 0;

    while (1)
    {
        numRead = read(fd, &ch, 1); /* read a byte */

        if (numRead == -1)
        {
            if (errno == EINTR) /* interrupted -> restart read() */
                continue;
            else
                return -1; /* some other error */
        }
        else if (numRead == 0)
        {                     /* EOF */
            if (totRead == 0) /* no byres read; return 0 */
                return 0;
            else
                break;
        }
        else
        { /* numRead must be 1 if we get here*/
            if (ch == '\n')
                break;
            if (ch == '\0')
                break;
            if (totRead < n - 1)
            { /* discard > (n-1) bytes */
                totRead++;
                *buf++ = ch;
            }
        }
    }

    *buf = '\0';
    return totRead;
}