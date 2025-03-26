#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "Interpreter.h"
#include "Sequence.h"
#include "Pipeline.h"
#include "Command.h"
#include "deq.h"

static Command i_command(T_command t);
static void i_pipeline(T_pipeline t, Pipeline pipeline);
static void i_sequence(T_sequence t, Sequence sequence);

static Command i_command(T_command t)
{
  if (!t)
    return 0;
  Command command = 0;
  if (t->words)
    command = newCommand(t);
  return command;
}

static void i_pipeline(T_pipeline t, Pipeline pipeline)
{
  if (!t)
    return;
  Command current = i_command(t->command);
  addPipeline(pipeline, current);
  i_pipeline(t->pipeline, pipeline);

  if (t->pipeline)
  {
    int pipefd[2];
    pipe(pipefd);
    // printf("%d %d %s\n", pipefd[0], pipefd[1], t->command->words->word->s);

    Deq processes = getprocesses(pipeline);
    Command previous = deq_tail_ith(processes, 0);

    setwritefd(current, pipefd[1]);
    setreadfd(previous, pipefd[0]);
  }
}

static void i_sequence(T_sequence t, Sequence sequence)
{
  if (!t)
    return;
  int fg = !(strcmp(t->op, "&") == 0);
  Pipeline pipeline = newPipeline(fg);
  i_pipeline(t->pipeline, pipeline);
  addSequence(sequence, pipeline);
  i_sequence(t->sequence, sequence);
}

extern void interpretTree(Tree t, int *eof, Jobs jobs)
{
  if (!t)
    return;
  Sequence sequence = newSequence();
  i_sequence(t, sequence);
  execSequence(sequence, jobs, eof);
}
