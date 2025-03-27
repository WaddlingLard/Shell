#include "Sequence.h"
#include "deq.h"
#include "error.h"

// Create a new sequence
extern Sequence newSequence() {
  return deq_new();
}

// Add a pipeline to the sequence
extern void addSequence(Sequence sequence, Pipeline pipeline) {
  deq_tail_put(sequence,pipeline);
}

// Free a sequence
extern void freeSequence(Sequence sequence) {
  deq_del(sequence,freePipeline);
}

// Execute a sequence of pipelines
extern void execSequence(Sequence sequence, Jobs jobs, int *eof) {
  while (deq_len(sequence) && !*eof)
    execPipeline(deq_head_get(sequence),jobs,eof);
  freeSequence(sequence);
}
