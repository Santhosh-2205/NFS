#include "ss.h"

extern char home[4096];
extern char dir[16];
extern char home_dir[4096];
extern char* ss_ip; 
extern int x;
// Creates a file or dir in ss
int create_file(file f) {
    char path[1024];
    snprintf(path, sizeof(path), "%s/%s/%s", home_dir, f.path, f.fileordir_name);
    // printf("path : %s\n", path);

    if (f.fileordir == iDIR) {
        x = mkdir(path, 0777);
        if (x == -1) {
            perror("[-]mkdir error: ");
            return 1;
        }
    }
    else if (f.fileordir == iFILE) {
        int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd == -1) {
            perror("[-]Open error: ");
            return 1;
        }
        close(fd);
    }
    printf("Creating File success!\n");
}
// Deletes a file or dir in ss
int delete_file(file f) {
    char path[1024]; 
    snprintf(path, sizeof(path), "%s/%s/%s", home_dir, f.path, f.fileordir_name);
    // printf("path : %s\n", path);

    if (f.fileordir == 1) {
        char command[256];
        snprintf(command, sizeof(command), "rm -r %s", path);
        int result = system(command);
        if (result==-1) 
            perror("[-]Error in executing commands: ");
    } else {
        if (remove(path) == 0) 
            return 0; // File deleted successfully
        else {
            perror("Error deleting file");
            return 1; // You can use a more specific error code if needed
        }
    }
    printf("Deleting File Success!\n");
}

void copy_file(cmd c)
{
    // printf("port : %d\n",c.cp_port);
    // printf("name : %s\n",c.cpy_f.fileordir_name);

    struct sockaddr_in cp_ss_addr;
    cp_ss_addr.sin_family = AF_INET;
    cp_ss_addr.sin_port = htons(c.cp_port);
    cp_ss_addr.sin_addr.s_addr = inet_addr(ss_ip);

    int tmp_sock = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
    connect(tmp_sock,(struct sockaddr*)&cp_ss_addr,sizeof(cp_ss_addr));

    send(tmp_sock,&c,sizeof(c),0);

    file f;
    while(1)
    {
        recv(tmp_sock,&f,sizeof(f),0);
        // printf("dest path : %s\n",f.path);
        if(strcmp(f.path,"DONE") == 0)
        {
            break;
        }
        char path[1024]; 
        snprintf(path, sizeof(path), "%s/%s", home_dir,f.path);
        // printf("path : %s\n",path);
        if(f.fileordir == 1 && strcmp(f.fileordir_name,".") != 0)
        {
            mkdir(path,0777);
        }
        else if(f.fileordir == 0)
        {
            int file_descriptor = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if(file_descriptor == -1)
            {
                perror("open");
            }
            while(1)
            {
                new s;
                recv(tmp_sock,&s,sizeof(s),0);
                if(strcmp(s.data,"THE END") == 0)
                {
                    break;
                }
                write(file_descriptor,s.data,strlen(s.data));
            }
            close(file_descriptor);   
        }
    }

    close(tmp_sock);
}

void copyFile(int sock,const char *srcPath)
{
    int fd;
    fd = open(srcPath,O_RDONLY);
    new s;
    // char buff[1024];

    int k = lseek(fd, 0, SEEK_END);

    int m,n;
    /*I set the file pointer to the end of the file+1*/
    while (k >= 0)
    { 
        if(k < 1024)
        {
            m = k ;
            k = 0;
        }
        else
        {
            k-= 1024;
            m = 1024;
        }
        lseek(fd, k, SEEK_SET);
        memset(s.data,'\0',sizeof(s.data));
        n = read(fd,s.data, m);
        if(n <= 0)
        {
           break;
        }
        else 
        {
            // printf("buff sending : %s\n",buff);
            if (send(sock, &s, sizeof(s), 0) == -1) {
                perror("Perry the Platypus");
            }
        }
    }
    memset(s.data, 0, sizeof(s.data));
    strcpy(s.data,"THE END");
    if(send(sock, &s, sizeof(s), 0) == -1)
    {
        perror("Perry the Platypus");
    }
    close(fd);
}

void send_dir(int socket_id,const char *srcDir, const char *destDir,int fileordir)
{
    if(fileordir == 0)
    {
        file tmp_f;
        tmp_f.fileordir = 0;
        strcpy(tmp_f.path,destDir);
        send(socket_id,&tmp_f,sizeof(tmp_f),0);

        copyFile(socket_id,srcDir);
        return;
    }
    DIR *dir;
    struct dirent *entry;
    struct stat statBuffer;
    char srcPath[PATH_MAX];
    char destPath[PATH_MAX];

    if((dir = opendir(srcDir)) == NULL) {
        perror("Error opening source directory");
        exit(EXIT_FAILURE);
    }

    file f;
    f.fileordir = 1;
    strcpy(f.path,destDir);
    // printf("dest : %s\n",destDir);
    if(send(socket_id,&f,sizeof(f),0) == -1)
    {
        perror("send");
    }
    while ((entry = readdir(dir)) != NULL) 
    {
        if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) 
        {
            snprintf(srcPath, PATH_MAX, "%s/%s", srcDir, entry->d_name);
            snprintf(destPath, PATH_MAX, "%s/%s", destDir, entry->d_name);
            if (stat(srcPath, &statBuffer) == 0) 
            {
                if (S_ISDIR(statBuffer.st_mode)) {
                    send_dir(socket_id,srcPath,destPath,1);
                } else {
                    file tmp_f;
                    tmp_f.fileordir = 0;
                    strcpy(tmp_f.path,destPath);
                    send(socket_id,&tmp_f,sizeof(tmp_f),0);

                    copyFile(socket_id,srcPath);
                }
            } else {
                perror("Error getting file information");
                exit(EXIT_FAILURE);
            }
        }
    }
    closedir(dir);
}
// reads the file from give path from ss
void iread(int client_sockid,file f)
{
    char path[1024]; // Use this
    snprintf(path, sizeof(path), "%s/%s/%s", home_dir,f.path, f.fileordir_name);
    // printf("path : %s\n",path);

    int fd;
    // char buff[1000000];
    new str;
    fd = open(path, O_RDONLY); /*Here I am opening the file according to their modes and permissions*/
    //send  err_msg -
    // un susces retrun 
    int k = lseek(fd, 0, SEEK_END);

    int m,n;
    /*I set the file pointer to the end of the file+1*/
    while (k >= 0)
    { 
        if(k < 1024)
        {
            m = k ;
            k = 0;
        }
        else
        {
            k-= 1024;
            m = 1024;
        }
        lseek(fd, k, SEEK_SET);
        n = read(fd, str.data, m);
        if(n <= 0)
        {
           break;
        }
        else 
        {
            // printf("%s",buff);
            if (send(client_sockid, str.data, m, 0) == -1) {
                perror("Perry the Platypus");
            }
        }
    }
    memset(str.data, 0, sizeof(str.data));
    strcpy(str.data,"THE END\n");
    // printf("%s\n",buff);
    if(send(client_sockid, str.data, strlen(str.data), 0) == -1)
    {
        perror("Perry the Platypus");
    }

    close(fd);
}
// writes to a file in given path in ss 
void iwrite(int client_sockid,file f,char* buff)
{
    char path[1024]; // Use this
    snprintf(path, sizeof(path), "%s/%s/%s", home_dir,f.path, f.fileordir_name);
    int fd;
    fd = open(path, O_WRONLY); /*Here I am opening the file according to their modes and permissions*/
    // printf("fd is : %d\n",fd);

    // int k = lseek(fd, 0, SEEK_END);
    if(write(fd, buff, strlen(buff))>=0){
        // char buff2[20];
        // strcpy(buff2,"Success");
        // send(client_sockid, buff, sizeof(buff), 0);
    }
    else
    {
        perror("write");
    }
    close(fd);
}

void cmd_write(file f,char* buff)
{
    char path[1024]; // Use this
    snprintf(path, sizeof(path), "%s/%s/%s", home_dir,f.path, f.fileordir_name);

    int fd;
    fd = open(path, O_WRONLY); /*Here I am opening the file according to their modes and permissions*/
    // printf("fd is : %d\n",fd);

    if(write(fd, buff, strlen(buff))>=0){
        // char buff2[20];
        // strcpy(buff2,"Success");
        // send(client_sockid, buff, sizeof(buff), 0);
    }
    else
    {
        perror("write");
    }
    close(fd);
}
// lists the contents of directory in given path.
void ilist (int client_sockid,file f) {
    new msg;
    DIR* dr;
    char path[1024]; // Use this
    struct dirent *en;
    snprintf(path, sizeof(path), "%s/%s/%s", home_dir, f.path,f.fileordir_name);
    dr=opendir(path);
    if (dr) {
        while ((en = readdir(dr)) != NULL) {
            // Get the file's status to check if it's a directory
            struct stat st;
            char full_path[1024];
            sprintf(full_path, "%s/%s", path, en->d_name);
            if (stat(full_path, &st) == 0) {
                if (S_ISDIR(st.st_mode)) {
                    snprintf(msg.data, sizeof(msg.data), "\x1b[34m%s\x1b[0m\n", en->d_name);
                    send(client_sockid, msg.data, strlen(msg.data), 0);
                } else {
                    snprintf(msg.data, sizeof(msg.data), "\x1b[32m%s\x1b[0m\n", en->d_name);
                    send(client_sockid, msg.data, strlen(msg.data), 0);
                }
            } 
        }
        closedir(dr); //close all directory
    }
    memset(msg.data,'\0',sizeof(msg.data));
    strcpy(msg.data,"DONE\n");
    send(client_sockid, msg.data, strlen(msg.data), 0);
}

// gets size and permissions of file or dir in given path
void size_and_permissions(int client_sockid, file f) {
    char path[1024];
    ssize_t bytes;
    snprintf(path, sizeof(path), "%s/%s/%s", home_dir, f.path, f.fileordir_name);
    // send req details to client
    struct stat fileStat;

    if (stat(path, &fileStat) == 0) {
        char message[1024];
        snprintf(message, sizeof(message), "File: %s\nSize: %ld bytes\nPermissions: %o\nLast Access Time: %sLast Modification Time: %sLast Status Change Time: %s",
                 path, fileStat.st_size, fileStat.st_mode & 0777, ctime(&fileStat.st_atime), ctime(&fileStat.st_mtime), ctime(&fileStat.st_ctime));

        // Send the message to the client
        bytes = send(client_sockid, message, strlen(message), 0);
        if (bytes==-1) 
            perror("[-]Send error: ");
    } else 
        perror("stat");
}


