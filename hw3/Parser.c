#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Parser.h"
#include "Tree.h"
#include "Scanner.h"
#include "error.h"

static Scanner scan;

#undef ERROR
#define ERROR(s) ERRORLOC(__FILE__, __LINE__, "error", "%s (pos: %d)", s, posScanner(scan))

static char *next() { return nextScanner(scan); }
static char *curr() { return currScanner(scan); }
static int cmp(char *s) { return cmpScanner(scan, s); }
static int eat(char *s) { return eatScanner(scan, s); }

static T_word p_word();
static T_words p_words();
static T_command p_command();
static T_pipeline p_pipeline();
static T_sequence p_sequence();
// static T_redir p_redir();

/*
static T_redir p_redir()
{

  // Creating redir to be returned
  T_redir redir = new_redir();

  // What is the first character?
  // Ex: "", <, >
  char *dir = curr();
  if (!dir || !(cmp("<") && cmp(">")))
  {
    // There is nothing for the redirection to do, just create an empty char redir
    // redir->dir1 = strdup("");

    // ? Possibly NULL other elements?

    // return redir;

    // Could just be returning 0
    return 0;
  }

  // Creating the new redir struct and saving the redirection
  redir->dir1 = strdup(dir);

  // Next token ([<, >] ==> word)
  next();

  // Creating the word section
  T_word word1 = p_word();

  // Unsuccessful call?
  if (word1->s == (char *)(void *)0)
  {
    // There is no word.
    // ? This should not be possible? (not syntactically valid)

    // ? Maybe we just NULL whole element?
    // redir = NULL;
    // return redir;

    // Could just be returning 0
    return 0;
  }
  */

// Word exists (Yay!)
redir->word1 = word1;

// Possible next section? ([<, > ==> word] ==>? [<, > ==> words])
next();
if (!dir || !(cmp("<") && cmp(">")))
{
  // There is no additional redirection
  // NULL out the last elements
  redir->dir2 = NULL;
  redir->word2 = NULL;
  return redir;
}

T_word word2 = p_word();

// Unsuccessful call?
if (word2->s == (char *)(void *)0)
{
  // There is no word.
  // ? This should not be possible? (not syntactically valid)

  // ? Maybe we just NULL whole element?
  // redir = NULL;
  // return redir;

  // Could just be returning 0
  return 0;
}

// Presumably, the redir is valid and will be sent back
return redir;
}

static T_word p_word()
{
  char *s = curr();
  if (!s)
    return 0;
  T_word word = new_word();
  word->s = strdup(s);
  next();
  return word;
}

static T_words p_words()
{
  T_word word = p_word();
  if (!word)
    return 0;
  T_words words = new_words();
  words->word = word;
  if (cmp("|") || cmp("&") || cmp(";"))
    return words;
  words->words = p_words();
  return words;
}

static T_command p_command()
{
  T_words words = 0;
  words = p_words();
  if (!words)
    return 0;

  // Implementing redir to comand
  // T_redir redir = 0;
  // redir = p_redir();
  // if (!redir)
  //   return 0;

  // Now creating the command, (All elements are there!)
  T_command command = new_command();
  command->words = words;
  // command->redir = redir;

  return command;
}

static T_pipeline p_pipeline()
{
  T_command command = p_command();
  if (!command)
    return 0;
  T_pipeline pipeline = new_pipeline();
  pipeline->command = command;
  if (eat("|"))
    pipeline->pipeline = p_pipeline();
  return pipeline;
}

static T_sequence p_sequence()
{
  T_pipeline pipeline = p_pipeline();
  if (!pipeline)
    return 0;
  T_sequence sequence = new_sequence();
  sequence->pipeline = pipeline;
  if (eat("&"))
  {
    sequence->op = "&";
    sequence->sequence = p_sequence();
  }
  if (eat(";"))
  {
    sequence->op = ";";
    sequence->sequence = p_sequence();
  }
  return sequence;
}

extern Tree parseTree(char *s)
{
  scan = newScanner(s);
  Tree tree = p_sequence();
  if (curr())
    ERROR("extra characters at end of input");
  freeScanner(scan);
  return tree;
}

static void f_word(T_word t);
static void f_words(T_words t);
static void f_command(T_command t);
static void f_pipeline(T_pipeline t);
static void f_sequence(T_sequence t);
// static void f_redir(T_redir t);

static void f_word(T_word t)
{
  if (!t)
    return;
  if (t->s)
    free(t->s);
  free(t);
}

static void f_words(T_words t)
{
  if (!t)
    return;
  f_word(t->word);
  f_words(t->words);
  free(t);
}

static void f_command(T_command t)
{
  if (!t)
    return;
  f_words(t->words);

  // Need to add the redirection part of the command
  // f_redir(t->redir);
  free(t);
}

static void f_pipeline(T_pipeline t)
{
  if (!t)
    return;
  f_command(t->command);
  f_pipeline(t->pipeline);
  free(t);
}

static void f_sequence(T_sequence t)
{
  if (!t)
    return;
  f_pipeline(t->pipeline);
  f_sequence(t->sequence);
  free(t);
}

/*

static void f_redir(T_redir t)
{
  // NEED TO CHECK AND MAKE SURE IT WORKS
  if (!t)
    return;
  f_word(t->word1);
  if (t->word2 != (void *)0)
  {
    f_word(t->word2);
  }
  free(t);
}

*/

extern void freeTree(Tree t)
{
  f_sequence(t);
}
