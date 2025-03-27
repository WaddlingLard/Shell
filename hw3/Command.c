#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

// Used for redirects
#include <fcntl.h>

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
  int pid;
  int fd[2];
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
  jobswait(jobs);
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
void appendfilepath(CommandRep r, char *argv, int len)
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
  currentWD = strncat(strdup(readyWD), argv, len);
}

BIDEFN(cd)
{
  builtin_args(r, 1);

  // Root directory ([size_t]1 to represent the size of "/")
  if (strlen(r->argv[1]) == 1 && !(strncmp(r->argv[1], "/", (size_t)1)))
  {

    if (currentWD)
    {
      // Need to free currentWD
      free(currentWD);
    }

    currentWD = strdup("/");
  }

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
    appendfilepath(r, r->argv[1], strlen(r->argv[1]));
  }
  else if (strncmp(r->argv[1], "../..", strlen(grandparentDir)) == 0)
  {
    // Need to go back to the grandparent directory
    truncatefilepath();
    truncatefilepath();

    // How long is the filepath?
    int len = strlen(r->argv[1]) - (strlen(grandparentDir) + 1);

    // Account for possible additional '/' with the +1
    if (strlen(r->argv[1]) > strlen(grandparentDir) + 1)
    {
      // There is stuff to append from

      // Grab the substring (+1 for terminator)
      char subfilepath[len + 1];

      // Get the substring (+1 is to account for the additional '/')
      strncpy(subfilepath, r->argv[1] + strlen(grandparentDir) + 1, len);
      subfilepath[len] = '\0';

      // Append
      appendfilepath(r, subfilepath, strlen(subfilepath));
    }
  }
  else if (strncmp(r->argv[1], "..", (size_t)strlen(parentDir)) == 0)
  {
    // Need to go back to the parent directory
    truncatefilepath();

    int len = strlen(r->argv[1]) - (strlen(parentDir) + 1);

    // Account for possible additional '/' with the +1
    if (strlen(r->argv[1]) > strlen(parentDir) + 1)
    {
      // There is stuff to append
      char subfilepath[len + 1];
      strncpy(subfilepath, r->argv[1] + strlen(parentDir) + 1, len);
      subfilepath[len] = '\0';
      appendfilepath(r, subfilepath, strlen(subfilepath));
    }
  }

  // Could be a bug, chdir returns -1 on error
  if (currentWD && chdir(currentWD) == -1)
  {
    // What is the chdir error?
    fprintf(stderr, "Name: %d\n", errno);
    fprintf(stderr, "Error: %s\n", strerror(errno));

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

// Helper command to display output, purely for debugging
void outputcommand(CommandRep r)
{
  fprintf(stdout, "New command created!\n");
  char **wordsptr = r->argv;

  fprintf(stdout, "Words: ");
  while (*wordsptr)
  {
    fprintf(stdout, "%s, ", *wordsptr++);
  }
  fprintf(stdout, "\n");

  fprintf(stdout, "File: %s\n", r->file);

  fprintf(stdout, "Input: %s\n", r->in);
  fprintf(stdout, "Output: %s\n", r->out);
}

extern Command newCommand(T_command command)
{
  CommandRep r = (CommandRep)malloc(sizeof(*r));
  if (!r)
    ERROR("malloc() failed");

  r->argv = getargs(command->words);
  r->file = r->argv[0];

  // Apply the new redir features into command
  if (command->in)
  {
    r->in = command->in;
  }
  else
  {
    r->in = NULL;
  }

  if (command->out)
  {
    r->out = command->out;
  }
  else
  {
    r->out = NULL;
  }

  // Assign fd
  r->fd[0] = STDIN_FILENO;
  r->fd[1] = STDOUT_FILENO;

  // outputcommand(r);

  // char **test = r->argv;

  // Outputting the command
  // fprintf(stdout, "New command: argv, %s and file, %s\n", while (!test) { strcat()}, r->file);

  return r;
}

static void child(CommandRep r, int fg)
{

  int inputfiledescriptor = -1, outputfiledescriptor = -1;
  // int filedescriptor = -1;

  int eof = 0;
  Jobs jobs = newJobs();
  if (builtin(r, &eof, jobs))
  {

    // This should not happen, child processes only execute on commands not builtin
    fprintf(stdout, "Child is builtin\n");

    return;
  }

  // Implement pipelines
  if (r->fd[0] != 0)
  {
    // fprintf(stdout, "Command: %s, reading from: %d\n", r->file, r->fd[0]);

    dup2(r->fd[0], 0);
    close(r->fd[0]);
  }

  if (r->fd[1] != 1)
  {
    // fprintf(stdout, "Command: %s, writing from: %d\n", r->file, r->fd[1]);

    dup2(r->fd[1], 1);
    close(r->fd[1]);
  }

  // Is there an input/output redirection?
  if (r->in)
  {
    inputfiledescriptor = open(r->in, O_RDONLY);
    // fprintf(stdout, "File open!\n");

    // fprintf(stdout, "Channel Open on %d\n", filedescriptor);

    if (inputfiledescriptor == -1)
    {
      // Error has occured when opening the input file
      // fprintf(stderr, "Name: %s\n", strerrorname_np(errno));
      fprintf(stderr, "Error: %s\n", strerror(errno));
      ERROR("Opening input failed!");
    }

    if (inputfiledescriptor != STDIN_FILENO)
    {
      // Move the file to the standard in
      if (dup2(inputfiledescriptor, STDIN_FILENO) != STDIN_FILENO)
      {
        ERROR("Input is already StandardIn!\n");
      }
      close(inputfiledescriptor);
    }
  }

  if (r->out)
  {
    outputfiledescriptor = open(r->out, O_CREAT | O_WRONLY | O_TRUNC);
    // outputfiledescriptor = creat(r->out, 00777);
    // fprintf(stdout, "Output open! %d\n", outputfiledescriptor);

    if (outputfiledescriptor == -1)
    {
      // Error has occured when opening the input file
      // fprintf(stderr, "Name: %s\n", strerrorname_np(errno));
      fprintf(stderr, "Error: %s\n", strerror(errno));
      ERROR("Opening input failed!");
    }

    if (outputfiledescriptor != STDOUT_FILENO)
    {
      // Move the file to standard out
      if (dup2(outputfiledescriptor, STDOUT_FILENO) != STDOUT_FILENO)
      {
        ERROR("Output is already StandardOut!\n");
      }
      close(outputfiledescriptor);
    }
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

  // printf("%d %d %s\n", r->fd[0], r->fd[1], r->file);

  // Forks the process because the command is not built in but is rather built into bash itself
  int pid = fork();
  if (pid == -1)
    ERROR("fork() failed");
  if (pid == 0)
  {
    // fprintf(stdout, "Child process running!\n");
    child(r, fg);
  }

  // Save process id into the rep
  r->pid = pid;

  // int *pidProcess = &pid;

  // Parent process
  // printf("Parent process (PID: %d) created child process (PID: %d)...\n", getpid(), pid);
  if (fg)
  {
    // Wait for the child to finish
    wait(pid);
    if (r->fd[0] != 0)
    {
      close(r->fd[0]);
    }

    if (r->fd[1] != 1)
    {
      close(r->fd[1]);
    }
  }

  // printf("Child process completed!\n");
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

  // Free redir
  if (r->in)
    free(r->in);
  if (r->out)
    free(r->out);

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

extern int getProcessID(Command command)
{
  CommandRep r = (CommandRep)command;
  return r->pid;
}

extern char *getname(Command command)
{
  CommandRep r = (CommandRep)command;
  return r->file;
}

extern void setreadfd(Command command, int fd)
{
  CommandRep r = (CommandRep)command;

  // fprintf(stdout, "Setting %s read file descriptor to %d previously from %d\n", r->file, fd, r->fd[0]);
  r->fd[0] = fd;
  // fprintf(stdout, "Now: %d\n", r->fd[0]);
}

extern void setwritefd(Command command, int fd)
{
  CommandRep r = (CommandRep)command;

  // fprintf(stdout, "Setting %s write file descriptor to %d previously from %d\n", r->file, fd, r->fd[1]);
  r->fd[1] = fd;
  // fprintf(stdout, "Now: %d\n", r->fd[1]);
}