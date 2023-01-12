#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <string.h>
#include <pthread.h>


#define BUFFER 500

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
pthread_t* thread;
pthread_mutex_t lock[3];

void enqueue(char* dir)
{
    struct task* temp = newTask(dir);

    if(q->rear == NULL)
    {
        q->front = q->rear = temp;
        return;
    }
    q->rear->next = temp;
    q->rear = temp;
}

char* dequeue()
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
    free(temp);

    return dir;
}

void *visit(int* arg)
{
    int id = *arg;
    pthread_mutex_lock(&lock[0]);
    char *path = dequeue();
    pthread_mutex_unlock(&lock[0]);

    if(path == NULL)
    {
        return NULL;
    }

    printf("[%d] DIR %s\n", id, path);
    

    DIR *dir;
    struct dirent *dp;
    char *file_name;

    dir = opendir(path);                          // Traversing folders in C recursively: https://iq.opengenus.org/traversing-folders-in-c/

    FILE *f;
    char *command;
    command = (char*)malloc(BUFFER);

    while ((dp = readdir(dir)) != NULL)                 // File
    {
        if (dp->d_type == DT_REG)                       // Checking file types: https://stackoverflow.com/questions/1121383/counting-the-number-of-files-in-a-directory-using-c
        {   
            pthread_mutex_lock(&lock[2]);

            char* filePath;
            filePath = (char*)malloc(BUFFER);
            
            strcpy(filePath, path);                         // Concatenating strings: https://www.educative.io/blog/concatenate-string-c
            strcat(filePath, "/");
            strcat(filePath, dp->d_name);   

            f = fopen(filePath, "r");               // Accessing files: https://stackoverflow.com/questions/16869467/command-line-arguments-reading-a-file
            
            strcpy(command, "grep ");                   // grep 
            strcat(command, search_string);
            strcat(command, " ");
            strcat(command, filePath);
            strcat(command, " >/dev/null");             // How to redirect stdout to /dev/null: https://unix.stackexchange.com/questions/119648/redirecting-to-dev-null
     
            
            int result = system(command);
            pthread_mutex_unlock(&lock[2]);

            if(!result)                        // System function: https://www.tutorialspoint.com/system-function-in-c-cplusplus#:~:text=The%20system()%20function%20is,%3Cstdlib.
                printf("[%d] PRESENT %s\n", id, filePath);
            else
                printf("[%d] ABSENT %s\n", id, filePath);

            fclose(f);

            free(filePath);
            
        }
        else if(strcmp(dp->d_name, "..") != 0 && strcmp(dp->d_name, ".") != 0)  // Check if directory is parent or current: https://stackoverflow.com/questions/50205605/how-to-figure-out-if-the-current-directory-is-the-root-in-c
        {
            pthread_mutex_lock(&lock[1]);

            char *nextPath;
            nextPath = (char*)malloc(BUFFER);
            strcpy(nextPath, path);
            strcat(nextPath, "/");
            strcat(nextPath, dp->d_name);
            
            enqueue(nextPath);
            pthread_mutex_unlock(&lock[1]);

            printf("[%d] ENQUEUE %s\n", id, nextPath);
        }
    }
    closedir(dir);
    free(command);
    free(path);
    visit(&id);

    return 0;
}

int main(int argc, char *argv[])
{
    int workers = atoi(argv[1]);
    search_string = argv[3];
    
    // Initialize Threads
    thread = malloc(sizeof(pthread_t)*workers);

    q = (struct taskQueue*)malloc(sizeof(struct taskQueue));

    char* absolutePath;
    absolutePath = (char*)malloc(BUFFER);
    realpath(argv[2], absolutePath);
    struct task* temp = newTask(absolutePath);

    q->front = q->rear = temp;

    for(int id = 0; id < workers; id++)
    {
        pthread_create(&thread[id], NULL, (void *) visit, &id);
    }

    for(int id = 0; id < workers; id++)
    {
        pthread_join(thread[id], NULL);
    }

    free(q);
    free(thread);

    return 0;
}