#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <dirent.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/wait.h>

// Struct de CLIENTE
typedef struct client_data {
    int client_sockfd;
    char image_name[1024];
    char video_name[1024];
    char text_name[1024];
} client_data;

sem_t mutex;

/* VALIDA SI UN ARCHIVO ES JPG o JPEG*/
bool is_jpg(char *file) {
    int i = 0;
    while (file[i] != '\0') {
        if (file[i] == '.' && file[i + 1] == 'j' && file[i + 2] == 'p') return true;
        i ++;
    }
    return false;
}

/* VALIDA SI UN ARCHIVO ES MP4 */
bool is_mp4(char *file) {
    int i = 0;
    while (file[i] != '\0') {
        if (file[i] == '.' && file[i + 1] == 'm'  && file[i + 2] == 'p'  && file[i + 3] == '4') return true;
        i ++;
    }
    return false;
}

/* VALIDA SI UN ARCHIVO ES TXT */
bool is_txt(char *file) {
    int i = 0;
    while (file[i] != '\0') {
        if (file[i] == '.' && file[i + 1] == 't'  && file[i + 2] == 'x') return true;
        i ++;
    }
    return false;
}

/* HILO QUE ENVÍA LOS DATOS DEL VIDEO AL CLIENTE */
void *send_video(void *args) {
    client_data *client = args;
    printf("Enviando -> %s\n", client->video_name);

    /* Apertura y lectura del archivo solicitado por el cliente */
    char output_stream[1024];
    memset(output_stream, 0, sizeof(output_stream));

    long bytes_read;
    int size_left_over=0;
    int index = 0;
    FILE *mp4_file;
  	if (!(mp4_file = fopen(client->video_name, "rb"))) return 0;
    do {
        bytes_read = fread(output_stream + size_left_over, 1, sizeof(output_stream)-1, mp4_file);
        if (bytes_read < 1) {
            index = 1;
            bytes_read  = 0;
        }
        write(client->client_sockfd, output_stream, sizeof(output_stream)-1);
        memset(output_stream, 0, sizeof(output_stream));
    } while(!index);

    /* Se cierra el archivo y la conexión una vez enviados */
    fclose(mp4_file);
    close(client->client_sockfd);
    exit(0);
}

/* HILO QUE ENVÍA LOS DATOS DE LA IMAGEN AL CLIENTE */
void *send_image(void *args) {
    client_data *client = args;
    printf("Enviando -> %s\n", client->image_name);

    /* Apertura y lectura del archivo solicitado por el cliente */
    char output_stream[1024];
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

void *write_log(void *args){
  sem_wait(&mutex);
  printf("%s\n", "File log writing...");
  FILE *log_file = fopen("./Index/server_log_file.txt", "a+");
  fputs("\nGET ", log_file);
  fputs(args, log_file);
  fclose(log_file);
  sem_post(&mutex);
}


int main(int argc, char *argv[]){
    DIR *this_directory;
    struct sockaddr_in server_addr, client_addr;
    struct dirent *dir_ptr;
    socklen_t sin_len = sizeof(client_addr);
    int fd_server, fd_client;
    int server_len, client_len;
    sem_init(&mutex,0,1);
    /* Storing the contents sent by the browser (a request) */
    char buf[10];
    int fdimg;
    int on = 1;
    int i = 0;

    fd_server = socket(AF_INET, SOCK_STREAM, 0);
    if(fd_server < 0){
        perror("socket");
        exit(1);
    }

    setsockopt(fd_server, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8000);
    server_len = sizeof(server_addr);

    if(bind(fd_server, (struct sockaddr *) &server_addr, server_len) == -1){
        perror("bind");
        close(fd_server);
        exit(1);
    }

    if(listen(fd_server, 10) == -1){
        perror("listen");
        close(fd_server);
        exit(1);
    }

    pthread_t send_video_thread, send_image_thread, write_log_thread;
    printf("%s\n", "El servidor se ha iniciado");

    while(1){
        client_len = sizeof(client_addr);
        fd_client = accept(fd_server, (struct sockaddr *) &client_addr, &client_len);

        if(fd_client < 0){
            exit(1);
        }

        //printf("Got client connection...\n");

        if(!fork()){
            close(fd_server);
            memset(buf, 0, 2048);
            client_data *client = malloc(sizeof *client);
            client->client_sockfd = fd_client;

            read(fd_client, buf, 2047); /* 2047 because of null char? */
            /* Print the request on the console */
            //printf("Esta es la peticion %s\n", buf);
            char *token = strtok(buf, " ");
            token = strtok(NULL, " ");
            pthread_create(&write_log_thread, NULL, write_log, token);
            pthread_join(write_log_thread,0);

            if (is_jpg(token+1)){
              memset(client->image_name, 0, sizeof(client->image_name));
              strcpy(client->image_name, token+1);
              pthread_create(&send_image_thread, NULL, &send_image, client);
              pthread_join(send_image_thread,0);

            } else if (is_mp4(token+1)) {
              memset(client->video_name, 0, sizeof(client->video_name));
              strcpy(client->video_name, token+1);
              pthread_create(&send_video_thread, NULL, &send_video, client);
              pthread_join(send_video_thread,0);

            } else if (strcmp(token+1, "info") == 0){
              printf("Se ha conectado  GET /info\n");
              this_directory = opendir(".");
              if (this_directory) {
                while ((dir_ptr = readdir(this_directory)) != NULL){
                  if (is_txt(dir_ptr->d_name)){
                    fdimg = open(dir_ptr->d_name, O_RDONLY);
                    int sent = sendfile(fd_client, fdimg, NULL, 1000);
                    close(fdimg);
                  }
                } exit(0);
              }
            } else {
              printf("Se ha conectado  GET /\n");
              fdimg = open("./Index/index.txt", O_RDONLY);
              int sent = sendfile(fd_client, fdimg, NULL, 10000);
              close(fdimg);
              printf("%s\n", "Page send");
            }
        }
        else close(fd_client);
        fflush(stdout);
    }
    sem_destroy(&mutex);
    return 0;
}
