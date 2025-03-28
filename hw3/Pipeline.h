#ifndef PIPELINE_H
#define PIPELINE_H

typedef void *Pipeline;

#include "Command.h"
#include "Jobs.h"
#include "deq.h"

extern Pipeline newPipeline(int fg);
extern void addPipeline(Pipeline pipeline, Command command);
extern int sizePipeline(Pipeline pipeline);
extern void execPipeline(Pipeline pipeline, Jobs jobs, int *eof);
extern void freePipeline(Pipeline pipeline);

extern int getlastprocess(Pipeline pipeline);
extern Deq getprocesses(Pipeline pipeline);

#endif
