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

// Unsure what the o is in wd
static char *owd = 0;
static char *currentWD = 0;

static void builtin_args(CommandRep r, int n)
{
  char **argv = r->argv;
  for (n++; *argv++; n--)
  {
  };
  if (n)
    ERROR("wrong number of arguments to builtin command"); // warn
}

BIDEFN(exit)
{
  printf("exit!");
  builtin_args(r, 0);
  *eof = 1;
}

BIDEFN(pwd)
{
  printf("pwd!");
  builtin_args(r, 0);
  if (!currentWD)
    currentWD = getcwd(0, 0);
  printf("%s\n", currentWD);
}

BIDEFN(cd)
{
  printf("cd!");
  builtin_args(r, 1);
  if (strcmp(r->argv[1], "-") == 0)
  {
    char *twd = currentWD;
    currentWD = owd;
    owd = twd;
  }
  else
  {
    if (owd)
      free(owd);
    owd = currentWD;
    currentWD = strdup(r->argv[1]);
  }
  if (currentWD && chdir(currentWD))
    ERROR("chdir() failed"); // warn
}

// History command
BIDEFN(history)
{
  builtin_args(r, 0);

  fprintf(stdout, "History!\n");
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
    // Should not be possible, child processes only execute on commands not builtin
    fprintf(stderr, "CHild is builtin\n");

    // ? Maybe?
    exit(-1);

    // return;
  }
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

  // Forks the process because the command is not build int but is rather built into bash itself
  int pid = fork();
  if (pid == -1)
    ERROR("fork() failed");
  if (pid == 0)
  {
    // Child process
    fprintf(stdout, "Child process is running!\n");
    child(r, fg);
  }
  else
  {
    // Parent process
    // fprintf(stdout, "Parent (PID: %d) creating child (PID: %d)...\n", getpid(), pid);

    // Wait for the child to finish
    wait(pid);

    // fprintf(stdout, "Child process completed!\n");
  }
}

extern void freeCommand(Command command)
{
  CommandRep r = command;
  char **argv = r->argv;
  while (*argv)
    free(*argv++);
  free(r->argv);
  free(r);
}

extern void freestateCommand()
{
  if (currentWD)
    free(currentWD);
  if (owd)
    free(owd);
}
