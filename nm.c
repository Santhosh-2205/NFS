// JGD
#include "headers.h"

char* nm_ip = "127.0.0.1";

#define no_of_redundant 2

int nm_port_ss = 5555; // Naming server port for ss to contact
struct sockaddr_in nm_addr_ss;
int nm_sockid_ss;

int nm_port_ci = 5566; // Naming server port for client interactions
struct sockaddr_in nm_addr_ci;
int nm_sockid_ci;

int random_ss = 0; // To allocate random ss for creating new dir
struct TrieNode* p;

ss storage_servers[100];
ss redundant_servers[2];

int ss_alive[100];
int red_server[2]; // 0 if not exist 1 if exist

new ch;

int log_message(const char* message) {
    FILE* log_file = fopen("network_manager.log", "a");
    if (log_file != NULL) {
        time_t current_time;
        struct tm* time_info;
        time(&current_time);
        time_info = localtime(&current_time);

        int q=fprintf(log_file, "[%s] %s\n", asctime(time_info), message);
        fclose(log_file);
        if(q>0){return 1;}
    } else {
    }
    return 0;
}

void process_request(const char* client_ip, int client_port, const char* request) {
    char log_messages[256];
    sprintf(log_messages, "Received request from %s:%d: %s", client_ip, client_port, request);
    if(log_message(log_messages)==1){/*printf("Request succesfully stored.\n");*/}
    else {printf("Request failed to store.\n");}
}

void init_nm()
{
    nm_sockid_ss = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
    nm_addr_ss.sin_family = AF_INET;
    nm_addr_ss.sin_port = htons(nm_port_ss);
    nm_addr_ss.sin_addr.s_addr = inet_addr(nm_ip);
    bind(nm_sockid_ss,(struct sockaddr *)&nm_addr_ss,sizeof(nm_addr_ss));
    listen(nm_sockid_ss,10);

    nm_sockid_ci = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
    nm_addr_ci.sin_family = AF_INET;
    nm_addr_ci.sin_port = htons(nm_port_ci);
    nm_addr_ci.sin_addr.s_addr = inet_addr(nm_ip);
    bind(nm_sockid_ci,(struct sockaddr *)&nm_addr_ci,sizeof(nm_addr_ci));
    listen(nm_sockid_ci,10);

    p = getNode();
    initializeCache();

    red_server[0] = 0;
    red_server[1] = 0;

    for(int i = 0 ; i < 100 ; i++)
    {
        ss_alive[i] = 0;
    }
}

int ss_id = 0;        // Unique number to each ss (nm gives to ss)
int client_id = 0; // Unique number to each client (nm gives to ss) // Helpful when doing multiple clients

int get_cp_ss(cmd c)
{
    int cp_id;
    if(strcmp(c.cpy_f.path,".") == 0)
    {
        cp_id = retrieve_ssid(c.cpy_f.fileordir_name);
        if(cp_id == -1) // Cache miss
        {
            cp_id = get_ss(p,c.cpy_f.fileordir_name);
            addEntry(c.cpy_f.fileordir_name,cp_id);
            // printf("Cache miss");
        }
        else
        {
            // printf("Cache hit");
        }
        return cp_id;
    }
    char path[1024];
    strcpy(path,c.cpy_f.path);
    int l = strlen(path);
    int end = 0;
    while(end < l && path[end] != '/') end++;
    end--;
    
    char root_dir[end+2];
    strncpy(root_dir, path, end + 1);
    root_dir[end + 1] = '\0';

    cp_id = retrieve_ssid(root_dir);
    if(cp_id == -1)
    {
        cp_id = get_ss(p,root_dir);
        addEntry(root_dir,cp_id);
        // printf("Cache miss");
    }
    else
    {
        // printf("Cache hit");
    }
    return cp_id;
}

int get_crct_ss(request r)
{
    int cmd_id;
    if(strcmp(r.f.path,".") == 0)
    {
        if(r.req == iCREATE || r.req == iCPY)
        {
            random_ss = (random_ss+1)%ss_id;
            // while ((random_ss+1)%ss_id != 1 /*EXIST*/ ){}
            cmd_id = random_ss;
            insert(p,r.f.fileordir_name,cmd_id);
            addEntry(r.f.fileordir_name,cmd_id);
        }
        else
        {
            cmd_id = retrieve_ssid(r.f.fileordir_name);
            if(cmd_id == -1) // Cache miss
            {
                cmd_id = get_ss(p,r.f.fileordir_name);
                addEntry(r.f.fileordir_name,cmd_id);
                // printf("Cache miss");
            }
            else
            {
                // printf("Cache hit");
            }
        }
        return cmd_id;
    }

    char path[1024];
    strcpy(path,r.f.path);

    int l = strlen(path);
    int end = 0;
    while(end < l && path[end] != '/') end++;
    end --;
    
    char root_dir[end+2];
    strncpy(root_dir, path, end + 1);
    root_dir[end + 1] = '\0';

    // printf("root dir : %s\n",root_dir);

    cmd_id = retrieve_ssid(root_dir);
    if(cmd_id == -1)
    {
        cmd_id = get_ss(p,root_dir);
        addEntry(root_dir,cmd_id);
        // printf("Cache miss");
    }
    else
    {
        // printf("Cache hit");
    }
    return cmd_id;
}

void* nm_ss(void *ss_sockid);
void* look_for_ss()
{
    struct sockaddr_in ss_addr; // Temp var
    int *ss_sockid; // Temp var
    int ss_addr_len = sizeof(ss_addr);
    while(1)
    {
        ss_sockid = (int*)malloc(sizeof(int)); // Freed in thread init_ss.
        *ss_sockid = accept(nm_sockid_ss,(struct sockaddr*)&ss_addr,&ss_addr_len);
        pthread_t accept_ss;
        pthread_create(&accept_ss,NULL,nm_ss,(void*)ss_sockid);
    } 
}

//cline nm , server is ss
void issue_cmd(int ss_id,cmd cmd,int cp_ss_id);
void* nm_ss(void *ss_sockid)
{
    int sockid = *(int*)ss_sockid; // Use this for conversation
    free(ss_sockid);

    int check;
    ssize_t bytes_received = recv(sockid,&check,sizeof(check),0);
    int is_copied = 1;
    
    if(check == -1)
    {
        is_copied = 0;
        ssize_t bytes_sent = send(sockid,&ss_id,sizeof(ss_id),0);
        check = ss_id;
        ss_id++;
    }
    
    ss SS_DETAILS;
    bytes_received = recv(sockid,&SS_DETAILS,sizeof(SS_DETAILS),0);

    printf("SS%d initiated with ports : %d %d %d\n",check,SS_DETAILS.port_nm,SS_DETAILS.port_client,SS_DETAILS.port_ss);
    process_request(nm_ip,nm_port_ss,"Init SS");
    if(check >= 0)
    {
        ss_alive[check] = 1;
        memcpy(&storage_servers[check],&SS_DETAILS,sizeof(ss));
        if(!is_copied) // Copy to rs
        {
            for(int rid = 0 ; rid < 2 ; rid++)
            {
                if(red_server[rid] == 1)
                {
                    cmd c;
                    c.cp_port = storage_servers[check].port_ss;
                    c.cmd = iCOPY;

                    c.f.fileordir = 1;
                    strcpy(c.f.path,".");
                    snprintf(c.f.fileordir_name, sizeof(c.f.fileordir_name), "SS%d", check);
                    
                    // Src
                    c.cpy_f.fileordir = 1;
                    strcpy(c.cpy_f.path,".");
                    strcpy(c.cpy_f.fileordir_name,".");
                    
                    int x = -(rid+2);
                    issue_cmd(x,c,check);
                }
            }
        }
        else // Recovery
        {
            if(red_server[0] == 1) // If red exist
            {
                cmd c;
                c.cp_port = redundant_servers[0].port_ss;
                c.cmd = iCOPY;

                printf("Recovering SS%d...\n",check);

                // Src
                c.cpy_f.fileordir = 1;
                strcpy(c.cpy_f.path,".");
                snprintf(c.cpy_f.fileordir_name, sizeof(c.cpy_f.fileordir_name), "SS%d", check);

                c.f.fileordir = 1;
                strcpy(c.f.path,".");
                strcpy(c.f.fileordir_name,".");

                issue_cmd(check,c,-2);
            }
        }
    }
    else
    {
        int rid = -check;
        rid -= 2;
        memcpy(&redundant_servers[rid],&SS_DETAILS,sizeof(ss));
        red_server[rid] = 1;

        for(int i = 0 ; i < ss_id ; i++)
        {
            cmd c;
            c.cp_port = storage_servers[i].port_ss;
            c.cmd = iCOPY;

            c.f.fileordir = 1;
            strcpy(c.f.path,".");
            snprintf(c.f.fileordir_name, sizeof(c.f.fileordir_name), "SS%d", i);
            
            // Src
            c.cpy_f.fileordir = 1;
            strcpy(c.cpy_f.path,".");
            strcpy(c.cpy_f.fileordir_name,".");

            issue_cmd(check,c,i);
        }
    }
    if(check >= 0)
    {
        ack a;
        a.ack_no = -1;
        // int flags = fcntl(sockid, F_GETFL, 0);
        // fcntl(sockid, F_SETFL, flags | O_NONBLOCK);
        while (1)
        {
            send(sockid,&ch,sizeof(ch),0);
            recv(sockid,&a,sizeof(a),0);
            sleep(5);
            // printf("ack is %d\n",a.ack_no);
            if(a.ack_no == -1)
            {
                printf("SS%d Failed !!\n",check);
                ss_alive[check] = 0;
                break;
            }
            a.ack_no = -1;
        }
    }
    
}

void issue_cmd(int ss_id,cmd cmd,int cp_ss_id)
{
    // printf("ids : %d %d\n",ss_id,cp_ss_id);
    int sockfd = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
    struct sockaddr_in cmd_issue_addr;
    cmd_issue_addr.sin_family = AF_INET;
    if(ss_id >= 0)
    {
        // printf("%d %d\n",ss_id,storage_servers[ss_id].port_nm);
        cmd_issue_addr.sin_port = htons(storage_servers[ss_id].port_nm);
    }
    else
    {
        int rid = -ss_id;
        rid -= 2;
        // printf("%d %d\n",rid,redundant_servers[rid].port_nm);
        cmd_issue_addr.sin_port = htons(redundant_servers[rid].port_nm);
    }
    cmd_issue_addr.sin_addr.s_addr = inet_addr(nm_ip);

    // does ss_id+1 exists 

    int x = connect(sockfd,(struct sockaddr *) &cmd_issue_addr,sizeof(cmd_issue_addr));
    if(x == -1)
    {
        perror("connect");
    }
    if(send(sockfd,&cmd,sizeof(cmd),0) == -1)
    {
        perror("send");
        exit(1);
    }
    process_request(nm_ip,storage_servers[ss_id].port_nm,"Issuing Command");
}

void* nm_cl(void *cl_sockid);
void* look_for_client()
{
    struct sockaddr_in cl_addr; // Temp var
    int *cl_sockid; // Temp var
    int cl_addr_len = sizeof(cl_addr);
    while(1)
    {
        cl_sockid = (int*)malloc(sizeof(int)); // Freed in thread init_ss.
        *cl_sockid = accept(nm_sockid_ci,(struct sockaddr*)&cl_addr,&cl_addr_len);
        pthread_t accept_ss;
        pthread_create(&accept_ss,NULL,nm_cl,(void*)cl_sockid);
    }
}

void* nm_cl(void *cl_sockid)
{
    int sockid = *(int*)cl_sockid; // Use this for conversation
    free(cl_sockid);

    int check;
    recv(sockid,&check,sizeof(check),0); 
    // printf("c cl : %d\n",check);
    if(check == -1)
    {
        send(sockid,&client_id,sizeof(client_id),0);
        check = client_id;
        client_id++;
        printf("Client%d connected\n",client_id);
    }

    request r;
    ssize_t bytes_received = recv(sockid,&r,sizeof(r),0);
    // printf("Data recv : %s\n",r.f.fileordir_name);
    process_request(nm_ip,nm_port_ci,"Request");

    int cmd_id = get_crct_ss(r);
    if(cmd_id == -1)
    {
        enum response res;
        res = PATH_NOT_FOUND;
        send(sockid,&res,sizeof(res),0);
    }
    if(r.req == iCREATE || r.req == iDELETE || r.req == iCPY)
    {
        enum response res;
        if(ss_alive[cmd_id] == 0)
        {
            res = SERVER_DOWN;
            ssize_t bytes_sent = send(sockid,&res,sizeof(res),0);
            return NULL;
        }
        cmd c;
        if(r.req == iCREATE) c.cmd = iCRE;
        else if(r.req == iCPY) c.cmd = iCOPY;
        else c.cmd = iDEL;
        c.f = r.f;
        c.cpy_f = r.cpy_f;

        int cp_id = -1;

        if(r.req == iCPY)
        {
            cp_id = get_cp_ss(c);
            if(cp_id == -1)
            {
                enum response res;
                res = PATH_NOT_FOUND;
                send(sockid,&res,sizeof(res),0);
            }
            c.cp_port = storage_servers[cp_id].port_ss;
            if(r.req == iCPY && ss_alive[cp_id] == 0)
            {
                res = SERVER_DOWN;
                ssize_t bytes_sent = send(sockid,&res,sizeof(res),0);
                return NULL;
            }
        }
        
        issue_cmd(cmd_id,c,cp_id);

        cmd c1; 
        if(r.req == iCREATE) c1.cmd = iCRE;
        else if(r.req == iCPY) c1.cmd = iCOPY;
        else c1.cmd = iDEL;
        c1.f = r.f;
        c1.cpy_f = r.cpy_f;
        for(int rid = 0 ; rid < 2 ; rid++)
        {
            if(red_server[rid] == 1)
            {
                snprintf(c1.f.path,sizeof(c1.f.path),"SS%d/%s",cmd_id,c.f.path);
                if(r.req == iCPY)
                {
                    c1.cp_port = redundant_servers[rid].port_ss;
                    snprintf(c1.cpy_f.path,sizeof(c1.cpy_f.path),"SS%d/%s",cp_id,c.cpy_f.path);
                }
                int x = -(rid+2);
                issue_cmd(x,c1,x);
            }
        }
        res = DONE;
        ssize_t bytes_sent = send(sockid,&res,sizeof(res),0);
    }
    else
    {
        enum response res;
        if(ss_alive[cmd_id] == 0)
        {
            if(r.req == iWRITE)
            {
                res = SERVER_DOWN;
                send(sockid,&res,sizeof(res),0);
                return NULL;
            }
            else
            {
                if(red_server[0] == 0)
                {
                    res = SERVER_DOWN;
                    send(sockid,&res,sizeof(res),0);
                    return NULL;
                }
                res = CONTACT_RS;
                send(sockid,&res,sizeof(res),0);

                struct sockaddr_in ss_addr;
                ss_addr.sin_family = AF_INET;
                ss_addr.sin_port = redundant_servers[0].port_client;
                send(sockid,&ss_addr,sizeof(ss_addr),0);

                send(sockid,&cmd_id,sizeof(cmd_id),0);
                return NULL;
            }
        }

        res = CONTACT_SERVER;
        send(sockid,&res,sizeof(res),0);

        struct sockaddr_in ss_addr;
        ss_addr.sin_family = AF_INET;
        ss_addr.sin_port = storage_servers[cmd_id].port_client;
        send(sockid,&ss_addr,sizeof(ss_addr),0);

        cmd c1;
        c1.cmd = iWRI;
        c1.f = r.f;
        snprintf(c1.f.path,sizeof(c1.f.path),"SS%d/%s",cmd_id,r.f.path);
        for(int rid = 0 ; rid < 2 ; rid++)
        {
            if(red_server[rid] == 1)
            {
                if(r.req == iWRITE)
                {
                    strcpy(c1.write_text,r.write_text);
                    int x = -(rid+2);
                    issue_cmd(x,c1,0);
                }
            }
        } 
    }
    close(sockid);
}

int main()
{
    strcpy(ch.data,"Check");

    init_nm(); 

    pthread_t look_ss;
    pthread_create(&look_ss,NULL,look_for_ss,NULL);    

    pthread_t look_client;
    pthread_create(&look_client,NULL,look_for_client,NULL);

    pthread_join(look_ss,NULL);
    pthread_join(look_client,NULL);
}
