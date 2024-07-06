// Client

#include "headers.h"

char* nm_ip = "127.0.0.1";

int client_id = -1;

request r;

void req_ss(struct sockaddr_in ss_addr)
{
    int client_sockid_tmp = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP); // tmp to contact to ss
    ss_addr.sin_addr.s_addr = inet_addr(nm_ip);
    int port = ss_addr.sin_port;
    ss_addr.sin_port = htons(port);
    int x = connect(client_sockid_tmp,(struct sockaddr*)&ss_addr,sizeof(ss_addr));
    if(x == -1)
    {
        perror("connect");
    }
    send(client_sockid_tmp,&r,sizeof(request),0);
    if(r.req == iREAD)
    {
        // eecv err msg 
        // 0 vaste ledu return
        // char buff[1000000];
        new str;
        memset(str.data, 0, sizeof(str.data));
        while(strcmp(str.data,"THE END") != 0 && strcmp(str.data,"THE END\n") != 0)
        {
            memset(str.data, 0, sizeof(str.data));
            recv(client_sockid_tmp,str.data,sizeof(str.data),0);
            printf("%s",str.data);
        }
        printf("\n");
    }
    if(r.req == iWRITE)
    {
        // char buff[1000000];
        // printf("what to write to file : ");
        // scanf("%s",buff);
        // send(client_sockid_tmp,buff,strlen(buff),0);
    }
    if(r.req == GET_PERM)
    {
        char buff[1000000];
        recv(client_sockid_tmp,buff,sizeof(buff),0);
        printf("%s\n",buff);
    }
    if(r.req == LS)
    {
        new str;
        memset(str.data, 0, sizeof(str.data));
        while(strcmp(str.data,"DONE") != 0 && strcmp(str.data,"DONE\n") != 0)
        {
            memset(str.data, 0, sizeof(str.data));
            recv(client_sockid_tmp,str.data,sizeof(str.data),0);
            printf("%s",str.data);
        }
        printf("\n");
    }
    enum response response;
    recv(client_sockid_tmp,&response,sizeof(enum response),0);
    close(client_sockid_tmp);
}
// Query format for helping the client. 
void total_help() {
    printf("╔════════════════════════════════════════════════════════════════════════════╗\n");
    printf("║  To perform operations successfully, use the following query format:       ║\n");
    printf("╠════════════════════════════════════════════════════════════════════════════╣\n");
    printf("║  - Read a file:              READ <path_to_file>                           ║\n");
    printf("║  - Write to a file:          WRITE <path_to_file>                          ║\n");
    printf("║  - Get file permissions:     GET_PERMISSIONS <path_to_file>                ║\n");
    printf("║  - List directory contents:  LIST <path_to_directory>                      ║\n");
    printf("║  - Create a file or dir:     CREATE <FILE/DIR> <path_to_file>              ║\n");
    printf("║  - Delete a file or dir:     DELETE <FILE/DIR> <path_to_file>              ║\n");
    printf("║  - Copy file or dir:         COPY <FILE/DIR> FROM <path_from> TO <path_to> ║\n");
    printf("╠════════════════════════════════════════════════════════════════════════════╣\n");
    printf("║  - No spaces in paths of files or directories.                             ║\n");
    printf("║  - Make sure to enter correct file types for the operation.                ║\n");
    printf("║  - FILE only for READ, WRITE,    DIR for LS                                ║\n");
    printf("║  - FILE/ DIR for CREATE, DELETE, COPY and GET_PERMISSIONS operations.      ║\n");
    printf("╚════════════════════════════════════════════════════════════════════════════╝\n");
}
// Finds the operation to be performed.
int find_request(char* str) {
    int count=-2;
    if (!strcmp(str,"READ")) {
        r.req = iREAD;
        count=1;
    } else if (!strcmp(str,"WRITE")) {
        r.req = iWRITE;
        count=2;
    } else if (!strcmp(str,"GET_PERM") || !strcmp(str,"GET_PERMISSIONS") || !strcmp(str,"GET_INFO")) {
        r.req = GET_PERM;
        count=3;
    } else if (!strcmp(str,"LIST") || !strcmp(str,"LS")) {
        r.req = LS;
        count=4; 
    } else if (!strcmp(str,"CREATE")) {
        r.req = iCREATE;
        count=5;
    } else if (!strcmp(str,"DELETE")) {
        r.req=iDELETE;
        count=6;
    } else if (!strcmp(str,"COPY")) {
        r.req=iCPY;
        count=7;
    }
    return count;
}

// help function when user does not enter any query.
void help_input1() {
    printf("Empty query is not valid.\n");
    printf("Please refer to the above table for Query Formats.\n");
}
// help function when user enter an invalid query.
void help_input2() {
    printf("The Operation you entered is invalid.\n");
    printf("kindly Re-check the operation name you entered.\n");
    printf("For any other queries regarding Query Format, Please refer to above table.\n");
}
// help function when user enter an invalid file type.
void help_input3() {
    printf("Datatype you entered is incompatible. Please enter correct one: FILE or DIR\n");
    printf("For any other queries regarding Query Format, Please refer to above table.\n");
}
// help function when user enter 
void help_input4() {
   printf("Query you entered is not valid.\n");
   printf("Please refer to the above table for more info.\n");
}

// processes the query entered by client
void input_langg(char* query) {
    char* token = strtok(query," ");
    if (token==NULL) {
        help_input1();
        char QUERY[1024];
        printf("Enter Query: ");
        scanf("\n");
        fgets(QUERY,1024,stdin);
        int len = strlen(QUERY);
        QUERY[len-1] = '\0';
        input_langg(QUERY);
        return;
    }
    int count=-1;
    if (token!=NULL) {
        if (count==-1) {
            count = find_request(token);
            if (count==-2) {
                help_input2();
                char QUERY[1024];
                printf("Enter Query: ");
                scanf("\n");
                fgets(QUERY,1024,stdin);
                int len = strlen(QUERY);
                QUERY[len-1] = '\0';
                input_langg(QUERY);
                return;
            } else {
                token = strtok(NULL," ");
            }
        }
        if (token ==NULL) {
            help_input4();
            char QUERY[1024];
            printf("Enter Query: ");
            scanf("\n");
            fgets(QUERY,1024,stdin);
            int len = strlen(QUERY);
            QUERY[len-1] = '\0';
            input_langg(QUERY);
            return;
        }
        if (count==1 || count==2 || count==3 || count==4) {
            int x=strlen(token);
            char file_name[1024];
            char path_name[1024];
            bzero(file_name,1024);
            bzero(path_name, 1024);
            int last_slash = 0;
            for (int i=0;i<x;i++) {
                if (token[i]=='/') 
                    last_slash = i;
            }
            if (last_slash==0) {
                strcpy(r.f.fileordir_name, token);
                r.f.fileordir = iFILE;
                strcpy(r.f.path,".");
            } else {
                for (int i=0;i<last_slash;i++) 
                    path_name[i] = token[i];
                for (int i=last_slash+1;i<x;i++)
                    file_name[i-last_slash-1] = token[i];
                strcpy(r.f.fileordir_name, file_name);
                strcpy(r.f.path,path_name);
            }
            if(count == 2)
            {
                printf("Enter content to write : ");
                fgets(r.write_text,4096,stdin);
                // printf("%s\n",r.write_text);
            }
        } else if (count==5 || count==6) {
            if (!strcmp(token,"FILE") || !strcmp(token, "File") || !strcmp(token,"copy")) 
                r.f.fileordir = iFILE;
            else if (!strcmp(token,"DIR") || !strcmp(token,"dir") || !strcmp(token,"Dir")) 
                r.f.fileordir = iDIR;
            else {
                help_input3();
                char QUERY[1024];
                printf("Enter Query: ");
                scanf("\n");
                fgets(QUERY,1024,stdin);
                int len = strlen(QUERY);
                QUERY[len-1] = '\0';
                input_langg(QUERY);
                return;
            }
            token = strtok(NULL," ");
            if (token ==NULL) {
                help_input4();
                char QUERY[1024];
                printf("Enter Query: ");
                scanf("\n");
                fgets(QUERY,1024,stdin);
                int len = strlen(QUERY);
                QUERY[len-1] = '\0';
                input_langg(QUERY);
                return;
            }
            int x=strlen(token);
            char file_name[1024];
            char path_name[1024];
            bzero(file_name,1024);
            bzero(path_name, 1024);
            int last_slash = 0;
            for (int i=0;i<x;i++) {
                if (token[i]=='/') 
                    last_slash = i;
            }
            if (last_slash==0) {
                strcpy(r.f.fileordir_name, token);
                strcpy(r.f.path,".");
            } else {
                for (int i=0;i<last_slash;i++) 
                    path_name[i] = token[i];
                for (int i=last_slash+1;i<x;i++)
                    file_name[i-last_slash-1] = token[i];
                strcpy(r.f.fileordir_name, file_name);
                strcpy(r.f.path,path_name);
            }
        } else if (count==7) {
            if (!strcmp(token,"FILE") || !strcmp(token, "File") || !strcmp(token,"copy")) {
                r.f.fileordir = iFILE;
                r.cpy_f.fileordir=iFILE;
            }
            else if (!strcmp(token,"DIR") || !strcmp(token,"dir") || !strcmp(token,"Dir")) {
                r.f.fileordir = iDIR;
                r.cpy_f.fileordir=iDIR;
            }
            else {
                help_input4();
                char QUERY[1024];
                printf("Enter Query: ");
                scanf("\n");
                fgets(QUERY,1024,stdin);
                int len = strlen(QUERY);
                QUERY[len-1] = '\0';
                input_langg(QUERY);
                return;
            }
            token = strtok(NULL, " ");
            if (token ==NULL) {
                help_input4();  
                char QUERY[1024];
                printf("Enter Query: ");
                scanf("\n");
                fgets(QUERY,1024,stdin);
                int len = strlen(QUERY);
                QUERY[len-1] = '\0';
                input_langg(QUERY);
                return;
            }
            int x2 = !strcmp(token,"FROM") || !strcmp(token,"from") || !strcmp(token,"From");
            if (x2==0) {
                help_input4();
                char QUERY[1024];
                printf("Enter Query: ");
                scanf("\n");
                fgets(QUERY,1024,stdin);
                int len = strlen(QUERY);
                QUERY[len-1] = '\0';
                input_langg(QUERY);
                return;
            }
            token = strtok(NULL," ");
            if (token ==NULL) {
                help_input4();  
                char QUERY[1024];
                printf("Enter Query: ");
                scanf("\n");
                fgets(QUERY,1024,stdin);
                int len = strlen(QUERY);
                QUERY[len-1] = '\0';
                input_langg(QUERY);
                return;
            }
            int x=strlen(token);
            char file_name[1024];
            char path_name[1024];
            bzero(file_name,1024);
            bzero(path_name, 1024);
            int last_slash = 0;
            for (int i=0;i<x;i++) {
                if (token[i]=='/') 
                    last_slash = i;
            }
            if (last_slash==0) {
                strcpy(r.cpy_f.fileordir_name, token);
                strcpy(r.cpy_f.path,".");
            } else {
                for (int i=0;i<last_slash;i++) 
                    path_name[i] = token[i];
                for (int i=last_slash+1;i<x;i++)
                    file_name[i-last_slash-1] = token[i];
                strcpy(r.cpy_f.fileordir_name, file_name);
                strcpy(r.cpy_f.path,path_name);
            }
            token = strtok(NULL, " ");
            if (token ==NULL) {
                help_input4();  
                char QUERY[1024];
                printf("Enter Query: ");
                scanf("\n");
                fgets(QUERY,1024,stdin);
                int len = strlen(QUERY);
                QUERY[len-1] = '\0';
                input_langg(QUERY);
                return;
            }
            int y = !strcmp(token,"TO") || !strcmp(token,"To") || !strcmp(token,"to");
            if (y==0) {
                help_input4();
                char QUERY[1024];
                printf("Enter Query: ");
                scanf("\n");
                fgets(QUERY,1024,stdin);
                int len = strlen(QUERY);
                QUERY[len-1] = '\0';
                input_langg(QUERY);
                return;
            }
            token = strtok(NULL," ");
            if (token ==NULL) {
                help_input4();  
                char QUERY[1024];
                printf("Enter Query: ");
                scanf("\n");
                fgets(QUERY,1024,stdin);
                int len = strlen(QUERY);
                QUERY[len-1] = '\0';
                input_langg(QUERY);
                return;
            }
            x=strlen(token);
            char cpy_file_name[1024];
            char cpy_path_name[1024];
            bzero(cpy_file_name,1024);
            bzero(cpy_path_name, 1024);
            last_slash = 0;
            for (int i=0;i<x;i++) {
                if (token[i]=='/') 
                    last_slash = i;
            }
            if (last_slash==0) {
                strcpy(r.f.fileordir_name, token);
                strcpy(r.f.path,".");
            } else {
                for (int i=0;i<last_slash;i++) 
                    path_name[i] = token[i];
                for (int i=last_slash+1;i<x;i++)
                    file_name[i-last_slash-1] = token[i];
                strcpy(r.f.fileordir_name, file_name);
                strcpy(r.f.path,path_name);
            }
        }
    } 
}

int main()
{

    // Connect to NM
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(5566);
    server_addr.sin_addr.s_addr = inet_addr(nm_ip);

    total_help();
    while(1)    
    {
        char query[1024];
        printf("Enter Query: ");
        scanf("\n");
        fgets(query,1024,stdin);
        int len = strlen(query);
        query[len-1] = '\0';
        input_langg(query);

        int client_sockid = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
        connect(client_sockid,(struct sockaddr*)&server_addr,sizeof(server_addr));

        send(client_sockid,&client_id,sizeof(client_id),0);
        // printf("Client id : %d\n",client_id);
        if(client_id == -1)
        {
            recv(client_sockid,&client_id,sizeof(client_id),0);
            printf("client id : %d \n",client_id);
        }

        send(client_sockid,&r,sizeof(request),0);

        enum response response;
        int a = recv(client_sockid,&response,sizeof(enum response),0);

        // printf("res : %d\n",response);

        if(response == CONTACT_SERVER)
        {
            printf("Contacting SS...\n");
            struct sockaddr_in ss_addr;
            a = recv(client_sockid,&ss_addr,sizeof(struct sockaddr_in),0);
            // printf("port : %d\n",ss_addr.sin_port);
            req_ss(ss_addr);
        }
        if(response == CONTACT_RS)
        {
            struct sockaddr_in ss_addr;
            a = recv(client_sockid,&ss_addr,sizeof(struct sockaddr_in),0);
            // printf("port : %d\n",ss_addr.sin_port);

            int fail_id;
            recv(client_sockid,&fail_id,sizeof(fail_id),0);

            char new_path[1024];
            snprintf(new_path,sizeof(new_path),"SS%d/%s",fail_id,r.f.path);
            strcpy(r.f.path,new_path);

            req_ss(ss_addr);
        }
        if(response == SERVER_DOWN)
        {
            printf("Server Down !!\n");
        }
        if(response == PATH_NOT_FOUND)
        {
            printf("Path Not Found !!\n");
        }
    }
} 
