/*
 * Author: Chris Jones
 *
 * tsh - A tiny shell program
 *
 * DISCUSS YOUR IMPLEMENTATION HERE!
 *******************************************************************************
 * sigchld_handler:
 *
 * In my sigchld_handler we enter a while loop checking to see if waitpid 
 * returned greater than 0, a 0 return value being if WNOHANG was specified 
 * on children whose state have not changed, and -1 on error.
 * On success the pid of the child whose state changed will return.  
 *  
 * Next we check a series of conditions using macros.
 *
 * First we check if a child terminated abnormally with the status integer 
 * saved by waitpid, if so we check if our pid is the same as the
 * current foreground pid, then we reset the foreground pid back to 0.
 *
 * Next we check if a child process was terminated by a signal, 
 * then  again we check if our pid is the same as the current foreground pid,
 * if it is then we reset the current foreground pid. 
 * Afterwards (or Otherwise) we output our format string with the number of
 * the signal that caused the child process to terminate using wtermsig
 * 
 * Finally we check if the child process was terminated by delivery of a
 * signal.
 * (according to the linux page this can only be possible if WUNTRACED is used)
 * If so we check if the pid is equal to the foreground pid,
 * if they are equal we set the suspended pid to store the value of our 
 * foreground pid, then we reset the foreground pid.
 * After the flow of procedure jumps out of that loop we output
 * our format strings, this time we use wstopsig which returns the number
 * of the signal which caused the child to stop
 *******************************************************************************
 * waitfg - waits for a foreground job to complete:
 *
 * While we're in the foreground we temporarily replace the signal mask of the 
 * calling thread with the mask, and then we suspend that threat until a 
 * signal is delivered.
 *
 * The signal must have an action to invoke a signal handler or terminate a 
 * process.
 *******************************************************************************
 * sigint_handler & sigtstp_handler:
 *
 * Both functions check to see that the foreground process isn't equal to 0
 * because if we're in the foreground when we call the signal the pid should
 * be valid.
 *
 * After that condition check we send a signal to the foreground process 
 * group. 
 *
 * In sigint_handler we call kill(-g_runningPid, SIGINT), which in turn
 * sends a SIGINT signal to the foreground process group
 *
 * In sigtstp_handler we call kill(-g_runningPid, SIGTSTP), which in turn
 * sends a SIGTSTP signal to the foreground process group suspending the 
 * foreground job
 *
 */

/*
 *******************************************************************************
 * INCLUDE DIRECTIVES
 *******************************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/*
 *******************************************************************************
 * TYPE DEFINITIONS
 *******************************************************************************
 */

typedef void handler_t (int);

/*
 *******************************************************************************
 * PREPROCESSOR DEFINITIONS
 *******************************************************************************
 */

// max line size 
#define MAXLINE 1024

// max args on a command line 
#define MAXARGS 128

/*
 *******************************************************************************
 * GLOBAL VARIABLES
 *******************************************************************************
 */

// defined in libc
extern char** environ;   

// command line prompt 
char prompt[] = "tsh> ";

// for composing sprintf messages
char sbuf[MAXLINE];

// PID of the foreground process
volatile pid_t g_runningPid = 0;

// PID of the suspended process
volatile pid_t g_suspendedPid = 0; 

/*
 *******************************************************************************
 * FUNCTION PROTOTYPES
 *******************************************************************************
 */

int
parseline (const char* cmdline, char** argv);

void
eval (char* cmdline);

int
builtin_command (char** argv);

pid_t
Fork (void);

void
waitfg (pid_t pid);

void
sigint_handler (int sig);

void
sigtstp_handler (int sig);

void
sigchld_handler (int sig);

void
sigquit_handler (int sig);

void
unix_error (char* msg);

void
app_error (char* msg);

handler_t*
Signal (int signum, handler_t* handler);

/*
 *******************************************************************************
 * MAIN
 *******************************************************************************
 */

int
main (int argc, char** argv)
{
  /* Redirect stderr to stdout */
  dup2 (1, 2);
  /* Install signal handlers */
  Signal (SIGINT, sigint_handler);   /* ctrl-c */
  Signal (SIGTSTP, sigtstp_handler); /* ctrl-z */
  Signal (SIGCHLD, sigchld_handler); /* Terminated or stopped child */
  Signal (SIGQUIT, sigquit_handler); /* quit */
  /* TODO -- shell goes here*/ 
  
  char cmdline[MAXLINE];
  
  while (1)
  {
    printf("tsh> ");
    fgets(cmdline, MAXLINE, stdin);
    if(feof(stdin))
      exit(0);
    eval(cmdline);
  }
  
  /* Quit */
  exit (0);
}

/*
*  parseline - Parse the command line and build the argv array.
*
*  Characters enclosed in single quotes are treated as a single
*  argument.
*
*  Returns true if the user has requested a BG job, false if
*  the user has requested a FG job.
*/

int
parseline (const char* cmdline, char** argv)
{
  static char array[MAXLINE]; /* holds local copy of command line*/
  char* buf = array;          /* ptr that traverses command line*/
  char* delim;                /* points to first space delimiter*/

  strcpy (buf, cmdline);
  buf[strlen (buf) - 1] = ' ';  /* replace trailing '\n' with space*/
  while (*buf && (*buf == ' ')) /* ignore leading spaces*/
    buf++;
  
  /* Build the argv list*/
  int argc = 0; /* number of args*/
  if (*buf == '\'')
  {
    buf++;
    delim = strchr (buf, '\'');
  }
  else
  {
    delim = strchr (buf, ' ');
  }

  while (delim)
  {
    argv[argc++] = buf;
    *delim = '\0';
    buf = delim + 1;
    
    while (*buf && (*buf == ' ')) /* ignore spaces*/
      buf++;
    
    if (*buf == '\'')
    {
      buf++;
      delim = strchr (buf, '\'');
    }
    else
    {
      delim = strchr (buf, ' ');
    }
  }

  argv[argc] = NULL;
  if (argc == 0) /* ignore blank line*/
    return 1;
  
  /* should the job run in the background?*/
  int bg; /* background job?*/
  if ((bg = (*argv[argc - 1] == '&')) != 0)
  {
    argv[--argc] = NULL;
  }
  return bg;
}

/*
* eval - evaluates the command line
*
* Checks to see if the arguments are not a built in function,
* then blocks sigchild.
* Then forks a process and unblocks sigchild before we execve 
* as well as error checks for execve.
*
* Next we check to see if we're not in the background, 
* if not we unblock sigchild to receive signals,
* and wait for the foreground process to finish.
*
* If we are in the background we unblock sigchild in order to receive signals
* and print our output message while setting the pgid to pid
*
*/

void
eval(char* cmdline)
{
  char *argv[MAXARGS];
  char buf[MAXLINE];
  strcpy(buf, cmdline);

  /* stores our background value from buffer contents*/
  int bg = parseline(buf, argv); 
  if(argv[0] == NULL)
  {
    return;
  }
  sigset_t mask_all, mask_one, prev_one;
  sigfillset(&mask_all); /* initializes set to full */
  sigemptyset(&mask_one); /* initialize signal set given by set to empty*/
  sigaddset(&mask_one, SIGCHLD); /* adds sigchild to the signal set to be
  blocked */

  pid_t pid;
  if(!builtin_command(argv))
  {
    sigprocmask(SIG_BLOCK, &mask_one, &prev_one);
    if((pid = Fork()) == 0)
    {
      sigprocmask(SIG_SETMASK, &prev_one, NULL);
      if(execve(argv[0], argv, environ) < 0)
      {
        fprintf(stderr, "%s: Command not found\n", argv[0]);
        exit(0);
      }
    }

    if(!bg)
    {
      g_runningPid = pid;
      sigprocmask(SIG_SETMASK, &prev_one, NULL);
      setpgid(pid,pid);
      waitfg(g_runningPid);
    }
    else
    {
      sigprocmask(SIG_SETMASK, &prev_one, NULL);
      int thing = pid;
      printf("(%d) %s", thing, cmdline);
      setpgid(pid,pid);
    }
  }   
}

/*
* builtin_cmd - recognize and interprets the built-in commands (quit & fg)
*
* Checks to see if first argument is quit, exits immediately if so.
*
* Checks to see if first argument is fg. 
* If so send sigcont to the suspended process,
* set the fg pid to the restored suspended processand reset the suspended pid.
* Afterwards wait for the foreground process to finish.
*
* Returns true on completion.
*
* Returns false if no built-in commands. 
*
*/

int
builtin_command(char **argv)
{
  if(!strcmp(argv[0], "quit"))
  {
    exit(0);
  }
  if(strcmp(argv[0], "fg") == 0)
  {
    //check to see if a job is suspended
    if(g_suspendedPid != 0)
    {
      kill(-g_suspendedPid, SIGCONT);
      g_runningPid = g_suspendedPid;
      g_suspendedPid = 0;
      waitfg(g_runningPid);
    }
    return 1;
  } 
  return 0;
}

/*
* Fork - wrapper function that creates a child process and error checks.
*
* Returns the pid of the child process
*/

pid_t
Fork (void)
{
  pid_t pid;
  if((pid = fork()) < 0)
  {
    unix_error("Fork error");
  }
  return pid;
}

void
waitfg(pid_t pid)
{
  sigset_t mask;
  sigemptyset(&mask); /* initialize the signal set given to empty */
 
  if(sigemptyset(&mask) < 0)
    perror("sigemptyset");
  while(pid == g_runningPid)
  {
   /* suspends calling thread until signal handler action is invoked or process is terminated */
    sigsuspend(&mask);
  }
}

/*
 *******************************************************************************
 * SIGNAL HANDLERS
 *******************************************************************************
 */

/*
*  sigchld_handler - The kernel sends a SIGCHLD to the shell whenever
*      a child job terminates (becomes a zombie), or stops because it
*      received a SIGSTOP or SIGTSTP signal. The handler reaps all
*      available zombie children, but doesn't wait for any other
*      currently running children to terminate.
*/

void
sigchld_handler (int sig)
{
  pid_t pid;
  int status;
  (void) signal;
  while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0)
  {
    if(WIFEXITED(status))
    {
       if(pid == g_runningPid)
       {
        g_runningPid = 0;
       }
    }

    else if(WIFSIGNALED(status))
    {
      if(pid == g_runningPid)
      {
        g_runningPid = 0;
      }
      printf("Job (%d) terminated by signal %d\n", pid, WTERMSIG(status));
      fflush(stdout);
    }

    else if(WIFSTOPPED(status))
    {
      
      if(pid == g_runningPid)
      {
        g_suspendedPid = g_runningPid;
        g_runningPid = 0;
      }
      printf("Job (%d) stopped by signal %d\n", pid, WSTOPSIG(status));
      fflush(stdout);
    }
  }
}

/*
 *  sigint_handler - The kernel sends a SIGINT to the shell whenever the
 *     user types ctrl-c at the keyboard.  Catch it and send it along
 *     to the foreground job.
 */

void
sigint_handler (int sig)
{
  //pid_t pid = g_runningPid;
  if(g_runningPid != 0)
  {
    kill(-g_runningPid, SIGINT);
  } 
}

/*
 *  sigtstp_handler - The kernel sends a SIGTSTP to the shell whenever
 *      the user types ctrl-z at the keyboard. Catch it and suspend the
 *      foreground job by sending it a SIGTSTP.
 */

void
sigtstp_handler (int sig)
{
  //pid_t pid = g_runningPid;
  if(g_runningPid != 0)
  {
    kill(-g_runningPid, SIGTSTP);
  }
}

/*
 *******************************************************************************
 * HELPER ROUTINES
 *******************************************************************************
 */

/*
 * unix_error - unix-style error routine
 */

void
unix_error (char* msg)
{
  fprintf (stdout, "%s: %s\n", msg, strerror (errno));
  exit (1);
}

/*
*  app_error - application-style error routine
*/

void
app_error (char* msg)
{
  fprintf (stdout, "%s\n", msg);
  exit (1);
}

/*
*  Signal - wrapper for the sigaction function
*/

handler_t*
Signal (int signum, handler_t* handler)
{
  struct sigaction action, old_action;
  action.sa_handler = handler;
  sigemptyset (&action.sa_mask); /* block sigs of type being handled*/
  action.sa_flags = SA_RESTART;  /* restart syscalls if possible*/
  if (sigaction (signum, &action, &old_action) < 0)
}
