#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include "router.h"

typedef int bool;
#define true 1
#define false 0

#define INF 10000

//need to run using -lthread

pthread_mutex_t cmMutex = PTHREAD_MUTEX_INITIALIZER;
int **cost_matrix;
int *lca;
int size;
list * arr;



int main(int argc, char *argv[])
{
  pthread_t thread1, thread2, thread3;
  FILE *f1, *f2;
  int iret1, iret2, iret3, k;
  
  if (argc != 4)
  {
    fprintf(stderr, "Command Line Format: <router_id> <cost_file> <route>\n");
    exit(EXIT_FAILURE);
  }

  //seed random function
  srand(time(NULL));

  //open cost file for reading
  f1 = fopen (argv[2], "r+");
  if (f1 == NULL) {fputs ("File error",stderr); exit(EXIT_FAILURE);}

  //open router info file for reading
  f2 = fopen (argv[3], "r+");
  if (f2 == NULL) {fputs ("File error",stderr); exit(EXIT_FAILURE);}

  //parse file1 into cost matrix
  parsef1(f1);
  //parse file2 into an array of structs 
  parsef2(f2);

  //create thread and pass in file1 as a parameter
  iret1 = pthread_create(&thread1, NULL, threadf1, (void *) argv[1]);
  iret2 = pthread_create(&thread2, NULL, threadf2, (void *) argv[1]);
  iret3 = pthread_create(&thread3, NULL, threadf3, (void *) argv[1]);
  
	if(iret1)
	{
		fprintf(stderr, "Error - pthread_create() return code: %d\n", iret1);
	  exit(EXIT_FAILURE);
	}
	
	if(iret2)
	{
		fprintf(stderr, "Error - pthread_create() return code: %d\n", iret2);
	  exit(EXIT_FAILURE);
  }
  
  if(iret3)
	{
		fprintf(stderr, "Error - pthread_create() return code: %d\n", iret2);
	  exit(EXIT_FAILURE);
	}
  
  //wait for threads to finish
	pthread_join(thread1, NULL);
  pthread_join(thread2, NULL);
  pthread_join(thread3, NULL);

  //free all allocated data structures
  free(arr);
  for(k = 0; k < size; k++)
    free(cost_matrix[k]);
  free(cost_matrix);
  free(lca);
	exit(EXIT_SUCCESS);

}

//loops forever, receives messages from other nodes and updates the cost_matrix
void* threadf1(void* argv)
{
  //get and cast new ID passed in as parameter
  char * temp = argv;
  int ID = atoi(temp);

  int sock;
  struct sockaddr_in serverAddr, clientAddr;
  struct sockaddr_storage serverStorage;
  socklen_t addr_size, client_addr_size;
  char buff[50];
  struct timeval timer;
  
  FILE *f2;
  char router[10], neighbor[10], cost[10];
  int routerID, neighborID, newCost;
   
  // init
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons ((short) arr[ID].port);
  serverAddr.sin_addr.s_addr = htonl (INADDR_ANY);
  memset ((char *)serverAddr.sin_zero, '\0', sizeof (serverAddr.sin_zero));
  addr_size = sizeof (serverStorage);
  
  // create socket
  if ((sock = socket (AF_INET, SOCK_DGRAM, 0)) < 0)
  {
      printf ("socket error\n");
      exit(EXIT_FAILURE);
  }
  
  // bind
  if (bind (sock, (struct sockaddr *)&serverAddr, sizeof (serverAddr)) != 0)
  {
      printf ("bind error\n");
      exit(EXIT_FAILURE);
  }
  
  while(1)  
  {
    //<router id> <neighbor id> <new cost>
    recvfrom (sock, buff, sizeof(buff), 0, (struct sockaddr *)&serverStorage, &addr_size);
    
    //parse the message
    sscanf(buff, "%s %s %s", router, neighbor, cost);
    printf("buff: %s\n", buff);
    routerID = atoi(router);
    neighborID = atoi(neighbor);
    newCost = atoi(cost);
    
    //update tables
    pthread_mutex_lock(&cmMutex);
    cost_matrix[routerID][neighborID] = newCost;
    cost_matrix[neighborID][routerID] = newCost;
    pthread_mutex_unlock(&cmMutex); 
  }
  return NULL;
}

void* threadf2(void* argv)
{
  //client setup
  int sock, portNum;
  struct sockaddr_in serverAddr;
  char *tempID = argv;
  socklen_t addr_size;
  
  int myID = atoi(tempID);
  int count = 0, i;
  char cost[10];
  char neighbor[10];
  char message[50];
  
  while(count < 10)
  {
    //read from keyboard every 10 seconds
    printf("Enter: <Neighbor ID> <New cost>\n");
    scanf("%s %s", neighbor, cost);

    int costNum = atoi(cost);
    int neighborNum = atoi(neighbor);
    
    pthread_mutex_lock(&cmMutex);
    //update the cost matrix for the current vertex
    cost_matrix[myID][neighborNum] = costNum;
    cost_matrix[neighborNum][myID] = costNum;
    pthread_mutex_unlock(&cmMutex);

    //create message to send
    memset(message, 0, 50);
    strcat(message,tempID);
    strcat(message, " ");
    strcat(message, neighbor);
    strcat(message, " ");
    strcat(message, cost);

    //loop through 
    for(i = 0; i < size; i++)
    {
      if(i == myID)
          continue;

      // configure address, send message
      serverAddr.sin_family = AF_INET;
      serverAddr.sin_port = htons (arr[i].port);
      inet_pton (AF_INET, arr[i].ip, &serverAddr.sin_addr.s_addr);
      memset (serverAddr.sin_zero, '\0', sizeof (serverAddr.sin_zero));
      addr_size = sizeof serverAddr;
      //create socket
      sock = socket(PF_INET, SOCK_DGRAM, 0);
      printf("sending: %s\n", message);
      sendto (sock, message, sizeof message, 0, (struct sockaddr *)&serverAddr, addr_size);
      close(sock);
    }
    count++;
    sleep(5);
  }
  
  sleep(30);
  return NULL;
}

void* threadf3(void* argv)
{
  int i, j;
  char * temp = argv;
  int ID = atoi(temp);
    
  while(1)
  {
    sleep(rand() % (11) /*+ 10*/);
    dijkstra(ID);     
  }

  return NULL;
}

void parsef1(FILE * f1)
{
  int cost, i, j; 
  char ch;
  char buff[50];
  size = 0;

  //count the number of lines 
  while(!feof(f1))
  {
    ch = fgetc(f1);
    if(ch == '\n')
      size++;
  }
    
  //check to see if last line has newline character
  fseek(f1,-1,SEEK_CUR);
  ch = fgetc(f1);
  if (ch != '\n')
    size++;

  rewind(f1);

  //allocate cost matrix
  cost_matrix = (int**)malloc(size * sizeof(int*));

  //allocate lca
  lca = (int*)malloc(size * sizeof(int));

  //create a 2D array to parse file1 into 
  for(i = 0; i < size; i++)
      cost_matrix[i] = (int *)malloc(size * sizeof(int));

  //parse in data from f1
  for(i = 0; i < size; i++)
    for(j = 0; j < size; j++)
    {
      fscanf(f1,"%s", buff);
      cost = atoi(buff);
      cost_matrix[i][j] = cost;
    }
}

void parsef2(FILE* f1)
{ 
  int i;
  //create array of structs 
  arr = (list *)malloc(size * sizeof(list));

  //enter in input from csv file to array of structs
  for (i = 0; i < size; i++)
  {
    fscanf(f1, "%s %s %s %d", arr[1].machine, arr[i].name, arr[i].ip, &arr[i].port);
    arr[i].id = i;
  } 
}

//Dijkstra's Algorithm
  
int min(int dist[], int set[])
{
  int s;
  // Initialize min value
  int min = INF, min_index;

  for (s = 0; s < size; s++)
    //if vertex hasn't been visited and weight is smaller than min
    if (set[s] == 0 && dist[s] <= min)
        min = dist[s], min_index = s;
  
  return min_index;
}
  
void dijkstra(int src)
{
  int tsize = size;
  int count, i, s;

  //output array
  int dist[tsize];     
  
  //visited nodes
  int set[tsize]; 
  
  //set all values to infinite and all nodes as unvisited 
  for (i = 0; i < tsize; i++)
    dist[i] = INF, set[i] = 0;

  //distance to source node
  dist[src] = 0;

  for (count = 0; count < tsize-1; count++)
  {
    //find minimum vertex not yet visisted 
    int u = min(dist, set);

    set[u] = 1;

    for (s = 0; s < tsize; s++)
    {
      //dist gets updated if a node that hasn't been visited yet has a smaller path to another vertext than current
      if ((cost_matrix[u][s] != INF) && !set[s] && dist[u] != INF && dist[u]+cost_matrix[u][s] < dist[s])
        dist[s] = dist[u] + cost_matrix[u][s];
    }
  }
  // print the least cost array
  printf("lca: ");
  for (i = 0; i < tsize; i++)
  {
    lca[i] = dist[i];
    printf("%d ", lca[i]);
  }
  printf("\n");
}