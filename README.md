# Histogram
This program shows how to use multiple processes, signal handling, pipes for inter-process communication, and file I/O in the programming language C. It works by creating child processes to compute the character frequencies in each file you provide as a command-line argument. Each child process writes its results to a separate file named file<child_pid>.hist. The program uses pipes to communicate between the parent and child processes and handles the termination of child processes using the SIGCHLD signal.

This project was created for an academic purpose assignment.

To compile the file, use command: gcc -o histogram histogram.c.
To run the file, run it through the terminal using command: ./histogram <textfile>.txt
