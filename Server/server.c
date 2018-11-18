#include <dirent.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

// Variables de entorno
#define MAX_CONN 1024
#define SERVER_PORT 8080
#define INPUT_STREAM 1024
#define OUTPUT_STREAM 1024
char response[] = "HTTP/1.1 200 OK\r\n"
"Content-Type: text/html; charset=UTF-8\r\n\r\n"
"<!DOCTYPE html><html><head><title>Bye-bye baby bye-bye</title>"
"<style>body { background-color: #111 }"
"h1 { font-size:4cm; text-align: center; color: black;"
" text-shadow: 0 0 2mm red}</style></head>"
"<body><h1>Goodbye, world!</h1></body></html>\r\n";

// Struct de CLIENTE
typedef struct client_data {
    int client_sockfd;
    char image_name[INPUT_STREAM];
    char song_name[INPUT_STREAM];
} client_data;



/* __________________________________________________________________________ */
/* VALIDA SI UN ARCHIVO ES MP3 */
bool is_mp3(char *file) {
    int i = 0;
    while (file[i] != '\0') {
        if (file[i] == '.' && file[i + 1] == 'm') return true;
        i ++;
    }
    return false;
}

/* VALIDA SI UN ARCHIVO ES JPG */
bool is_jpg(char *file) {
    int i = 0;
    while (file[i] != '\0') {
        if (file[i] == '.' && file[i + 1] == 'j' && file[i + 2] == 'p') return true;
        i ++;
    }
    return false;
}

/* HILO QUE ENVÍA LOS DATOS DE LA IMAGEN AL CLIENTE */
void *send_image(void *args) {
    client_data *client = args;

    /* Apertura y lectura del archivo solicitado por el cliente */
    char output_stream[OUTPUT_STREAM];
    memset(output_stream, 0, sizeof(output_stream));

    long bytes_read;
    int size_left_over=0;
    int index = 0;
    FILE *jpg_file;
  	if (!(jpg_file = fopen(client->image_name, "rb"))) return 0;
    do {
        bytes_read = fread(output_stream + size_left_over, 1, sizeof(output_stream)-1, jpg_file);
        if (bytes_read < 1) {
            index = 1;
            bytes_read  = 0;
        }
        write(client->client_sockfd, output_stream, sizeof(output_stream)-1);
        memset(output_stream, 0, sizeof(output_stream));
    } while(!index);

    /* Se cierra el archivo y la conexión una vez enviados */
    fclose(jpg_file);
    close(client->client_sockfd);
    exit(0);
}

/* HILO QUE ENVÍA LOS DATOS DE LA CANCIÓN AL CLIENTE */
void *send_song(void *args) {
    client_data *client = args;
    printf("Enviando -> %s\n", client->song_name);

    /* Apertura y lectura del archivo solicitado por el cliente */
    char output_stream[OUTPUT_STREAM];
    memset(output_stream, 0, sizeof(output_stream));

    long bytes_read;
    int size_left_over=0;
    int index = 0;
    FILE *mp3_file;
  	if (!(mp3_file = fopen(client->song_name, "rb"))) return 0;
    do {
        bytes_read = fread(output_stream + size_left_over, 1, sizeof(output_stream)-1, mp3_file);
        if (bytes_read < 1) {
            index = 1;
            bytes_read  = 0;
        }
        write(client->client_sockfd, output_stream, sizeof(output_stream)-1);
        memset(output_stream, 0, sizeof(output_stream));
    } while(!index);

    /* Se cierra el archivo y la conexión una vez enviados */
    fclose(mp3_file);
    close(client->client_sockfd);
    exit(0);
}

void open_file(){
  FILE *fp;
    char str[INPUT_STREAM];
    char* filename = "\\galeria\\html.txt";

    fp = fopen(filename, "r");
    if (fp == NULL){
        printf("Could not open file %s",filename);
        return;
    }
    while (fgets(str, INPUT_STREAM, fp) != NULL)
        printf("%s", str);
    fclose(fp);
}

/* MAIN */
int main() {
    DIR *this_directory;

    int server_sockfd, client_sockfd;
    int server_len, client_len;
    int i = 0;

    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    struct dirent *dir_ptr;

    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(SERVER_PORT);
    server_len = sizeof(server_address);

    bind(server_sockfd, (struct sockaddr *)&server_address, server_len);
    listen(server_sockfd, MAX_CONN);
    pthread_t send_song_thread[MAX_CONN];
    pthread_t send_image_thread[MAX_CONN];
    printf("\nEl servidor se ha iniciado...\n\n");

    while(1) {
        char input_stream[INPUT_STREAM];
        char output_stream[OUTPUT_STREAM];
        client_len = sizeof(client_address);
        client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_address, &client_len);

        /* Listener de conexiones del servidor */
        if(fork() == 0) {

            /* Limpia los buffers para evitar que se manden caracteres basura */
            memset(input_stream, 0, sizeof(input_stream));
            memset(output_stream, 0, sizeof(output_stream));

            /* Se inicializan los datos del cliente y se obtiene el input enviado */
            client_data *client = malloc(sizeof *client);
            client->client_sockfd = client_sockfd;

            /* Lee los datos enviados por el cliente y los limpia de caracteres indeseados */
            read(client_sockfd, &input_stream, sizeof(input_stream));
            input_stream[strcspn(input_stream,"\n")] = '\0';
            input_stream[strcspn(input_stream,"\r")] = '\0';
            input_stream[strcspn(input_stream,"|")] = '\0';

            /* Procesamiento de solicitud de reproducción */
            if (is_mp3(input_stream)) {
                printf("Se ha solicitado la canción -> %s\n", input_stream);
                memset(client->song_name, 0, sizeof(client->song_name));
                strcpy(client->song_name, input_stream);
          			pthread_create(&send_song_thread[i++], NULL, &send_song, client);
            }

            /* Procesamiento de solicitud de reproducción */
            else if (is_jpg(input_stream)) {
                memset(client->image_name, 0, sizeof(client->image_name));
                strcpy(client->image_name, input_stream);
          			pthread_create(&send_image_thread[i++], NULL, &send_image, client);
            }

            /* Envío de play_list para primera conexión */
            else {

                /* Bitácora de conexión de usuarios */
                printf("Se ha conectado -> %s\n", input_stream);
                FILE *log_file = fopen("server_log_file.txt", "a+");
                fputs(input_stream, log_file);
                fclose(log_file);
                send(client->client_sockfd, "HTTP/1.1 200 OK\r\n", 17, 0);
                write(client->client_sockfd, response, strlen(response)-1);
                close(client->client_sockfd);
            }
        }

        /* Cierra la conexión */
        else close(client_sockfd);

        /* Para que se muestre el standard output en los threads */
        fflush(stdout);
    }
}
