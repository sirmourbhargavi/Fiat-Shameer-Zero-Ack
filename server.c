#include<sys/socket.h>
#include<stdio.h>
#include<string.h>
#include<netdb.h>
#include<stdlib.h>
#include "../miracl.h"
#include <time.h>

#define BUFFER_SIZE 1024
#define PORT 8090
#define SERVER_N "server_n.key"
#define SERVER_V "server_v.key"

void receive();
void writeFile(FILE *fptr, char *buffer,char *fileName);
big readFile(char *fileName);
int randInRange(int min, int max);
big xveModN(big x, big v, big e, big n);
void auth(big e, big n , char *s , big y,big x , big v , char *buffer , big y_mod_n , big xvemodn);
int temp_sock_desc = 0;

int main() {
    char buf[BUFFER_SIZE];
    int k;
    socklen_t len;
    int sock_desc;
    struct sockaddr_in server,client;

    //Server setup
    memset(&server,0,sizeof(server));
    memset(&client,0,sizeof(client));

    sock_desc=socket(AF_INET,SOCK_STREAM,0);
    if(sock_desc==-1) {
        printf("Error in socket creation");
        exit(1);
    }

    server.sin_family=AF_INET;
    server.sin_addr.s_addr=INADDR_ANY;
    server.sin_port=PORT;

    k=bind(sock_desc,(struct sockaddr*)&server,sizeof(server));
    if(k==-1) {
        printf("Error in binding");
        exit(1);
    }

    k=listen(sock_desc,20);
    if(k==-1){
        printf("Error in listening");
        exit(1);
    }

    len=sizeof(client);//VERY IMPORTANT
    temp_sock_desc=accept(sock_desc,(struct sockaddr*)&client,&len);
    if(temp_sock_desc==-1){
        printf("Error in temporary socket creation");
        exit(1);
    }
    //Server setup end
    
    //MIRACL set-up
    miracl *mir = mirsys(1024,10);

    receive();
    exit(0);
    return 0;
}

void receive() {
    FILE *sPublic;
    int k;
    char buffer[BUFFER_SIZE];
    big x,e,y,y_mod_n,n,v,xvemodn;
    x = mirvar(0);
    e = mirvar(0);
    y = mirvar(0);
    n = mirvar(0);
    v = mirvar(0);
    y_mod_n = mirvar(0);
    xvemodn = mirvar(0);
    char *s = (char *)malloc(sizeof(char)*1024);
    irand(2566);
    while (1) {
        k=recv(temp_sock_desc,buffer,BUFFER_SIZE,0);
        printf("%c \n",buffer[0]);
        int a = 0;

        switch (buffer[0]) {
        case '1':
           recv(temp_sock_desc,buffer,BUFFER_SIZE,0);
           writeFile(sPublic,buffer,SERVER_N);
           printf("Public key n %s\n",buffer);
           recv(temp_sock_desc,buffer,BUFFER_SIZE,0);
           writeFile(sPublic,buffer,SERVER_V);
           printf("Public key v %s\n",buffer);
           printf("\n");
           break;
        case '2':
           //TODO: Authentication 
           printf("Authentication \n");
           recv(temp_sock_desc,buffer,BUFFER_SIZE,0);
           printf("Recived x %s\n",buffer);
           cinstr(x,buffer);
           convert(randInRange(0,1),e);
           otstr(e,buffer);
           printf("Send c %s\n",buffer);
           send(temp_sock_desc,buffer,BUFFER_SIZE,0);
           recv(temp_sock_desc,buffer,BUFFER_SIZE,0);
           printf("Recived y %s\n",buffer);
           cinstr(y,buffer);
           n = readFile(SERVER_N);
           powmod(y,mirvar(2),n,y_mod_n);
           v = readFile(SERVER_V);
           xvemodn = xveModN(x,v,e,n);
           if(mr_compare(y_mod_n,xvemodn)==0) {
                printf("Autorized \n");
                s = "1 Autorized";
                send(temp_sock_desc,s,BUFFER_SIZE,0);
           } else {
               printf("Unauthorized \n");
               s = "0 Unauthorized";
               send(temp_sock_desc,s,BUFFER_SIZE,0);
           }
           printf("\n");
           break;
        case '3':
           //TODO: Update Key
           //Client will check if it is auth or not and generate key;
           break;
        case '4':
           //TODO: End the application
           exit(1);
           break;

        default:
            break;
        }
    }
    
}

void writeFile(FILE *fptr, char *buffer,char *fileName) {
    fptr = fopen(fileName,"w");
    if(fptr==NULL) {
        printf("Unable to write file %s\n",fileName);
        exit(1);
    }
    fputs(buffer,fptr);
    fclose(fptr);
}

big readFile(char *fileName) {
    FILE *fptr;
    fptr = fopen(fileName,"r");
    char buffer[BUFFER_SIZE];
    if(fptr == NULL) {
        printf("ERROR reading file %s\n",fileName);
        exit(1);
    }
    fgets(buffer,BUFFER_SIZE,fptr);
    fclose(fptr);
    //printf("File out %s\n",buffer);
    big n = mirvar(0);
    cinstr(n,buffer);
    otnum(n,stdout);
    return n;
}

int randInRange(int min, int max) {
    double r_max = RAND_MAX;
  return min + (int) (rand() / (double) (r_max + 1) * (max - min + 1));
}

big bigPow(big a, big b) {
    big c = mirvar(0);
    if(mr_compare(b,mirvar(0))==0) return mirvar(1);
    subdiv(b,2,c);
    big k = bigPow(a,c);
    big d = mirvar(1);
    multiply(k,k,d);
    if(!subdivisible(b,2)) {
        multiply(d,a,d);
    }
    return d;
}

big xveModN(big x, big v, big e, big n) {
    big l = bigPow(v,e);
    multiply(l,x,l);
    powmod(l,mirvar(1),n,l);
    return l;
}

void auth(big e, big n , char * s , big y,big x , big v , char *buffer , big y_mod_n , big xvemodn)
{
	convert(randInRange(0,1),e);
           otstr(e,buffer);
           printf("Send e %s\n",buffer);
           send(temp_sock_desc,buffer,BUFFER_SIZE,0);
           recv(temp_sock_desc,buffer,BUFFER_SIZE,0);
           printf("Recived y %s\n",buffer);
           cinstr(y,buffer);
           n = readFile(SERVER_N);
           powmod(y,mirvar(2),n,y_mod_n);
           v = readFile(SERVER_V);
           xvemodn = xveModN(x,v,e,n);
           if(mr_compare(y_mod_n,xvemodn)==0) {
                printf("Autorized \n");
                s = "1 Autorized";
                send(temp_sock_desc,s,BUFFER_SIZE,0);
           } else {
               printf("Unauthorized \n");
               s = "0 Unauthorized";
               send(temp_sock_desc,s,BUFFER_SIZE,0);
           }
           printf("\n");
    }
