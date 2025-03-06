#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "deq.h"
#include "error.h"

// indices and size of array of node pointers
typedef enum {
  Head,
  Tail,
  Ends,

  Next = Head, // 0
  Prev = Tail // 1

} End;

typedef struct Node {
  struct Node *np[Ends];        // next/prev neighbors
  Data data;
} *Node;

typedef struct {
  Node ht[Ends];                // head/tail nodes
  int len;
} *Rep;

static Rep rep(Deq q) {
  if (!q) ERROR("zero pointer");
  return (Rep)q;
}

static void put(Rep r, End e, Data d) {
  
  // Validating data
  if (!d) ERROR("DATA ERROR");

  // Setting directional flag, ex: if e is the head (0) then the direction 1 for next
  int dirE = (!e) ? Prev : Next;

  // Creating new node to be stored
  Node add = (Node)malloc(sizeof(struct Node));

  // Validating new node
  if (!add) ERROR("malloc() failed");
  add->data = d;

  // e can either be Head = 0, or Tail = 1
  if (r->len != 0) {
    // Grab the node, either at the tail or head
    Node end = r->ht[e];

    // Setting the end node's neighbor to be next to the new node
    end->np[e] = add;

    // Setting the new node's neighbor to be the current end node
    add->np[dirE] = end;

    // Readjust where the end pointer is
    r->ht[e] = add;

  } else {
    // No data, head and tail will point to same node
    r->ht[Head] = add;
    r->ht[Tail] = add;
  }

  // Increment size
  r->len = r->len + 1;
}

static Data ith(Rep r, End e, int i)  { 

  // Checking to see if ith is a valid call
  if (r->len == 0 || i > r->len) ERROR("Invalid method call!");

  // Setting directional flag, ex: if e is the head (0) then the direction 1 for next
  int dirE = (!e) ? Prev : Next;

  // Grabbing the current node and data to iterate
  Node curr = r->ht[e];
  Data pulled = curr->data;
  
  // Using 0-based index and iterate over each node until 1 == index
  int index = 0;
  while (i != index++) {
    // Grab the next element and update the current data
    curr = curr->np[dirE];
    pulled = curr->data;
  }

  // Returning the current data
  return pulled;
}

static Data get(Rep r, End e)         { 

  // Invalid call, nothing in the deq
  if (r->len == 0) ERROR("no items in deq");

  // Setting directional flag, ex: if e is the head (0) then the direction 1 for next
  int dirE = (!e) ? Prev : Next;
  
  // Grabbing node and pulling data
  Node end = r->ht[e];
  Data pulled = end->data;  

  // deq is empty now
  if (r->len - 1 == 0) {
    // Setting both ends to null as there is nothing in the deq
    r->ht[e] = NULL;
    r->ht[dirE] = NULL;

    // Freeing grabbed node
    free(end);
  } else {
    // Readjusting pointers

    // Grabbing the node which is adjacent to the grabbed one
    Node adjacent = end->np[dirE];

    // The adjacent node's connection is severed with the grabbed node
    adjacent->np[e] = NULL;

    // Adjusting end pointer
    r->ht[e] = adjacent;
    
    // Freeing grabbed node
    free(end);
  }

  // Decrementing size
  r->len = r->len - 1;

  return pulled;
}

static Data rem(Rep r, End e, Data d) { 

  // Invalid call, nothing in the deq
  if (r->len == 0) ERROR("No items in deq");

  // Setting directional flag, ex: if e is the head (0) then the direction 1 for next
  int dirE = (!e) ? Prev : Next;
  
  // Grabbing node and pulling data
  Node current = r->ht[e];
  Data pulled = current->data;

  // Used to store the node that will be removed
  Node remove;
  
  // Loop through to find the matching data
  int index = 0;
  while (index != r->len) {
    // Have to use '==' comparison
    if (pulled == d) {
      remove = current;
      // printf("Found a match!");
      break;
    }

    // Refresh values to continue looking
    current = current->np[dirE];
    pulled = current->data;

    // Incrementing index
    index++;

    // After executing, the loop either found a match, or it reached the end
  }

  // Final check, fails if match not found
  if (pulled != d) ERROR("Data is not in the list");

  // deq is empty now
  if (r->len - 1 == 0) {
    // Setting both ends to null as there is nothing in the deq
    r->ht[e] = NULL;
    r->ht[dirE] = NULL;

    // Freeing grabbed node
    free(remove);
  } else if (index == 0){
    // At the front of the searching point

    // Grabbing the node which is adjacent to the grabbed one
    Node adjacent = current->np[dirE];

    // The adjacent node's connection is severed with the grabbed node
    adjacent->np[e] = NULL;

    // Adjusting end pointer
    r->ht[e] = adjacent;
    
    // Freeing grabbed node
    free(remove);
  } else if (index == r->len - 1) {
    // At the end of the searching point
    
    // Grabbing the node which is adjacent to the grabbed one
    Node adjacent = current->np[e];

    // The adjacent node's connection is severed with the grabbed node
    adjacent->np[dirE] = NULL;

    // Adjusting end pointer
    r->ht[dirE] = adjacent; 
    
    // Freeing grabbed node
    free(remove);
  } else {
    // Somewhere in the middle
    // printf("%d %d", index, r->len);
    
    // Going to need to grab 2 nodes, one ahead of the grabbed and one before
    Node adjacentForward, adjacentBackward;
    adjacentForward = current->np[dirE];
    adjacentBackward = current->np[e];

    // The node in front of the grabbed node will be attached to the previous one
    adjacentForward->np[e] = adjacentBackward;

    // The node behind of the grabbed node will be attached to the next one
    adjacentBackward->np[dirE] = adjacentForward;
    
    // Freeing grabbed node
    free(remove);
  }

  // Decrementing size
  r->len = r->len - 1;

  return pulled; 
}

extern Deq deq_new() {
  Rep r=(Rep)malloc(sizeof(*r));
  if (!r) ERROR("malloc() failed");
  r->ht[Head]=0;
  r->ht[Tail]=0;
  r->len=0;
  return r;
}

extern int deq_len(Deq q) { return rep(q)->len; }

extern void deq_head_put(Deq q, Data d) {        put(rep(q),Head,d); }
extern Data deq_head_get(Deq q)         { return get(rep(q),Head);   }
extern Data deq_head_ith(Deq q, int i)  { return ith(rep(q),Head,i); }
extern Data deq_head_rem(Deq q, Data d) { return rem(rep(q),Head,d); }

extern void deq_tail_put(Deq q, Data d) {        put(rep(q),Tail,d); }
extern Data deq_tail_get(Deq q)         { return get(rep(q),Tail);   }
extern Data deq_tail_ith(Deq q, int i)  { return ith(rep(q),Tail,i); }
extern Data deq_tail_rem(Deq q, Data d) { return rem(rep(q),Tail,d); }

extern void deq_map(Deq q, DeqMapF f) {
  for (Node n=rep(q)->ht[Head]; n; n=n->np[Tail])
    f(n->data);
}

extern void deq_del(Deq q, DeqMapF f) {
  if (f) deq_map(q,f);
  Node curr=rep(q)->ht[Head];
  while (curr) {
    Node next=curr->np[Tail];
    free(curr);
    curr=next;
  }
  free(q);
}

extern Str deq_str(Deq q, DeqStrF f) {
  char *s=strdup("");
  for (Node n=rep(q)->ht[Head]; n; n=n->np[Tail]) {
    char *d=f ? f(n->data) : n->data;
    char *t; asprintf(&t,"%s%s%s",s,(*s ? " " : ""),d);
    free(s); s=t;
    if (f) free(d);
  }
  return s;
}
