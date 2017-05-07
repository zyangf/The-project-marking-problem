/*
 * This is a skeleton program for COMSM2001 (Server Software) coursework 1
 * "the project marking problem". Your task is to synchronise the threads
 * correctly by adding code in the places indicated by comments in the
 * student, marker and run functions.
 * You may create your own global variables and further functions.
 * The code in this skeleton program can be used without citation in the files
 * that you submit for your coursework.
 */

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

/*
 * Parameters of the program. The constraints are D < T and
 * S*K <= M*N.
 */
struct demo_parameters {
    int S;   /* Number of students */
    int M;   /* Number of markers */
    int K;   /* Number of markers per demo */
    int N;   /* Number of demos per marker */
    int T;   /* Length of session (minutes) */
    int D;   /* Length of demo (minutes) */
};

/* Global object holding the demo parameters. */
struct demo_parameters parameters;

/* The demo start time, set in the main function. Do not modify this. */
struct timeval starttime;

/*
 * You may wish to place some global variables here.
 * Remember, globals are shared between threads.
 * You can also create functions of your own.
 */
pthread_mutex_t process;
pthread_cond_t StudentGrabbed;
pthread_cond_t StudentStart;
pthread_cond_t StudentFinished;

int allStudent[100];
int grabbedMarker[100];
int finishDemo[100];
int bufferHead;
int bufferTail;
/*
 * timenow(): returns current simulated time in "minutes" (cs).
 * Assumes that starttime has been set already.
 * This function is safe to call in the student and marker threads as
 * starttime is set in the run() function.
 */
int timenow() {
    struct timeval now;
    gettimeofday(&now, NULL);
    return (now.tv_sec - starttime.tv_sec) * 100 + (now.tv_usec - starttime.tv_usec) / 10000;
}

/* delay(t): delays for t "minutes" (cs) */
void delay(int t) {
    struct timespec rqtp, rmtp;
    t *= 10;
    rqtp.tv_sec = t / 1000;
    rqtp.tv_nsec = 1000000 * (t % 1000);
    nanosleep(&rqtp, &rmtp);
}

/* panic(): simulates a student's panicking activity */
void panic() {
    delay(random() % (parameters.T - parameters.D));
}

/* demo(): simulates a demo activity */
void demo() {
    delay(parameters.D);
}

/*
 * A marker thread. You need to modify this function.
 * The parameter arg is the number of the current marker and the function
 * doesn't need to return any values.
 * Do not modify the printed output as it will be used as part of the testing.
 */
void *marker(void *arg) {
    int markerID = *(int *)arg;

    /*
     * The following variable is used in the printf statements when a marker is
     * grabbed by a student. It shall be set by this function whenever the
     * marker is grabbed - and before the printf statements referencing it are
     * executed.
     */
    int studentID, job;

    /* 1. Enter the lab. */
    printf("%d marker %d: enters lab\n", timenow(), markerID);
    for(job = 0; job < parameters.N; job++)
    {
      /* A marker marks up to N projects. */
      if(timenow()>(parameters.T- parameters.D))
      {
        printf("%d marker %d: exits lab (timeout)\n", timenow(), markerID);
        break;
      }
      if( pthread_mutex_lock(&process)!=0)
      {
          perror("pthread_mutex_lock() error");
          exit(1);
      }
      //if(err){abort();}
      //printf("%s lock successful!\n" );

    /* 2. Repeat (N times).
     *    (a) Wait to be grabbed by a student.
     *    (b) Wait for the student's demo to begin
     *        (you may not need to do anything here).
     *    (c) Wait for the demo to finish.
     *        Do not just wait a given time -
     *        let the student signal when the demo is over.
     *    (d) Exit the lab.
     */
    /*
     * 3. If the end of the session approaches (i.e. there is no time
     *    to start another demo) then the marker waits for the current
     *    demo to finish (if they are currently attending one) and then
     *    exits the lab.
     */
    while(bufferHead == bufferTail && timenow()<(parameters.T - parameters.D))
     {
      // printf("%s wait successful!\n" );
       if(pthread_cond_wait(&StudentGrabbed, &process)!=0)
       {
         perror("pthread_cond_wait() error");
         exit(1);
       }
     }
     studentID = allStudent[bufferTail];
     grabbedMarker[studentID]--;
     if(grabbedMarker[studentID] == 0)
     {
       bufferTail++;
       bufferTail = bufferTail % parameters.S;
     }
     if(pthread_mutex_unlock(&process)!=0)
     {
       perror("pthread_mutex_unlock() error");
       exit(1);
     }

    /* The following line shall be printed when a marker is grabbed by a student. */
    if(pthread_mutex_lock(&process)!=0)
    {
      perror("pthread_mutex_lock() error");
      exit(1);
    }
    //if(err3){abort();}
      printf("%d marker %d: grabbed by student %d (job %d)\n", timenow(), markerID, studentID, job + 1);

      if(grabbedMarker[studentID] == 0)
      {
        if(pthread_cond_broadcast(&StudentStart)!=0)
        {
          perror("pthread_cond_broadcast() error!");
          exit(1);
        }
      }
      if(pthread_mutex_unlock(&process)!=0)
      {
        perror("pthread_mutex_unlock() error");
        exit(1);
      }

      if(pthread_mutex_lock(&process)!=0)
      {
        perror("pthread_mutex_lock() error");
        exit(1);
      }
      
      while(finishDemo[studentID]!=1){
        //the start 1 is means the current thread has finished, otherwise thread continue
        if(pthread_cond_wait(&StudentFinished, &process)!=0)
        {
           perror("pthread_cond_wait() error");
           exit(1);
         }
      }
      if(pthread_mutex_unlock(&process)!=0)
      {
        perror("pthread_mutex_unlock() error");
        exit(1);
      }
      
    /* The following line shall be printed when a marker has finished attending a demo. */
      printf("%d marker %d: finished with student %d (job %d)\n", timenow(), markerID, studentID, job + 1);
    }
    /*
     * When the marker exits the lab, exactly one of the following two lines shall be
     * printed, depending on whether the marker has finished all their jobs or there
     * is no time to complete another demo.
     */
     if(job == parameters.N)
    {
      printf("%d marker %d: exits lab (finished %d jobs)\n", timenow(), markerID, parameters.N);

    }
    else
    {
      printf("%d marker %d: exits lab (timeout)\n", timenow(), markerID);
    }
    return NULL;
}


/*
 * A student thread. You must modify this function.
 */
void *student(void *arg) {
    /* The ID of the current student. */
    int studentID = *(int *)arg;

    /* 1. Panic! */
    printf("%d student %d: starts panicking\n", timenow(), studentID);
    panic();

    /* 2. Enter the lab. */
    printf("%d student %d: enters lab\n", timenow(), studentID);

    /* 3. Grab K markers. */
    if(pthread_mutex_lock(&process)!=0)
    {
      perror("pthread_mutex_lock() error");
      exit(1);
    }
    
    allStudent[bufferHead] = studentID;
    bufferHead++;
    bufferHead = bufferHead % parameters.S; //avoid excessive studentID
    if(pthread_mutex_unlock(&process)!=0)
    {
      perror("pthread_mutex_unlock() error");
      exit(1);
    }

    if(pthread_mutex_lock(&process)!=0)
    {
      perror("pthread_mutex_lock() error");
      exit(1);
    }
    //if(err3){abort();}
    if(pthread_cond_broadcast(&StudentGrabbed)!=0)
    {
      perror("pthread_cond_broadcast() error!");
      exit(1);
    }
    if(pthread_mutex_unlock(&process)!=0)
    {
      perror("pthread_mutex_unlock() error");
      exit(1);
    }
    //if(err4){abort();}

    if(pthread_mutex_lock(&process)!=0)
    {
      perror("pthread_mutex_lock() error");
      exit(1);
    }
    //if(err5){abort();}
    while(grabbedMarker[studentID] > 0 && timenow() < (parameters.T - parameters.D))
    {
      if(pthread_cond_wait(&StudentStart, &process)!=0)
      {
        perror("pthread_cond_wait() error");
        exit(1);
      }
    }
    if(pthread_mutex_unlock(&process)!=0)
    {
      perror("pthread_mutex_unlock() error");
      exit(1);
    }
    //if(err6){abort();}

    /* 4. Demo! */
    /*
     * If the student succeeds in grabbing K markers and there is enough time left
     * for a demo, the following three lines shall be executed in order.
     * If the student has not started their demo and there is not sufficient time
     * left to do a full demo, the following three lines shall not be executed
     * and the student proceeds to step 5.
     */
    if(timenow() > (parameters.T- parameters.D))
    {
      if(pthread_mutex_lock(&process)!=0)
      {
        perror("pthread_mutex_lock() error");
        exit(1);
      }
        //if(err3){abort();}
        printf("%d student %d: exits lab (timeout)\n", timenow(), studentID);
        finishDemo[studentID] = 1;
        if(pthread_cond_broadcast(&StudentFinished)!=0)
        {
            perror("pthread_cond_broadcast() error!");
            exit(1);
        }
        if(pthread_mutex_unlock(&process)!=0)
        {
          perror("pthread_mutex_unlock() error");
          exit(1);
        }
        //if(err4){abort();}

    }
    /* 5. Exit the lab. */
    else{
        printf("%d student %d: starts demo\n", timenow(), studentID);
        demo();
        printf("%d student %d: ends demo\n", timenow(), studentID);
        if(pthread_mutex_lock(&process)!=0)
        {
          perror("pthread_mutex_lock() error");
          exit(1);
        }
        //if(err){abort();}
        finishDemo[studentID] = 1;
        if(pthread_cond_broadcast(&StudentFinished)!=0)
        {
            perror("pthread_cond_broadcast() error!");
            exit(1);
        }
        if(pthread_mutex_unlock(&process)!=0)
        {
          perror("pthread_mutex_unlock() error");
          exit(1);
        }
        //if(err2){abort();}
         printf("%d student %d: exits lab (finished)\n", timenow(), studentID);
    /*
     * Exactly one of the following two lines shall be printed, depending on
     * whether the student got to give their demo or not.
     */
    }
    return NULL;
}

/* The function that runs the session.
 * You MAY want to modify this function.
 */
void run() {
    int i;
    int markerID[100], studentID[100];
    pthread_t markerT[100], studentT[100];

    if(pthread_mutex_init(&process, NULL)!=0)
    {
      perror("pthread_mutex_init()error");
      exit(1);
    }
    if(pthread_cond_init(&StudentGrabbed, NULL)!=0)
    {
      perror("pthread_cond_init()error");
      exit(1);
    }
    if(pthread_cond_init(&StudentStart, NULL)!=0)
    {
      perror("pthread_cond_init() error");
      exit(1);
    }
    if(pthread_cond_init(&StudentFinished, NULL)!=0)
    {
      perror("pthread_cond_init() error");
      exit(1);
    }

    for(i=0; i< parameters.S; i++) {
              allStudent[i] = -1;
              grabbedMarker[i] = parameters.K;
              finishDemo[i] = 0;
  }
    printf("S=%d M=%d K=%d N=%d T=%d D=%d\n",
        parameters.S,
        parameters.M,
        parameters.K,
        parameters.N,
        parameters.T,
        parameters.D);
    gettimeofday(&starttime, NULL);  /* Save start of simulated time */

    /* Create S student threads */
    for (i = 0; i<parameters.S; i++) {
        studentID[i] = i;
        pthread_create(&studentT[i], NULL, student, &studentID[i]);
    }
    /* Create M marker threads */
    for (i = 0; i<parameters.M; i++) {
        markerID[i] = i;
        pthread_create(&markerT[i], NULL, marker, &markerID[i]);
    }

    /* With the threads now started, the session is in full swing ... */
    delay(parameters.T - parameters.D);

    /*
     * When we reach here, this is the latest time a new demo could start.
     * You might want to do something here or soon after.
     */

    if(pthread_cond_broadcast(&StudentGrabbed)!=0)
    {
      perror("pthread_cond_broadcast() error!");
      exit(1);
    }
    if(pthread_cond_broadcast(&StudentStart)!=0)
    {
      perror("pthread_cond_broadcast() error!");
      exit(1);
    }
    if(pthread_cond_broadcast(&StudentFinished)!=0)
    {
        perror("pthread_cond_broadcast() error!");
        exit(1);
    }
    /* Wait for student threads to finish */
    for (i = 0; i<parameters.S; i++) {
        pthread_join(studentT[i], NULL);
    }

    /* Wait for marker threads to finish */
    for (i = 0; i<parameters.M; i++) {
        pthread_join(markerT[i], NULL);
    }
}

/*
 * main() checks that the parameters are ok. If they are, the interesting bit
 * is in run() so please don't modify main().
 */
int main(int argc, char *argv[]) {
    if (argc < 6) {
        puts("Usage: demo S M K N T D\n");
        exit(1);
    }
    parameters.S = atoi(argv[1]);
    parameters.M = atoi(argv[2]);
    parameters.K = atoi(argv[3]);
    parameters.N = atoi(argv[4]);
    parameters.T = atoi(argv[5]);
    parameters.D = atoi(argv[6]);
    if (parameters.M > 100 || parameters.S > 100) {
        puts("Maximum 100 markers and 100 students allowed.\n");
        exit(1);
    }
    if (parameters.D >= parameters.T) {
        puts("Constraint D < T violated.\n");
        exit(1);
    }
    if (parameters.S*parameters.K > parameters.M*parameters.N) {
        puts("Constraint S*K <= M*N violated.\n");
        exit(1);
    }
    // We're good to go.
    run();

    return 0;
}
