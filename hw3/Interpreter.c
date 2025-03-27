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

static int locationofjob = 0;

/**
 * Converts a T_command structure to a command object.
 * @param t The T_command to convert.
 * @return The corresponding command object, 0 if command (t) is null
 */
static Command i_command(T_command t)
{
  if (!t)
    return 0;
  Command command = 0;
  if (t->words)
    command = newCommand(t);
  return command;
}

/**
 * Recursively processes a pipeline, setting up commands and pipes between them.
 * @param t The pipeline to process.
 * @param pipeline The pipeline object to add the commands to.
 */
static void i_pipeline(T_pipeline t, Pipeline pipeline)
{
  if (!t)
    return;
  Command current = i_command(t->command);
  addPipeline(pipeline, current);
  i_pipeline(t->pipeline, pipeline);

  if (t->pipeline)
  {

    // fprintf(stdout, "Location: %d\n", locationofjob);

    int pipefd[2];
    pipe(pipefd);
    // printf("%d %d %s\n", pipefd[0], pipefd[1], t->command->words->word->s);

    Deq processes = getprocesses(pipeline);

    Command previous = deq_tail_ith(processes, locationofjob);
    // fprintf(stdout, "Current size of processes %d\n", deq_len(processes));

    // Command previous = deq_tail_ith(processes, 1);

    // fprintf(stdout, "The current command is: %s\n", t->command->words->word->s);
    // fprintf(stdout, "The previous command is: %s\n", getname(previous));

    setreadfd(previous, pipefd[0]);
    setwritefd(current, pipefd[1]);

    locationofjob++;
  }
  else
  {
    // printf("%s\n", t->command->words->word->s);
    locationofjob = 0;
  }
}

/**
 * Processes a sequence of commands
 * @param t The sequence of commands 
 * @param sequence The sequence object to add the pipelines 
 */
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

/**
 * Interprets the tree of commands and executes it
 * @param t The tree structure
 * @param eof Tracks the end of the file status
 * @param jobs The list of jobs
 */
extern void interpretTree(Tree t, int *eof, Jobs jobs)
{
  if (!t)
    return;
  Sequence sequence = newSequence();
  i_sequence(t, sequence);
  execSequence(sequence, jobs, eof);
}
