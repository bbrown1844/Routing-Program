#include <stdio.h>

typedef struct list
{
  char machine[100];
  char name[100];
  char ip[100];
  int port;
  int id;
}list;

/*
    * void* threadf1(void* argv)
    *
    * create thread1 to take in strings from other nodes
    *
    * @params {void*} argv - file1 to be parsed
    *
    * @returns {void*}
    * Side Effect: thread shares global variables
    *
*/
void* threadf1(void* argv);

/*
    * void* threadf1(void* argv)
    *
    * create thread2 to take in input from keyboard and update cost matrix
    *
    * @params {void*} argv - file1 to be parsed
    *
    * @returns {void*}
    * Side Effect: thread shares global variables
    *
*/
void* threadf2(void* argv);

/*
    * void* threadf1(void* argv)
    *
    * create thread3 to run Dijkstra's algorithm and update cost table 
    *
    * @params {void*} argv - file1 to be parsed.
    *
    * @returns {void*}
    * Side Effect: thread shares global variables
    *
*/
void* threadf3(void* argv);

/*
    * void parsef1(FILE* f1)
    *
    * parses the data from f1 and inputs it into cost matrix 
    *
    * @params {FILE*} f1 - file1 to be parsed.
    *
    * @returns {void*}
    * Side Effect: 
    *
*/
void parsef1(FILE* f1);

/*
    * void parsef1(FILE* f1)
    *
    * parses the data from f2 and inputs it into arr
    *
    * @params {FILE*} f2 - file2 to be parsed.
    *
    * @returns {void*}
    * Side Effect: 
    *
*/
void parsef2(FILE* f2);

/*
    * min(int dist[], int set[])
    *
    * gets the min distance to a vertex
    *
    * @params {int} dist[] - output array, contains the least cost paths
    *
    * @returns {int}
    * Side Effect: updates dist[] 
    *
*/
int min(int dist[], int set[]);

/*
    * void dijkstra(int src)
    *
    * runs dijkstra's algorithm and print out least cost array 
    *
    * @params {int} src - source vector, ID of the curent router
    *
    * @returns {void}
    * Side Effect: updates lca 
    *
*/
void dijkstra(int src);