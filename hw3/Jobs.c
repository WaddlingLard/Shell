#include "Jobs.h"
#include "deq.h"
#include "error.h"

extern Jobs newJobs()
{
  return deq_new();
}

extern void addJobs(Jobs jobs, Pipeline pipeline)
{
  deq_tail_put(jobs, pipeline);
  // fprintf(stdout, "Adding a job! Size of deq: %d\n", deq_len(jobs));
}

extern int sizeJobs(Jobs jobs)
{
  return deq_len(jobs);
}

extern void freeJobs(Jobs jobs)
{
  deq_del(jobs, freePipeline);
}

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
