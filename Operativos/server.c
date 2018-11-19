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

char webpage[] =
"HTTP/1.1 200 Ok\r\n"
"Content-Type: text/html;\r\n\r\n"
"<!DOCTYPE html>\r\n"
"<html>"
  "<head>"
    "<title>Galeria de fotos</title>"
    "<style>"
    "body{background-color: #dedede;font-family: 'calibri', Garamond, 'Comic Sans';}"
    ".button {background-color: white;color: black;border: 2px solid #4CAF50;font-size: 22x;}"
    ".gallery{display: flex;  width: 900px;  margin: auto;  justify-content: space-between;flex-wrap: wrap;}"
    "figure{width: 200px;margin: 8px 0;border: 1px solid #777;padding: 8px;box-sizing: border-box;background-color: #fff;}"
    "figure img{width: 100%;}"
    "figure figcaption{text-align: center;padding: 8px 4px;}"
    "</style>"
  "</head>"
  "<body>"
  "<h1 align='center'> Galeria de videos</h1>"
    "<div class='gallery'>"
      "<figure>"
        "<img src='dog1.jpeg'>"
        "<figcaption>Video de..</figcaption>"
        "<input  class='button' type='button' value='Ver video'>"
      "</figure>"
      "<video> <source src='oveja.mp4' type='video/mp4'></video> "
      "<figure>"
        "<img src='dog2.jpeg'>"
        "<figcaption>Video de..</figcaption>"
        "<input  class='button' type='button' value='Ver video'>"
      "</figure>"
      "<figure>"
        "<img src='dog3.jpeg'>"
        "<figcaption>Video de..</figcaption>"
        "<input  class='button' type='button' value='Ver video'>"
      "</figure>"
      "<figure>"
        "<img src='dog4.jpeg'>"
        "<figcaption>Video de..</figcaption>"
        "<input  class='button' type='button' value='Ver video'>"
      "</figure>"
      "<figure>"
        "<img src='dog5.jpeg'>"
        "<figcaption>Video de..</figcaption>"
        "<input  class='button' type='button' value='Ver video'>"
      "</figure>"
      "<figure>"
        "<img src='dog6.jpeg'>"
        "<figcaption>Video de..</figcaption>"
        "<input  class='button' type='button' value='Ver video'>"
      "</figure>"
    "</div>"
  "</body>"
"</html>";


int main(int argc, char *argv[]){
    struct sockaddr_in server_addr, client_addr;
    socklen_t sin_len = sizeof(client_addr);
    int fd_server, fd_client;
    /* Storing the contents sent by the browser (a request) */
    char buf[10];
    int fdimg;
    int on = 1;

    fd_server = socket(AF_INET, SOCK_STREAM, 0);
    if(fd_server < 0){
        perror("socket");
        exit(1);
    }

    setsockopt(fd_server, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8080);

    if(bind(fd_server, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1){
        perror("bind");
        close(fd_server);
        exit(1);
    }

    if(listen(fd_server, 10) == -1){
        perror("listen");
        close(fd_server);
        exit(1);
    }

    while(1){
        fd_client = accept(fd_server, (struct sockaddr *) &client_addr, &sin_len);

        if(fd_client == -1){
            perror("Connection failed...\n");
            continue;
        }

        printf("Got client connection...\n");

        if(!fork()){

            /* Child process */

            /* Close this as the client no longer needs it */
            close(fd_server);
            memset(buf, 0, 2048);
            read(fd_client, buf, 2047); /* 2047 because of null char? */

            /* Print the request on the console */
            printf("%s\n", buf);
           // Carga las imagenes
            if(!strncmp(buf, "GET /dog1.jpeg", 13)){
                printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
                fdimg = open("dog1.jpeg", O_RDONLY);
                int sent = sendfile(fd_client, fdimg, NULL, 10000);
                printf("sent: %d", sent);
                close(fdimg);
            }
            if(!strncmp(buf, "GET /dog2.jpeg", 13)){
                printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
                fdimg = open("dog2.jpeg", O_RDONLY);
                int sent = sendfile(fd_client, fdimg, NULL, 10000);
                printf("sent: %d", sent);
                close(fdimg);
            }
            if(!strncmp(buf, "GET /dog3.jpeg", 13)){
                printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
                fdimg = open("dog3.jpeg", O_RDONLY);
                int sent = sendfile(fd_client, fdimg, NULL, 10000);
                printf("sent: %d", sent);
                close(fdimg);
            }
            if(!strncmp(buf, "GET /dog4.jpeg", 13)){
                printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
                fdimg = open("dog4.jpeg", O_RDONLY);
                int sent = sendfile(fd_client, fdimg, NULL, 10000);
                printf("sent: %d", sent);
                close(fdimg);
            }
            if(!strncmp(buf, "GET /dog5.jpeg", 13)){
                printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
                fdimg = open("dog5.jpeg", O_RDONLY);
                int sent = sendfile(fd_client, fdimg, NULL, 10000);
                printf("sent: %d", sent);
                close(fdimg);
            }
            if(!strncmp(buf, "GET /dog6.jpeg", 13)){
                printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
                fdimg = open("dog6.jpeg", O_RDONLY);
                int sent = sendfile(fd_client, fdimg, NULL, 10000);
                printf("sent: %d", sent);
                close(fdimg);
            }
            if(!strncmp(buf, "GET /oveja.mp4", 13)){
                printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
                fdimg = open("oveja.mp4", O_RDONLY);
                int sent = sendfile(fd_client, fdimg, NULL, 100000);
                printf("sent: %d", sent);
                close(fdimg);
            }
            else{
                write(fd_client, webpage, sizeof(webpage) - 1);
            }

            close(fd_client);
            printf("closing connection...\n");
            exit(0);
        }

        /* Parent process */
        close(fd_client);


    }


    return 0;

}
