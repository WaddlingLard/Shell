#include "Jobs.h"
#include "deq.h"
#include "error.h"

/**
 * Creates a new deq to hold the jobs
 * @return deq
 */
extern Jobs newJobs()
{
  return deq_new();
}

/**
 * Adds a new pipeline job to the jobs deq
 * @param jobs The deq of jobs to add the pipeline
 * @param pipeline The pipeline to be added as a job
 */
extern void addJobs(Jobs jobs, Pipeline pipeline)
{
  deq_tail_put(jobs, pipeline);
  // fprintf(stdout, "Adding a job! Size of deq: %d\n", deq_len(jobs));
}

/**
 * Returns the number of jobs in the jobs deq
 * @param jobs The deq of the jobs
 * @return The number of jobs in the deq
 */
extern int sizeJobs(Jobs jobs)
{
  return deq_len(jobs);
}

/**
 * Frees the memory allocated for the jobs via the freepipeline function
 * @param jobs The deq of jobs to be freed.
 */
extern void freeJobs(Jobs jobs)
{
  deq_del(jobs, freePipeline);
}

/**
 * Waits for each job in the jobs deq to finish.
 * @param jobs The deq of jobs to wait on.
 */
extern void jobswait(Jobs jobs)
{
  for (int i = 0; i < sizeJobs(jobs); i++)
  {
    Pipeline p = deq_head_get(jobs);
    int lastprocess = getlastprocess(p);
    wait(lastprocess);

    freePipeline(p);
  }
}
