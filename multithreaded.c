#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <string.h>
#include <pthread.h>


#define BUFFER 260

// Global Variables
struct taskQueue* q;
char* search_string;
pthread_t* thread;
// 1 lock for enqueue/grep if folder/file, 1 for dequeue
pthread_mutex_t lock[2];
// For the auxiliary function to count expected directories
int folderCount = 0;
//  Increment this every dequeue
int visitedFolders = 0;

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

// Traverses directory and increments folderCount. This 
// will be used to check when to end the infinite loop
void traverse(char *path)
{
    char nextPath[BUFFER];
    struct dirent *dp;
    DIR *dir = opendir(path);

   
    if (!dir)
        return;

    while ((dp = readdir(dir)) != NULL)
    {
        if ((strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0) && (dp->d_type != DT_REG))
        {
            // increment folderCount
            folderCount++;
            strcpy(nextPath, path);
            strcat(nextPath, "/");
            strcat(nextPath, dp->d_name);

            traverse(nextPath);
        }
    }

    closedir(dir);
}

void* fn(void* arg)
{
    int id = *((int*)arg);

    // End infinite loop when we have already
    // visited all the folders
    while(visitedFolders < folderCount)
    {
        while(q->front != NULL)
        {
            // Lock during dequeue so only 1 thread per directory.
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

            // Traversing folders in C recursively: https://iq.opengenus.org/traversing-folders-in-c/
            dir = opendir(path);

            FILE *f;
            char *command;
            command = (char*)malloc(BUFFER);


            while ((dp = readdir(dir)) != NULL)                 
            {
                // Checking file types: 
                // https://stackoverflow.com/questions/1121383/counting-the-number-of-files-in-a-directory-using-c

                // We have a file
                if (dp->d_type == DT_REG)                       
                {   
                    // Lock during grep so only one thread can execute grep command per file.
                    pthread_mutex_lock(&lock[1]);

                    char* filePath;
                    filePath = (char*)malloc(BUFFER);
                    
                    // Concatenating strings: https://www.educative.io/blog/concatenate-string-c
                    strcpy(filePath, path);                         
                    strcat(filePath, "/");
                    strcat(filePath, dp->d_name);   

                    // Accessing files: https://stackoverflow.com/questions/16869467/command-line-arguments-reading-a-file
                    f = fopen(filePath, "r");               
                    
                    // Generating grep command 
                    strcpy(command, "grep ");                   
                    strcat(command, search_string);
                    strcat(command, " ");
                    strcat(command, filePath);

                    // How to redirect stdout to /dev/null: https://unix.stackexchange.com/questions/119648/redirecting-to-dev-null
                    strcat(command, " >/dev/null");             

                    int result = system(command);
                    pthread_mutex_unlock(&lock[1]);

                    // System function: 
                    // https://www.tutorialspoint.com/system-function-in-c-cplusplus#:~:text=The%20system()%20function%20is,%3Cstdlib.
                    if(!result)                        
                        printf("[%d] PRESENT %s\n", id, filePath);
                    else
                        printf("[%d] ABSENT %s\n", id, filePath);

                    fclose(f);

                    free(filePath);
                    
                }
                // Check if directory is parent or current: 
                // https://stackoverflow.com/questions/50205605/how-to-figure-out-if-the-current-directory-is-the-root-in-c
                else if(strcmp(dp->d_name, "..") != 0 && strcmp(dp->d_name, ".") != 0)  
                {
                    // Same Lock during enqueue so only one thread can enqueue per directory discovered. 
                    pthread_mutex_lock(&lock[1]);

                    // Create next path with new found folder
                    char *nextPath;
                    nextPath = (char*)malloc(BUFFER);
                    strcpy(nextPath, path);
                    strcat(nextPath, "/");
                    strcat(nextPath, dp->d_name);
                    
                    // Enqueue new found folder
                    enqueue(nextPath);
                    visitedFolders++;

                    pthread_mutex_unlock(&lock[1]);

                    printf("[%d] ENQUEUE %s\n", id, nextPath);
                    
                }
            }

            closedir(dir);
            free(command);
            free(path);

        }
    }

    free(arg);
}

int main(int argc, char *argv[])
{
    int workers = atoi(argv[1]);

    // search_string can work with strings that has multiple strings eg. "hello world" but   
    // with the assumption that the strings are only separated with a single space. Otherwise,
    // this will not work as expected.
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
    
    // Initialize Threads
    thread = malloc(sizeof(pthread_t)*workers);

    // Initialize queue
    q = (struct taskQueue*)malloc(sizeof(struct taskQueue));

    // Enqueue root
    char* absolutePath;
    absolutePath = (char*)malloc(BUFFER);
    // Get absolute path
    realpath(argv[2], absolutePath);
    struct task* temp = newTask(absolutePath);
    q->front = q->rear = temp;

    // Auxiliary function to count the number of enqueues 
    // that we will expect to end the infinite loop
    traverse(absolutePath);
    
    // Start worker threads
    for(int i = 0; i < workers; i++)
    {
        int *arg = malloc(sizeof(*arg));
        
        *arg = i;
        pthread_create(&thread[i], NULL, (void *) fn, arg);
    }

    for(int i = 0; i < workers; i++)
    {
        pthread_join(thread[i], NULL);
    }
    
    free(q);
    free(thread);
    free(search_string);
    
    return 0;
}