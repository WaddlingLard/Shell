#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

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
  char *in;
  char *out;
} *CommandRep;

#define BIARGS CommandRep r, int *eof, Jobs jobs
#define BINAME(name) bi_##name
#define BIDEFN(name) static void BINAME(name)(BIARGS)
#define BIENTRY(name) {#name, BINAME(name)}

static char *oldWD = 0;
static char *currentWD = 0;

static char *parentDir = "..";
static char *grandparentDir = "../..";

static void
builtin_args(CommandRep r, int n)
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

  // Need to wait for any jobs running in the background
}

BIDEFN(pwd)
{
  builtin_args(r, 0);
  if (!currentWD)
    currentWD = getcwd(0, 0);
  printf("%s\n", currentWD);
}

// A string truncating method that snips the filepath
// ? Has not been tested on root
void truncatefilepath()
{
  // Is there a current working directory?
  if (!currentWD)
  {
    // Need to get it
    currentWD = getcwd(0, 0);
  }

  // Look for the last file
  int slash = '/';
  char *lastSlash = strrchr(currentWD, slash);

  // Truncate the filepath
  lastSlash[0] = '\0';
}

// A string appending method that adds another file to the filepath
void appendfilepath()
{

  // Is there a current working directory?
  if (!currentWD)
  {
    // Need to grab it
    currentWD = getcwd(0, 0);
  }

  char readyWD[strlen(currentWD) + 1];

  strncpy(readyWD, currentWD, strlen(currentWD));
  readyWD[strlen(currentWD)] = '/';
  readyWD[strlen(currentWD) + 1] = '\0';

  if (oldWD)
    free(oldWD);
  // Cleared old directory reset with the 'new' one
  oldWD = strdup(currentWD);

  free(currentWD);

  // Appending file onto the current path
  currentWD = strncat(strdup(readyWD), r->argv[1], strlen(readyWD) + strlen(r->argv[1]));
}

BIDEFN(cd)
{
  builtin_args(r, 1);

  if (strcmp(r->argv[1], "-") == 0)
  {
    // Switch to old working directory

    // Is there a previous working directory?
    if (!oldWD)
    {
      ERROR("There is no previous working directory!\n");
    }

    // Swap current working directory and the old working directory
    char *twd = currentWD;
    currentWD = oldWD;
    oldWD = twd;
  }
  // Checking if there needs to be any shifting upwards
  else if (strncmp(r->argv[1], "..", (size_t)strlen(parentDir)) != 0 && strncmp(r->argv[1], "../..", strlen(grandparentDir)) != 0)
  {
    // Need to append file to the currentWD
    appendfilepath();
  }
  else if (strncmp(r->argv[1], "../..", strlen(grandparentDir)) == 0)
  {
  }
  else
  {
    // Need to go back to the grandparent directory
    truncatefilepath();
    truncatefilepath();
  }

  // Could be a bug, chdir returns -1 on error
  if (currentWD && chdir(currentWD) == -1)
  {

    // What is the chdir error?
    fprintf(stdout, "Name: %d\n", errno);
    fprintf(stdout, "Error: %s\n", strerror(errno));

    ERROR("chdir() failed"); // warn
  }
}

// History Command
BIDEFN(history)
{
  builtin_args(r, 0);

  // Open history file
  FILE *historyFile = fopen(".history", "r");
  if (!historyFile)
  {
    // History file does not exist
    ERROR("History file does not exist!\n");
  }

  // Read each line from the file
  char *line = NULL;
  size_t len = 0;
  int lineNumber = 1;

  fprintf(stdout, "History of commands: \n");
  while (getline(&line, &len, historyFile) != -1)
  {
    // Read the line
    fprintf(stdout, "Line %d: %s", lineNumber++, line);
  }

  // Close the file and free the line
  fclose(historyFile);
  free(line);

  // fprintf(stdout, "History!\n");
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

  // fprintf(stdout, "Getting args\n");

  int n = 0;
  T_words p = words;
  while (p)
  {
    p = p->words;
    n++;
  }
  char **argv = (char **)malloc(sizeof(char *) * (n + 2));
  memset(argv, 0, sizeof(char *) * (n + 2));
  if (!argv)
    ERROR("malloc() failed");
  p = words;
  int i = 0;
  while (p)
  {
    argv[i++] = strdup(p->word->s);

    // fprintf(stdout, "Arg %d: %s\n", i - 1, argv[i - 1]);

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

  // char **test = r->argv;

  // Outputting the command
  // fprintf(stdout, "New command: argv, %s and file, %s\n", while (!test) { strcat()}, r->file);

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
    // printf("Parent process (PID: %d) created child process (PID: %d)...\n", getpid(), pid);

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
    // fprintf(stdout, "%s\n", *argv);

    free(*argv++);
  }

  free(r->argv);
  free(r);

  // fprintf(stdout, "Command has been freed!\n");
}

extern void freestateCommand()
{
  if (currentWD)
    free(currentWD);
  if (oldWD)
    free(oldWD);
}
