#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "Pipeline.h"
#include "deq.h"
#include "error.h"

typedef struct
{
  Deq processes;
  int fg; // not "&"
  int lastprocessid;
} *PipelineRep;


/**
 * Creates a new Pipeline object.
 * @param fg Flag indicating if the pipeline should run in the foreground (not "&").
 * @return A new Pipeline object.
 */
extern Pipeline newPipeline(int fg)
{
  PipelineRep r = (PipelineRep)malloc(sizeof(*r));
  if (!r)
    ERROR("malloc() failed");
  r->processes = deq_new();
  r->fg = fg;
  return r;
}

/**
 * Adds a command to the pipeline
 * @param pipeline Pipeline for command to add to
 * @param command Command to add to the pipeline
 */
extern void addPipeline(Pipeline pipeline, Command command)
{
  PipelineRep r = (PipelineRep)pipeline;
  deq_tail_put(r->processes, command);
}

/**
 * Gets size of pipeline
 * @param pipeline The pipeline to get size of
 * @return The number of processes in the pipeline
 */
extern int sizePipeline(Pipeline pipeline)
{
  PipelineRep r = (PipelineRep)pipeline;
  return deq_len(r->processes);
}

/**
 * Executes the pipeline
 * @param pipeline The pipeline containing the commands
 * @param jobs The list of jobs
 * @param jobbed Pointer indicating whether a job was started
 * @param eof Pointer tracking the end-of-file status
 */
static void execute(Pipeline pipeline, Jobs jobs, int *jobbed, int *eof)
{

  PipelineRep r = (PipelineRep)pipeline;
  Command last = deq_tail_ith(r->processes, 0);
  // fprintf(stdout, "Executing pipeline... , number of jobs: %d\n", sizePipeline(r));

  for (int i = 0; i < sizePipeline(r) && !*eof; i++)
    execCommand(deq_head_ith(r->processes, i), pipeline, jobs, jobbed, eof, r->fg);

  r->lastprocessid = getProcessID(last);
  if (r->fg)
  {
    wait(r->lastprocessid);
  }
}

/**
 * Executes the pipeline
 * @param pipeline The pipeline to execute
 * @param jobs The list of jobs
 * @param eof Pointer tracking the end-of-file status.
 */
extern void execPipeline(Pipeline pipeline, Jobs jobs, int *eof)
{
  int jobbed = 0;
  execute(pipeline, jobs, &jobbed, eof);
  if (!jobbed)
    freePipeline(pipeline); // for fg builtins, and such
}

/**
 * Frees the memory allocated for the pipeline and its commands
 * @param pipeline The pipeline to free
 */
extern void freePipeline(Pipeline pipeline)
{
  PipelineRep r = (PipelineRep)pipeline;
  deq_del(r->processes, freeCommand);
  free(r);
}

/**
 * Gets the process ID of the last command in the pipeline
 * @param pipeline The pipeline
 * @return The process ID of the last command
 */
extern int getlastprocess(Pipeline pipeline)
{
  PipelineRep r = (PipelineRep)pipeline;
  return r->lastprocessid;
}

/**
 * Gets the deq of processes in the pipeline.
 * @param pipeline The pipeline 
 * @return The deq containing the processes in the pipeline
 */
extern Deq getprocesses(Pipeline pipeline)
{
  PipelineRep r = (PipelineRep)pipeline;
  return r->processes;
}
