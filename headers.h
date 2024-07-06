// JGD

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
    CONTACT_SERVER,
    CONTACT_RS,
    SERVER_DOWN,
    PATH_NOT_FOUND,
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


struct TrieNode *getNode(void);
void insert(struct TrieNode *root, const char *key,int ss_id);
int get_ss(struct TrieNode *root, const char *key);
int delete_node(struct TrieNode *root, const char *key);


typedef struct {
    char path[100];
    int ss_id;
} cache;

typedef struct
{
    char data[1024];
}new;

void initializeCache() ;
void addEntry(const char *path, int ss_id);
int retrieve_ssid(const char *path);

enum feedback
{
    SUCCESS,
    FAIL,
};
