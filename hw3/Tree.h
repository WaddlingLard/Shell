#ifndef TREE_H
#define TREE_H

// typedef struct T_redir *T_redir;
typedef struct T_sequence *T_sequence;
typedef struct T_pipeline *T_pipeline;
typedef struct T_command *T_command;
typedef struct T_words *T_words;
typedef struct T_word *T_word;

struct T_sequence
{
  T_pipeline pipeline;
  char *op; /* ; or & */
  T_sequence sequence;
};

struct T_pipeline
{
  T_command command;
  char *pipe;
  T_pipeline pipeline;
};

struct T_command
{
  T_words words;
  char *in;
  char *out;
};

struct T_words
{
  T_word word;
  T_words words;
};

struct T_word
{
  char *s;
};

/*
struct T_redir
{
  char *dir1; // < or > or even ""
  T_word word1;
  char *dir2; // < or > or even ""
  T_word word2;

  // T_redir redir;
};
*/

// extern T_redir new_redir();
extern T_sequence new_sequence();
extern T_pipeline new_pipeline();
extern T_command new_command();
extern T_words new_words();
extern T_word new_word();

#endif
