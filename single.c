#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <string.h>
#include <pthread.h>

#define BUFFER 1000

int count = 0;
int workers = 0;
int currentWorkers = 0;
char* search_string;

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

char* getAbsolutePath(char* path)
{
    char* actualPath;
    actualPath = (char*)malloc(BUFFER);
    char *ptr;
    ptr = realpath(path, actualPath);           // Getting absolute path: https://stackoverflow.com/questions/229012/getting-absolute-path-of-a-file

    return actualPath;
}

struct taskQueue* q;
pthread_t * thread;
// struct task* temp = newTask(getAbsolutePath(dir));


// struct taskQueue* createQueue(char* dir)
// {
//     struct taskQueue* q = (struct taskQueue*)malloc(sizeof(struct taskQueue));
//     struct task* temp = newTask(getAbsolutePath(dir));
//     q->front = q->rear = temp;
    
//     return q;
// }

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

    printf("[%d] DIR %s\n", id, getAbsolutePath(dir));
    free(temp);
    return dir;
}

// int countElements(struct task* head) {
//     int count = 0;

//     while (head != NULL) {
//         count++;
//         head = head->next;
//     }

//     return count;
    
// }

int parallelGrep(char* path)
{

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
    count--;
    dir = opendir(path);                          // Traversing folders in C recursively: https://iq.opengenus.org/traversing-folders-in-c/

    FILE *f;
    char *command;

    command = (char*)malloc(BUFFER);

    while ((dp = readdir(dir)) != NULL)                 // File
    {
        char* filePath;
        filePath = (char*)malloc(BUFFER);
        
        strcpy(filePath, path);                         // Concatenating strings: https://www.educative.io/blog/concatenate-string-c
        strcat(filePath, "/");
        strcat(filePath, dp->d_name);

        char* absolutePath;
        absolutePath = getAbsolutePath(filePath);

        if (dp->d_type == DT_REG)                       // Checking file types: https://stackoverflow.com/questions/1121383/counting-the-number-of-files-in-a-directory-using-c
        {                                      
            f = fopen(absolutePath, "r");               // Accessing files: https://stackoverflow.com/questions/16869467/command-line-arguments-reading-a-file
            
            strcpy(command, "grep ");                   // grep 
            strcat(command, search_string);
            strcat(command, " ");
            strcat(command, absolutePath);
            strcat(command, " >/dev/null");             // How to redirect stdout to /dev/null: https://unix.stackexchange.com/questions/119648/redirecting-to-dev-null
     
            if(!system(command))                        // System function: https://www.tutorialspoint.com/system-function-in-c-cplusplus#:~:text=The%20system()%20function%20is,%3Cstdlib.
                printf("[%d] PRESENT %s\n", id, absolutePath);
            else
                printf("[%d] ABSENT %s\n", id, absolutePath);

            fclose(f);
        }
        else if(strcmp(dp->d_name, "..") != 0 && strcmp(dp->d_name, ".") != 0)  // Check if directory is parent or current: https://stackoverflow.com/questions/50205605/how-to-figure-out-if-the-current-directory-is-the-root-in-c
        {
            char *nextPath;
            nextPath = (char*)malloc(BUFFER);
            strcpy(nextPath, path);
            strcat(nextPath, "/");
            strcat(nextPath, dp->d_name);
            count++;
            enqueue(id, getAbsolutePath(nextPath));
        }
    }
    closedir(dir);

    visit(id);

    return 0;
}

int main(int argc, char *argv[])
{
    workers = atoi(argv[1]);
    search_string = argv[3];
    thread = malloc(sizeof(pthread_t)*workers);

    q = (struct taskQueue*)malloc(sizeof(struct taskQueue));
    struct task* temp = newTask(getAbsolutePath(argv[2]));
    q->front = q->rear = temp;

    // for(int i = 0; i < atoi(argv[1]); i++)
    // {
    //     printf("[%d] Initialized Worker Thread\n", i);
    // }

    // currentWorkers++;
    pthread_create(&thread[0], NULL, (void *) visit, 0);

    pthread_join(thread[0], NULL);

    return 0;
}