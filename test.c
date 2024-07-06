// Client

#include "headers.h"

char home[4096];
char dir[16];
char home_dir[4096];

int id = 0;

int x;
request r;

enum status
{
    SUCESS,
    FAIL
};


struct book_cl
{
    int number_of_visits;
    request r[100];
    enum status s[100];
    int ss_ids[100];
};

struct book_cl cl_book[100];

struct book_ss
{
    int exists;
    int number_of_visits;
    cmd r[100];
    enum status s[100];
}

struct communication
{
    int server_id ; // -1 if server is nm
    int client ; // 0-> ss 1-> cl 
    int id;
    int port;
};

communication cms[10000];

struct book_ss ss_book[100];


add_entry_clbook(int cl_id,request r,int ss_id,enum status s)
{

}

int create_file(file f)
{
    char path[1024]; 
    snprintf(path, sizeof(path), "%s/%s/%s", home_dir,f.path, f.fileordir_name);
    printf("path : %s\n",path);
    
    if(f.fileordir == DIR){
        x = mkdir(path,0777);
        if(x == -1)
        {
            perror("mkdir");
        }
    }
    else if(f.fileordir == iFILE){
        int file_descriptor = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if(file_descriptor == -1)
        {
            perror("open");
        }
        close(file_descriptor);
    }
}

int delete_file(file f)
{
    char path[1024]; // Use this
    snprintf(path, sizeof(path), "%s/%s/%s", home_dir,f.path, f.fileordir_name);
    printf("path : %s\n",path);


    if(f.fileordir == 1)
    {
        char command[256];
        snprintf(command, sizeof(command), "rm -r %s", path);
        int result = system(command);
    }
    else
    {
        if (remove(path) == 0) {
            return 0; // File deleted successfully
        } else {
            perror("Error deleting file");
            return 1; // You can use a more specific error code if needed
        }
    }
}

void iread(int client_sockid,file f)
{
    char path[1024]; // Use this
    snprintf(path, sizeof(path), "%s/%s/%s", home_dir,f.path, f.fileordir_name);
    printf("path : %s\n",path);

    int fd;
    char buff[1000000];
    fd = open(path, O_RDONLY); /*Here I am opening the file according to their modes and permissions*/
    int k = lseek(fd, 0, SEEK_END);

    int m,n;
    /*I set the file pointer to the end of the file+1*/
    while (k >= 0)
    { 
        if(k < 1000000)
        {
            m = k ;
            k = 0;
        }
        else
        {
            k-= 1000000;
            m = 1000000;
        }
        lseek(fd, k, SEEK_SET);
        n = read(fd, buff, m);
        if (n <= 0)
        {
           break;
        }
        else 
        {
            printf("%s",buff);
            // if (send(client_sockid, buff, m, 0) == -1) {
            //     perror("Perry the Platypus");
            // }
        }
    }
    memset(buff, 0, sizeof(buff));
    strcpy(buff,"THE END");
    printf("%s\n",buff);
    // if (send(client_sockid, buff, m, 0) == -1)
    // {
    //     perror("Perry the Platypus");
    // }
    close(fd);
}

void iwrite(int client_sockid,file f)
{
    char path[1024]; // Use this
    snprintf(path, sizeof(path), "%s/%s/%s", home_dir,f.path, f.fileordir_name);
    printf("path : %s\n",path);

    int fd;
    char buff[1000000];
    fd = open(path, O_WRONLY); /*Here I am opening the file according to their modes and permissions*/
    int k = lseek(fd, 0, SEEK_END);
    if(write(fd, buff, sizeof(buff))>=0){
        char buff2[20];
        strcpy(buff2,"Success");
        send(client_sockid, buff, sizeof(buff), 0);
    }
    close(fd);
}

void take_input()
{
    int i;
    printf("Enter req : "); // enum req
    scanf("%d",&i);
    r.req = i;

    printf("Enter path : ");
    scanf("%s",r.f.path);

    printf("Enter file or dir :");
    scanf("%d",&i);
    r.f.fileordir = i;
    if(r.f.fileordir == iFILE)
    {
        printf("Enter name of file : ");
    }
    else
    {
        printf("Enter name of dir :");
    }
    scanf("%s",r.f.fileordir_name);
}


int main()
{
    getcwd(home,sizeof(home));
    snprintf(dir, sizeof(dir), "SS%d", id);
    mkdir(dir,0777);
    snprintf(home_dir,sizeof(home_dir),"%s/%s",home,dir);

    chdir(home_dir);
    printf("exp : %s\n",home_dir);

    char tmp[1024];
    getcwd(tmp,sizeof(tmp));
    printf("org : %s\n",tmp);

    while(1)
    {
        take_input();
        if(r.req == iCREATE)
        {
            create_file(r.f);
        }
        if(r.req == iDELETE)
        {
            delete_file(r.f);
        }
        if(r.req == iREAD)
        {
            iread(0,r.f);
        }
    }
}

void copyDirectory(const char *srcDir, const char *destDir) 
{
    DIR *dir;
    struct dirent *entry;
    struct stat statBuffer;
    char srcPath[PATH_MAX];
    char destPath[PATH_MAX];

    if ((dir = opendir(srcDir)) == NULL) {
        perror("Error opening source directory");
        exit(EXIT_FAILURE);
    }

    if (mkdir(destDir, 0777) == -1) {
        perror("Error creating destination directory");
        exit(EXIT_FAILURE);
    }

    while ((entry = readdir(dir)) != NULL) 
    {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) 
        {
            snprintf(srcPath, PATH_MAX, "%s/%s", srcDir, entry->d_name);
            snprintf(destPath, PATH_MAX, "%s/%s", destDir, entry->d_name);

            if (stat(srcPath, &statBuffer) == 0) 
            {
                if (S_ISDIR(statBuffer.st_mode)) {
                    copyDirectory(srcPath, destPath);
                } else {
                    copyFile(srcPath, destPath);
                }
            } else {
                perror("Error getting file information");
                exit(EXIT_FAILURE);
            }
        }
    }
    closedir(dir);
}
