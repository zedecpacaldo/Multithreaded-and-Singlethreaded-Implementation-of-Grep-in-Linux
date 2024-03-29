#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <string.h>


#define BUFFER 260

// Creating a linked list queue: https://stackoverflow.com/questions/36394860/linked-list-of-strings-in-c
struct task
{
    char *dir;
    struct task *next;
};

struct taskQueue 
{
    struct task *front, *rear;
};

struct task* newTask(char *dir)
{
    struct task* temp = (struct task*)malloc(sizeof(struct task));
    temp->dir = dir;
    temp->next = NULL;

    return temp;
}

struct taskQueue* q;
char* search_string;


void enqueue(int id, char* dir)
{
    struct task* temp = newTask(dir);
    printf("[%d] ENQUEUE %s\n", id, dir);

    if(q->rear == NULL)
    {
        q->front = q->rear = temp;
        return;
    }
    q->rear->next = temp;
    q->rear = temp;
}

char* dequeue(int id)
{
    if (q->front == NULL)
    {
        return NULL;
    }

    struct task* temp = q->front;
    char *dir = temp->dir;

    q->front = q->front->next;

    if(q->front == NULL)
    {
        q->rear = NULL;
    }

    printf("[%d] DIR %s\n", id, dir);
    free(temp);
    
    return dir;
}

void *visit(int id)
{
    DIR *dir;
    struct dirent *dp;
    char *file_name;

    char *path = dequeue(id);
    if(path == NULL)
    {
        return NULL;
    }

    // Traversing folders in C recursively: https://iq.opengenus.org/traversing-folders-in-c/
    dir = opendir(path);                          

    FILE *f;
    char *command;
    command = (char*)malloc(BUFFER);

    // File
    while ((dp = readdir(dir)) != NULL)                 
    {
        // Checking file types: https://stackoverflow.com/questions/1121383/counting-the-number-of-files-in-a-directory-using-c
        if (dp->d_type == DT_REG)                       
        {     
            char* filePath;
            filePath = (char*)malloc(BUFFER);
            
            // Concatenating strings: https://www.educative.io/blog/concatenate-string-c
            strcpy(filePath, path);                         
            strcat(filePath, "/");
            strcat(filePath, dp->d_name);             

            // Accessing files: https://stackoverflow.com/questions/16869467/command-line-arguments-reading-a-file
            f = fopen(filePath, "r");               
            
            // grep command
            strcpy(command, "grep ");                   
            strcat(command, search_string);
            strcat(command, " ");
            strcat(command, filePath);

            // How to redirect stdout to /dev/null: https://unix.stackexchange.com/questions/119648/redirecting-to-dev-null
            strcat(command, " >/dev/null");             
     
            // System function: https://www.tutorialspoint.com/system-function-in-c-cplusplus#:~:text=The%20system()%20function%20is,%3Cstdlib.
            if(!system(command))                        
                printf("[%d] PRESENT %s\n", id, filePath);
            else
                printf("[%d] ABSENT %s\n", id, filePath);

            fclose(f);
            
            free(filePath);
        }
        // Check if directory is parent or current: https://stackoverflow.com/questions/50205605/how-to-figure-out-if-the-current-directory-is-the-root-in-c
        else if(strcmp(dp->d_name, "..") != 0 && strcmp(dp->d_name, ".") != 0)  
        {
            char *nextPath;
            nextPath = (char*)malloc(BUFFER);
            strcpy(nextPath, path);
            strcat(nextPath, "/");
            strcat(nextPath, dp->d_name);

            enqueue(id, nextPath);
        }
    }
    closedir(dir);
    free(command);
    free(path);
    visit(id);

    return 0;
}

int main(int argc, char *argv[])
{
    search_string = (char*)malloc(BUFFER);
    strcpy(search_string, "'");
    if(argc > 4)
    {
        strcat(search_string, argv[3]);
        for(int i = 4; i < argc; i++)
        {
            strcat(search_string, " ");
            strcat(search_string, argv[i]);
        }
    }
    else
    {
        strcat(search_string, argv[3]);
    }
    strcat(search_string, "'");

    q = (struct taskQueue*)malloc(sizeof(struct taskQueue));

    char* absolutePath;
    absolutePath = (char*)malloc(BUFFER);

    // Getting absolute path: https://stackoverflow.com/questions/229012/getting-absolute-path-of-a-file
    realpath(argv[2], absolutePath);                           
    struct task* temp = newTask(absolutePath);

    q->front = q->rear = temp;

    visit(0);
    
    free(q);
    free(search_string);
    return 0;
}

