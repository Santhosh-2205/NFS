// JGD

#include "ss.h"

int server_id;
ss server;

char home[4096];
char dir[16];
char home_dir[4096];
char* ss_ip = "127.0.0.1"; 
int x;

char* nm_ip = "127.0.0.1"; // Naming server IP
int nm_port = 5555; // Naming server port for ss to contact
struct sockaddr_in nm_addr;

int ss_port_nm;       // = 5454; // ss port for ss to contact
struct sockaddr_in ss_addr_nm;
int ss_sockid_nm;

int ss_port_ci ;       //= 4545; // ss port for client interactions
struct sockaddr_in ss_addr_ci;
int ss_sockid_ci;

int ss_port_ss ;       //= 4545; // ss port for ss interactions
struct sockaddr_in ss_addr_ss;
int ss_sockid_ss;
// initialises connections with nm and client and other servers for copy.
void init_ss() {
    server.ss_ip = "127.0.0.1";
    int n;

    server.port_nm = ss_port_nm;
    server.port_client = ss_port_ci;
    server.port_ss = ss_port_ss;

    for(int i = 0 ; i < MAX_PATHS ; i++) {
        server.acc_paths[i] = (char*)malloc(sizeof(char)*MAX_PATH_LEN);
        server.acc_paths[i] = NULL;
    } 

    ss_sockid_nm = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
    ss_addr_nm.sin_family = AF_INET;
    ss_addr_nm.sin_port = htons(ss_port_nm);
    ss_addr_nm.sin_addr.s_addr = inet_addr(server.ss_ip);
    n = bind(ss_sockid_nm,(struct sockaddr*) &ss_addr_nm,sizeof(ss_addr_nm));
    if (n<0) 
        perror("ss_nm bind error: ");
    listen(ss_sockid_nm,10);
    if (n<0) 
        perror("ss_nm listen error: ");

    ss_sockid_ci = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
    ss_addr_ci.sin_family = AF_INET;
    ss_addr_ci.sin_port = htons(ss_port_ci);
    ss_addr_ci.sin_addr.s_addr = inet_addr(server.ss_ip);
    n = bind(ss_sockid_ci,(struct sockaddr*) &ss_addr_ci,sizeof(ss_addr_ci));
    if (n<0) 
        perror("ss_client bind error: ");
    n = listen(ss_sockid_ci,10);
    if (n<0) 
        perror("ss_client listen error: ");

    ss_sockid_ss = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
    ss_addr_ss.sin_family = AF_INET;
    ss_addr_ss.sin_port = htons(ss_port_ss);
    ss_addr_ss.sin_addr.s_addr = inet_addr(server.ss_ip);
    n = bind(ss_sockid_ss,(struct sockaddr*) &ss_addr_ss,sizeof(ss_addr_ss));
    if (n<0) 
        perror("ss_other ss bind error: ");
    listen(ss_sockid_ss,10);
    if (n<0) 
        perror("ss_other ss listen error: ");

    getcwd(home,sizeof(home));
}

void* nm_serve(void* nm_socket) {
    int isocket = *(int*)nm_socket; // Use this for conversation
    free(nm_socket);

    cmd cmd;
    if(recv(isocket,(struct cmd*)&cmd,sizeof(cmd),0) == -1) 
        perror("[-]recv: ");
    printf("Command recieved from NM\n");

    if(cmd.cmd == iCRE)
        create_file(cmd.f);
    else if(cmd.cmd == iDEL)
        delete_file(cmd.f);
    else if(cmd.cmd == iCOPY)
        copy_file(cmd);
    else if(cmd.cmd == iWRI)
        cmd_write(cmd.f,cmd.write_text);
}
// receives the request from client and performs corresponding operation. 
void* serve_client(void *cl_sockid) {
    int socket = *(int*)cl_sockid; 
    free(cl_sockid);

    request r;
    if(recv(socket,&r,sizeof(r),0) == -1) 
        perror("[-]recv error: ");
    printf("recvd from cl: %s\n",r.f.fileordir_name);

    if(r.req == iREAD)
        iread(socket,r.f);
    else if(r.req == iWRITE)
        iwrite(socket,r.f,r.write_text);
    else if(r.req == GET_PERM)
        size_and_permissions(socket,r.f);
    else if(r.req == LS)
        ilist(socket,r.f);

    enum response res;  
    if(send(socket,&res,sizeof(res),0)==-1) 
        perror("[-]Send error: "); 

    close(socket);
}
// connection between ss and other ss to copy files between them. 
void* ss_ss(void *ss_sockid) {
    int socket = *(int*)ss_sockid; 
    free(ss_sockid);

    cmd c;
    if(recv(socket,&c,sizeof(c),0)==-1) 
        perror("[-]Recv error: ");
    printf("Copy Request recieved from SS\n");

    char srcDir[4096];
    char destDir[4096];

    snprintf(destDir,sizeof(destDir),"%s/%s",c.f.path,c.f.fileordir_name);
    snprintf(srcDir, sizeof(srcDir), "%s/%s/%s", home_dir,c.cpy_f.path, c.cpy_f.fileordir_name);
    // printf("srcDir : %s\n",srcDir);

    send_dir(socket,srcDir,destDir,c.f.fileordir);

    file f;
    strcpy(f.path,"DONE");
    if(send(socket,&f,sizeof(f),0)==-1)
        perror("[-]Send error: ");

    close(socket);
}

void* send_nm()
{
    int nm_sockid = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
    nm_addr.sin_family = AF_INET;
    nm_addr.sin_port = htons(nm_port);
    nm_addr.sin_addr.s_addr = inet_addr(nm_ip);

    if (connect(nm_sockid,(struct sockaddr*) &nm_addr, sizeof(nm_addr))) 
        perror("[-]Connect error: ");

    if (send(nm_sockid,&server_id,sizeof(server_id),0)==-1) 
        perror("[-]Send error: ");
    if (server_id == -1) {
        if(recv(nm_sockid,&server_id,sizeof(server_id),0)==-1)
            perror("[-]Recv Error:");
        printf("Served id : %d \n",server_id);
    }
    if (send(nm_sockid,&server,sizeof(server),0)==-1) 
        perror("[-]Send error: ");

    snprintf(dir, sizeof(dir), "SS%d", server_id);
    // Redundancy server
    if(server_id < -1)
    {
        int rid = -server_id;
        rid -= 2;
        memset(dir,'\0',sizeof(dir));
        snprintf(dir, sizeof(dir), "RS%d", rid);
    }
    mkdir(dir,0777);
    snprintf(home_dir,sizeof(home_dir),"%s/%s",home,dir);
    chdir(home_dir);

    if(server_id >= 0)
    {
        new ch;
        while(1)
        {
            recv(nm_sockid,&ch,sizeof(ch),0);
            // printf("%s\n",ch.data);
            ack a;
            a.ack_no = server_id;
            int x = send(nm_sockid,&a,sizeof(a),0);
            // printf("x is %d\n",x);
        }
    }
}
// ss connects to client
void* connect_client()
{
    struct sockaddr_in cl_addr; // Temp var
    int *cl_sockid; // Temp var
    int cl_addr_len = sizeof(cl_addr);
    while(1)
    {
        cl_sockid = (int*)malloc(sizeof(int)); // Freed in thread init_ss.
        *cl_sockid = accept(ss_sockid_ci,(struct sockaddr*)&cl_addr,&cl_addr_len);
        pthread_t accept_client;
        pthread_create(&accept_client,NULL,serve_client,(void*)cl_sockid);
    }
}
// ss connects to another ss
void* connect_ss()
{
    struct sockaddr_in ss_addr; // Temp var
    int *ss_sockid; // Temp var
    int ss_addr_len = sizeof(ss_addr);
    while(1)
    {
        ss_sockid = (int*)malloc(sizeof(int)); // Freed in thread init_ss.
        *ss_sockid = accept(ss_sockid_ss,(struct sockaddr*)&ss_addr,&ss_addr_len);
        pthread_t accept_ss;
        pthread_create(&accept_ss,NULL,ss_ss,(void*)ss_sockid);
    }
}
// ss connects to nm
void* connect_nm()
{
    struct sockaddr_in nm_addr; // Temp var
    int nm_addr_len = sizeof(nm_addr);
    int* nm_socket;
    while(1)
    {
        nm_socket = (int*)malloc(sizeof(int));
        *nm_socket = accept(ss_sockid_nm,(struct sockaddr*)&nm_addr,&nm_addr_len);
        pthread_t nm_cmd;
        pthread_create(&nm_cmd,NULL,nm_serve,(void*)nm_socket);
    }
}
// takes in the input port numbers 
void take_input()
{
    printf("Enter 3 ports : ");
    scanf("%d %d %d",&ss_port_nm,&ss_port_ci,&ss_port_ss);
    printf("Enter ss_id : ");
    scanf("%d",&server_id);
}


int main()
{
    take_input();
    init_ss();
    
    pthread_t alive;
    pthread_create(&alive,NULL,send_nm,NULL);

    pthread_t client_connect;
    pthread_create(&client_connect,NULL,connect_client,NULL);

    pthread_t ss_connect;
    pthread_create(&ss_connect,NULL,connect_ss,NULL);

    pthread_t nm_cmd;
    pthread_create(&nm_cmd,NULL,connect_nm,NULL);

    pthread_join(nm_cmd,NULL);
}
