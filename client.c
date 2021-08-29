#include<sys/socket.h>
#include<stdio.h>
#include<string.h>
#include<netdb.h>
#include<stdlib.h>
#include "../miracl.h"
#include<time.h>

#define BUFFER_SIZE 1024
#define PORT 8090
#define CLIENT_N "client_n.key"
#define CLIENT_S "client_s.key"
#define CLIENT_V "client_v.key"

int sock_desc;

big generateKey();
big generateSprivate(big key);
big generateVpublic(big key, big s);
big readFile(char *fileName);
big bigPow(big a, big b);
big generateX(big r,big n);
big generateY(big r, big s, big e,big n);
int auth(big n,big x, big s, big v, big y, big e,big r);
void writeFile(FILE *fptr, char *buffer,char *fileName);

int main() {
    FILE * keyFile;
    FILE * sFile;
    //init client setup
    char buffer[BUFFER_SIZE];
    int k;
    struct sockaddr_in client;
    memset(&client,0,sizeof(client));
    sock_desc=socket(AF_INET,SOCK_STREAM,0);

    if(sock_desc==-1) {
        printf("Error in socket creation");
        exit(1);
    }

    client.sin_family=AF_INET;
    client.sin_addr.s_addr=INADDR_ANY;
    client.sin_port=PORT;

    k=connect(sock_desc,(struct sockaddr*)&client,sizeof(client));
    if(k==-1) {
        printf("Error in connecting to server\n");
        exit(1);
    }
    //End client set-up

    
    //MIRACL set-up
    miracl *mir = mirsys(1024,10);
    big key,s,v,x,r,n,y,e;
    key = mirvar(0);
    s = mirvar(0);
    v = mirvar(0);
    x = mirvar(0);
    r = mirvar(0);
    n = mirvar(0);
    y = mirvar(0);
    e = mirvar(0);
    printf(" -----Fiat Shameer Zero Ack -----\n");
   
    //Client - Server data exchange
    while (1) {
          printf("...choose here....");
    printf("\n1. Registration \n2. Authentication \n3. Renew \n4. Exit.\n");
        fgets(buffer,BUFFER_SIZE,stdin);
        send(sock_desc,buffer,BUFFER_SIZE,0);
       
        switch (buffer[0]) {
            case '1':
            key = generateKey();
            s = generateSprivate(key);
            v = generateVpublic(key,s);
            otstr(key,buffer);
            send(sock_desc,buffer,BUFFER_SIZE,0);
            printf("Register key %s\n",buffer);
            otstr(s,buffer);
            printf("Private S key %s\n",buffer);
            otstr(v,buffer);
            send(sock_desc,buffer,BUFFER_SIZE,0);
            printf("Public V key %s\n",buffer);
            printf("\n");

            break;
            case '2':
            //TODO: Authentication
            auth(key,x,s,v,y,e,r);
            break;
            case '3':
            //TODO: Update Key
            buffer[0] = '2';
            send(sock_desc,buffer,BUFFER_SIZE,0);
            if(!auth(key,x,s,v,y,e,r)) {
                printf("You are not valid user!\n");
            }
            else
            {
                buffer[0] = '1';
                send(sock_desc,buffer,BUFFER_SIZE,0);
                printf("Update Key\n");
                key = generateKey();
                s = generateSprivate(key);
                v = generateVpublic(key,s);
                otstr(key,buffer);
                send(sock_desc,buffer,BUFFER_SIZE,0);
                printf("Register key %s\n",buffer);
                otstr(s,buffer);
                printf("Private S key %s\n",buffer);
                otstr(v,buffer);
                send(sock_desc,buffer,BUFFER_SIZE,0);
                printf("Public V key %s\n",buffer);
                printf("\n");
                
            }

            break;
            case '4':
            //TODO: End the application
            exit(1);
            break;

            default:printf("choose valid option");
                break;
        }
        //End Client - server data exchange
    }

    exit(0);
    return 0;
}

big generateKey() {
   long seed = (long)time(NULL);
	irand(seed);
    big prime = mirvar(0);
    big generator = mirvar(0);
    bigbits(256,prime);
    bigbits(128,generator);
    nxprime(prime,prime);
    nxprime(generator,generator);
    big key = mirvar(0);
    multiply(prime,generator,key);
    FILE *keyFile;
    char buffer[BUFFER_SIZE];
    otstr(key,buffer);
    writeFile(keyFile,buffer,CLIENT_N);
    return key;
}

big generateSprivate(big key) {
    big s = mirvar(0);
    big t = mirvar(0);
    big z = mirvar(0);
    big o = mirvar(0);
    convert(0,z);
    convert(1,o);
    do{
        bigrand(key,s);
        egcd(s,key,t);
    }while (mr_compare(s,z)==0|| 
    mr_compare(s,o)==0||
    mr_compare(t,o)!=0);
    FILE *sFile;
    char buffer[BUFFER_SIZE];
    otstr(s,buffer);
    writeFile(sFile,buffer,CLIENT_S);
    return s;
}

big generateVpublic(big key, big s) {
    big v = mirvar(0);
    powmod(s,mirvar(2),key,v);
    FILE *sFile;
    char buffer[BUFFER_SIZE];
    otstr(v,buffer);
    writeFile(sFile,buffer,CLIENT_V);
    return v;
}

big generateY(big r, big s, big e,big n) {
    big k = mirvar(0);
    big y = mirvar(0);
    if(mr_compare(e,mirvar(0))==0) {
        k = mirvar(1);
    }else copy(s,k);
    multiply(r,k,k);
    powmod(k,mirvar(1),n,y);
    return y;
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
    big n = mirvar(0);
    cinstr(n,buffer);
    //otnum(n,stdout);
    return n;
}

big generateX(big r,big n) {
    big x = mirvar(0);
    powmod(r,mirvar(2),n,x);
    return x;
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

int auth(big n,big x, big s, big v, big y, big e,big r) {
    char buffer[BUFFER_SIZE];
    long seed = (long)time(NULL);
    for(int i = 0; i<5; i++) {
        printf("Authentication \n");
        irand((seed++)*2);
        n = readFile(CLIENT_N);
        bigrand(n,r);
        x = generateX(r,n);
        otstr(x,buffer);
        send(sock_desc,buffer,BUFFER_SIZE,0);
        printf("Send x %s \n",buffer);
        recv(sock_desc,buffer,BUFFER_SIZE,0);
        printf("Recived c %s \n",buffer);
        cinstr(e,buffer);
        s = readFile(CLIENT_S);
        y = generateY(r,s,e,n);
        otstr(y,buffer);
        send(sock_desc,buffer,BUFFER_SIZE,0);
        printf("Send y %s \n",buffer);
        recv(sock_desc,buffer,BUFFER_SIZE,0);
        printf("Recived %s \n",buffer+2);
        if(buffer[0]=='0') return 0;
        if(i!=4) {
            buffer[0] = '2';
            send(sock_desc,buffer,BUFFER_SIZE,0);
        }

        printf("\n");

    }

    return 1;
}
