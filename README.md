# Multithreaded-and-Singlethreaded-Implementation-of-Grep-in-Linux

Project in CS 140: Operating Systems

This is a demonstration of the step by step process of how threads divide the workload of searching for all the files that contains matched strings through a specified directory.

To run **multithreaded**, 
- compile multithreaded.c
- execute compiled via the command "./<filename> <number of workers> <directory> <string to match>"
- terminal will output once a file gets enqueued to the task queue and when it gets dequeued and assigned to a worker thread. It will also indicate whether this file contains a matches string or not.

To run **singlethreaded**,
- compile singlethreaded.c
- execute compiled via the command "./<filename> <number of workers> <directory> <string to match>"
    # note that for singlethreaded, it will ignore what was placed on the number of workers and will only operate on the main thread.
- outputs will be the same with multithreaded

Video demo: https://drive.google.com/file/d/1bDYsRsY3KGHblgfcMSqKn4yRsyBfe4DJ/view
