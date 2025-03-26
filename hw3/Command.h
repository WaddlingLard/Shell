#ifndef COMMAND_H
#define COMMAND_H

typedef void *Command;

#include "Tree.h"
#include "Jobs.h"
#include "Sequence.h"

// extern Command newCommand(T_words words);
extern Command newCommand(T_command command);

extern void execCommand(Command command, Pipeline pipeline, Jobs jobs,
						int *jobbed, int *eof, int fg);

extern void freeCommand(Command command);
extern void freestateCommand();

extern int getProcessID(Command command);
extern char *getname(Command command);
extern void setreadfd(Command command, int fd);
extern void setwritefd(Command command, int fd);

#endif
