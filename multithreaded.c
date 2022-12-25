#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>

#define BUFFER 1000

void *grepThread(char *argv[]) 
{
    DIR *dir;
    struct dirent *dp;
    char *file_name;

    dir = opendir(argv[2]);                             // Traversing folders in C recursively: https://iq.opengenus.org/traversing-folders-in-c/

    FILE *f;
    char *filePath, *command;

    filePath = (char*)malloc(BUFFER);
    command = (char*)malloc(BUFFER);

    while ((dp = readdir(dir)) != NULL)
    {
        if (dp->d_type == DT_REG)                       // Checking file types: https://stackoverflow.com/questions/1121383/counting-the-number-of-files-in-a-directory-using-c
        {                                      
            strcpy(filePath, argv[2]);                  // Concatenating strings: https://www.educative.io/blog/concatenate-string-c
            strcat(filePath, "/");
            strcat(filePath, dp->d_name);

            f = fopen(filePath, "r");                   // Accessing files: https://stackoverflow.com/questions/16869467/command-line-arguments-reading-a-file
            
            strcpy(command, "grep ");
            strcat(command, argv[3]);
            strcat(command, " ");
            strcat(command, filePath);
            strcat(command, " >/dev/null");             // How to redirect stdout to /dev/null: https://unix.stackexchange.com/questions/119648/redirecting-to-dev-null
     
            char actualPath[BUFFER];
            char *ptr;
            ptr = realpath(filePath, actualPath);       // Getting absolute path: https://stackoverflow.com/questions/229012/getting-absolute-path-of-a-file

            pid_t tid = syscall(SYS_gettid);

            if(!system(command))                        // System function: https://www.tutorialspoint.com/system-function-in-c-cplusplus#:~:text=The%20system()%20function%20is,%3Cstdlib.
            {
                printf("[%d] PRESENT %s\n", tid, actualPath);
            }
            else
            {
                printf("[%d] ABSENT %s\n", tid, actualPath);
            }

            fclose(f);
        }
    }
    closedir(dir);

    return NULL;
}

int main(int argc, char *argv[])
{
    pthread_t *threads = malloc(sizeof(pthread_t) * atoi(argv[1]));     // Creating n threads: https://stackoverflow.com/questions/20276010/c-creating-n-threads
    int ret = -1;

    for(int i = 0; i < atoi(argv[1]); i++)
    {
        pthread_create(&threads[i], NULL, (void *) grepThread, argv);
    }

    for(int i = 0; i < atoi(argv[1]); i++)
    {
        pthread_join(threads[i], NULL);
    }

    return 0;
}