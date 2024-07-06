// JGD

#ifndef __SS_H
#define __SS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <dirent.h>

#define MAX_PATH_LEN 32
#define MAX_PATHS 128

#define PATH_MAX 4096

enum FILEORDIR
{
    iFILE,
    iDIR
};

enum req
{
    iREAD,
    iWRITE,
    GET_PERM,
    LS,
    iCREATE,
    iDELETE,
    iCPY,
};

enum CMD
{
    iCRE,
    iDEL,
    iCOPY,
    iWRI,
};

enum response
{
    DONE,
    CONTACT_SERVER
};

typedef struct
{
    char path[1024];
    enum FILEORDIR fileordir;
    char fileordir_name[128];
}file;

typedef struct
{
    enum req req;
    file f;
    file cpy_f;
    char write_text[4096];
}request;

typedef struct
{
    enum CMD cmd;
    file f;
    file cpy_f;
    int cp_port;
    char write_text[4096];
}cmd;

typedef struct{
    char* ss_ip; // IP address of ss
    int port_nm; // Port for NM Connection
    int port_client; // Port for Client Connection
    int port_ss; // Port for other SS to contact
    char* acc_paths[MAX_PATHS]; // Accesable paths by SS
}ss;

typedef struct
{
    file src;
    file dest;
}cpy;

typedef struct
{
    int ack_no;
}ack;

int create_file(file f);
int delete_file(file f);
void send_dir(int socket_id,const char *srcDir, const char *destDir,int fileordir);
void copy_file(cmd c);
void copyFile(int sock,const char *srcPath);
void iread(int client_sockid,file f);
void iwrite(int client_sockid,file f,char* buff);
void ilist (int client_sockid,file f);
void size_and_permissions(int client_sockid,file f);
void cmd_write(file f,char* buff);

typedef struct
{
    char data[1024];
}new;

enum feedback
{
    SUCCESS,
    FAIL,
};

void init_ss();
void* nm_serve(void* nm_socket);
void* serve_client(void *cl_sockid);
void* ss_ss(void *ss_sockid);
void* connect_nm();
void* connect_client();
void* connect_ss();
void* send_nm();
void take_input();

#endif

