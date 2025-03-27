#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <termios.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "Jobs.h"
#include "Parser.h"
#include "Interpreter.h"
#include "error.h"

int main()
{
  int eof = 0;
  Jobs jobs = newJobs();
  char *prompt = 0;

  // Checking if stdin is open ready to take user input for the shell
  if (isatty(fileno(stdin)))
  {
    using_history();
    read_history(".history");
    prompt = "$ ";
  }
  else
  {
    rl_bind_key('\t', rl_insert);
    rl_outstream = fopen("/dev/null", "w");
  }

  // This is the main loop that runs the shell, when eof=1 is when the execution stops
  while (!eof)
  {
    char *line = readline(prompt);
    // fprintf(stdout, "%s\n", line);
    if (!line)
      break;
    if (*line)
      add_history(line);
    Tree tree = parseTree(line);
    free(line);
    interpretTree(tree, &eof, jobs);
    freeTree(tree);
  }

  // As the file channel for stdin closes, history will be written to the .history file
  if (isatty(fileno(stdin)))
  {
    write_history(".history");
    rl_clear_history();
  }
  else
  {
    fclose(rl_outstream);
  }

  // Freeing the command and jobs1
  freestateCommand();
  freeJobs(jobs);
  return 0;
}
