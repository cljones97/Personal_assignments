/*
Filename    : queue.c
Author      : Christopher L. Jones
Course      : CSCI 380
Assignment  : Lab1, Assessing Your C Programming Skills
Description : Give us practice in the style of programming we will need to be able to do our labs proficiently
*/
/* 
 * Code for basic C skills diagnostic.
 * Developed for courses 15-213/18-213/15-513 by R. E. Bryant, 2017
 */
/*
 * This program implements a queue supporting both FIFO and LIFO
 * operations.
 *
 * It uses a singly-linked list to represent the set of queue elements
 */
#include <stdio.h>
#include <stdlib.h>
#include "harness.h"
#include "queue.h"

/*
  Create empty queue.
  Return NULL if could not allocate space.
*/
queue_t *
q_new ()
{
  queue_t *q = malloc (sizeof (queue_t));
  /* What if malloc returned NULL? */
  if (q == NULL)
  {
    return NULL;
  }
  
  q -> head = NULL;
  q -> tail = NULL;
  q -> size = 0;
  return q;
}

/* Free all storage used by queue */
void
q_free (queue_t *q)
{
  
  /* How about freeing the list elements? */
  /* Check if head is NULL, if so the list is empty and can just return. */
  
  if (q == NULL)
  {
    return;
  }
  list_ele_t *currentNode = q -> head;
  /* Free queue structure */
  /* After freeing, q->head should just point to the rest of the list */
  /* 
    Save q->head in our current node and make nextNode point to next node after the head. 
    Free currentNode before moving to the next node.
    Set our current node to the next node in order to move down the list.
  */
  while (currentNode != NULL)
  {
    list_ele_t *nextNode = currentNode -> next;
    free (currentNode);
    currentNode = nextNode;
  }
  
  free (q);
}

/*
  Attempt to insert element at head of queue.
  Return true if successful.
  Return false if q is NULL or could not allocate space.
 */
bool
q_insert_head (queue_t *q, int v)
{
  list_ele_t *newh;
  /* What should you do if the q is NULL? */
  if (q == NULL)
    return false;
  if (q)
  {
    newh = malloc (sizeof (list_ele_t));
    /* Return false if no space was allocated */
    if ((newh))
    {
      /* 
        1) Here to start, the element after newh (newh->next) stores the information previously stored in q->head.
        2) Next, q->head now stores our value of newh therefore inserting at the queue and moving
        the previous head to the next allocated space in the queue.
        3) Increment size to make room for the new element of course.
      */
      newh -> value = v;
      newh -> next = q -> head;
      q -> head = newh;
      if (!q -> size)
      {
        /* When inserting in a queue with no size the head and tail should point to the same place */
        q -> tail = newh;
      }
      ++q -> size;
      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    return false;
  }
}

/*
  Attempt to insert element at tail of queue.
  Return true if successful.
  Return false if q is NULL or could not allocate space.
 */
bool
q_insert_tail (queue_t *q, int v)
{
  /* You need to write the complete code for this function */
  /* Remember: It should operate in O(1) time */
  list_ele_t *newh;
  if (q)
  {
    newh = malloc (sizeof (list_ele_t));
    if ((newh))
    {
      /* An element inserted at the tail of a queue shouldn't have an element after it */
      newh -> value = v;
      newh -> next = NULL;
      if (q -> size)
      {
        /*
          The original tail's next element should now be our element since we are inserting at the tail,
          After we do this the new tail has to be the element we just inserted.
         */
        q -> tail -> next = newh;
        q -> tail = newh;
        ++q -> size;
      }
      /* When inserting in a queue with no size the head, tail and new element are all the same */
      else
      {
        q -> head = q -> tail = newh;
        ++q -> size;
      }
      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    return false;
  }
}

/*
  Attempt to remove element from head of queue.
  Return true if successful.
  Return false if queue is NULL or empty.
  If vp non-NULL and element removed, store removed value at *vp.
  Any unused storage should be freed
*/
bool
q_remove_head (queue_t *q, int *vp)
{
  /* You need to fix up this code. */
  /* (Maybe Unnecessary) you can't remove the head of an empty queue. */
  if (!q || !q -> size)
  {
    return false;
  }
  else
  {
    if (vp)
    {
      *vp = q -> head -> value;
    }
    /* 
      1) currentNode holds the original head.
      2) q->head is then told to store the values in q->head->next (the next element).
      3) then we free currentNode (the holder of the original head).
      4) and we decrement the size and return true for removing the head.
    */
    list_ele_t *currentNode = q -> head;
    q -> head = q -> head -> next;
    free (currentNode);
    --q -> size;
    return true;
  }
}

/*
  Return number of elements in queue.
  Return 0 if q is NULL or empty
 */
int
q_size (queue_t *q)
{
  /* You need to write the code for this function */
  /* Remember: It should operate in O(1) time */
  /* No size to return if q is Null */
  if (q != NULL)
  {
    return q -> size; 
  }
  return 0;
}

/*
  Reverse elements in queue.
  Your implementation must not allocate or free any elements (e.g., by
  calling q_insert_head or q_remove_head).  Instead, it should modify
  the pointers in the existing data structure.
 */
void
q_reverse (queue_t *q)
{
  /* You need to write the code for this function */
  list_ele_t *currentNode = NULL;
  list_ele_t *prevNode = NULL;
  if (q == NULL)
    return;
  /* Stores the values from the head of the queue into the tail of the queue. */
  // I imagined this to act like a linked list.
  currentNode = q -> head;
  q -> tail = currentNode;
  while (currentNode != NULL)
  {
    list_ele_t *tempNode = currentNode -> next;
    currentNode -> next = prevNode;
    prevNode = currentNode;
    currentNode = tempNode;
  }
  q -> head = prevNode;
}
