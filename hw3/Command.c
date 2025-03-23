#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

// Used for managing child processes
#include <sys/types.h>
#include <sys/wait.h>

#include "Command.h"
// #include "erclearror.h"
#include "error.h"

typedef struct
{
  char *file;
  char **argv;
} *CommandRep;

#define BIARGS CommandRep r, int *eof, Jobs jobs
#define BINAME(name) bi_##name
#define BIDEFN(name) static void BINAME(name)(BIARGS)
#define BIENTRY(name) {#name, BINAME(name)}

static char *oldWD = 0;
static char *currentWD = 0;

static void builtin_args(CommandRep r, int n)
{
  char **argv = r->argv;
  for (n++; *argv++; n--)
    ;
  if (n)
    ERROR("wrong number of arguments to builtin command"); // warn
}

BIDEFN(exit)
{
  builtin_args(r, 0);
  *eof = 1;
}

BIDEFN(pwd)
{
  builtin_args(r, 0);
  if (!currentWD)
    currentWD = getcwd(0, 0);
  printf("%s\n", currentWD);
}

BIDEFN(cd)
{
  builtin_args(r, 1);

  if (strcmp(r->argv[1], "-") == 0)
  {
    // Switch to old working directory

    // Is there a current working directory?
    // if (!currentWD)
    // {
    //   // Need to adjust it
    //   currentWD=getcwd(0,0);
    // }

    // Swap current working directory and the old working directory
    char *twd = currentWD;
    currentWD = oldWD;
    oldWD = twd;
  }
  else if (strcmp(r->argv[1], "..") != 0 || strcmp(r->argv[1], "../..") != 0)
  {
    // Need to append file to the currentWD

    char *readyWD;

    if (oldWD)
      free(oldWD);
    oldWD = currentWD;

    // Is there a current working directory?
    // ? Important to check after so you don't get new stuff (I believe)
    if (!currentWD)
    {
      // Need to adjust it
      // readyWD = getcwd(0, 0);
      currentWD = getcwd(0, 0);
    }

    // Appending file onto the current path
    // readyWD = strcat(readyWD, "/");
    // currentWD = strcat(readyWD, strdup("/"));
    currentWD = strcat(currentWD, strdup("/"));
    currentWD = strcat(currentWD, strdup(r->argv[1]));

    fprintf(stdout, "Current working directory: %s\n", currentWD);
  }
  if (currentWD && chdir(currentWD))
  {
    // fprintf(stdout, "%d, %d\n", strcmp(r->argv[1], "..") != 0, strcmp(r->argv[1], "../..") != 0);

    ERROR("chdir() failed"); // warn
  }
}

// History Command
BIDEFN(history)
{
  builtin_args(r, 0);

  fprintf(stdout, "History!\n");
}

BIDEFN(debug)
{
  builtin_args(r, 0);

  // Get current oldWD and currentWD
  fprintf(stdout, "Current working directory: %s, Old working directory: %s\n", currentWD, oldWD);
}

static int builtin(BIARGS)
{
  typedef struct
  {
    char *s;
    void (*f)(BIARGS);
  } Builtin;
  static const Builtin builtins[] = {
      BIENTRY(exit),
      BIENTRY(pwd),
      BIENTRY(cd),

      // Adding history command
      BIENTRY(history),

      // A little debugging information command
      BIENTRY(debug),

      {0, 0}};
  int i;
  for (i = 0; builtins[i].s; i++)
    if (!strcmp(r->file, builtins[i].s))
    {
      builtins[i].f(r, eof, jobs);
      return 1;
    }
  return 0;
}

static char **getargs(T_words words)
{
  int n = 0;
  T_words p = words;
  while (p)
  {
    p = p->words;
    n++;
  }
  char **argv = (char **)malloc(sizeof(char *) * (n + 1));
  if (!argv)
    ERROR("malloc() failed");
  p = words;
  int i = 0;
  while (p)
  {
    argv[i++] = strdup(p->word->s);
    p = p->words;
  }
  argv[i] = 0;
  return argv;
}

extern Command newCommand(T_words words)
{
  CommandRep r = (CommandRep)malloc(sizeof(*r));
  if (!r)
    ERROR("malloc() failed");
  r->argv = getargs(words);
  r->file = r->argv[0];
  return r;
}

static void child(CommandRep r, int fg)
{
  int eof = 0;
  Jobs jobs = newJobs();
  if (builtin(r, &eof, jobs))
  {

    // This should not happen, child processes only execute on commands not builtin
    fprintf(stdout, "Child is builtin\n");

    return;
  }

  // Executing the command in a file from $HOME (Similar behavior, atleast)
  execvp(r->argv[0], r->argv);
  ERROR("execvp() failed");

  // fprintf(stdout, "Child process complete!\n");

  exit(0);
}

extern void execCommand(Command command, Pipeline pipeline, Jobs jobs,
                        int *jobbed, int *eof, int fg)
{
  CommandRep r = command;
  if (fg && builtin(r, eof, jobs))
    return;
  if (!*jobbed)
  {
    *jobbed = 1;
    addJobs(jobs, pipeline);
  }

  // Forks the process because the command is not built in but is rather built into bash itself
  int pid = fork();
  if (pid == -1)
    ERROR("fork() failed");
  if (pid == 0)
  {
    // fprintf(stdout, "Child process running!\n");
    child(r, fg);
  }
  else
  {

    int *pidProcess = &pid;

    // Parent process
    printf("Parent process (PID: %d) created child process (PID: %d)...\n", getpid(), pid);

    // Wait for the child to finish
    wait(pidProcess);

    // printf("Child process completed!\n");
  }
}

extern void freeCommand(Command command)
{
  CommandRep r = command;
  char **argv = r->argv;
  // fprintf(stdout, "%ld\n", sizeof(argv));
  while (*argv)
  {
    // What argument is it freeing?
    fprintf(stdout, "%s\n", *argv);
    
    free(*argv++);
  }
  free(r->argv);
  free(r);

  fprintf(stdout, "Command has been freed!\n");
}

extern void freestateCommand()
{
  if (currentWD)
    free(currentWD);
  if (oldWD)
    free(oldWD);
}
