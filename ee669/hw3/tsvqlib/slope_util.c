/******************************************************************************
 * Name
 *    slope_util.c
 *    J. R. Goldschneider some of this is based on code from the ftp site
 *        at Stanford. I do not know who is responsible for it.
 *    February 1994
 *    Last Revision
 *        December 1994, add a test to slope() to make sure that a node
 *        cannot be added to the tree unless it has a minimum number of
 *        nodes.  This tries to ensure that each codeword has at least
 *        DEF_min_vectors training vectors per codeword.
 *
 * SYNOPSIS
 *    void      update_rate(maxslopenode,bits,distortion)
 *    DISTTYPE  slope(node)
 *    BOOLEAN   conditional_insert(node)
 *    BOOLEAN   forced_insert(node)
 *    BOOLEAN   initialize_slopelist()
 *    void      delete_slopelist(list_element)
 *    SlopeList *find_maxslope()
 *    SlopeList *find_oldest_entry()
 *
 * DESCRIPTION
 *    update_rate recomputes bits and distortion given that the node pointed
 *    to by maxslopenode is being replaced by its two children.
 *
 *    slope computes the change in distortion one would achieve if the node
 *    was replaced by its two children.
 *
 *    conditional_insert creates a SlopeList element pointing to the node and
 *    inserts it into slist only if the slope is non-zero. This should be
 *    used for unbalanced trees.
 *
 *    forced_insert creates a SlopeList element pointing to the node and
 *    inserts it into slist regardless of the slope.  This should be used
 *    for balanced trees.
 *
 *    initialize_slopelist creates the slist with a dummy element that
 *    heads slist and will never be selected.
 *
 *    delete_slopelist removes the list_element from the slist.
 *
 *    find_maxslope searches slist to find the node with the largest slope.
 *    This should be used for an unbalanced tree.
 *
 *    find_oldest_entry searches slist to find the oldest node.  This should
 *    be used for a balanced tree.
 *
 * RETURN VALUE
 *    update_rate returns no value, but bits and distortion are modified.
 *
 *    slope returns the change in distortion due to replacing node with
 *    its children.
 *
 *    conditiional_insert returns TRUE if successful, FALSE if not successful.
 *
 *    forced_insert returns TRUE if successful, FALSE if not successful.
 *
 *    initialize_slopelist returns TRUE if successful, FALSE if not successful.
 *
 *    delete_slopelist returns no value.
 *
 *    find_maxslope returns the list element pointing to the node with
 *    the maximum slope.
 *
 *    find_oldest_entry returns the list element pointing to the node
 *    that was entered before all the others.
 *
 * PARAMETERS
 *    maxslopenode is pointer to the node being replaced.
 *    bits is the total number of bits used to encode the training sequence.
 *    distortion is the result from encoding the training sequence.
 *
 *    node is the node that may be replaced be its children.
 *
 *    node is the node to which the SlopeList element should point.
 *
 *    list_element is the element to be removed from slist.
 *
 * CALLS
 *
 *****************************************************************************/
#include "tsvq.h"

char      *programname;
SlopeList *slist;

void update_rate(maxslopenode,bits,distortion)
     SlopeList *maxslopenode;
     long      *bits;
     DISTTYPE  *distortion;
{
  /* the distortion is the same as the old distortion less the distortion
     due to the node being replaced plus the distortion due to the
     two nodes just added */
  *distortion +=  maxslopenode->node->left_child->avmse *
    ((DISTTYPE) maxslopenode->node->left_child->count) +
      maxslopenode->node->right_child->avmse *
	((DISTTYPE) maxslopenode->node->right_child->count) -
	  maxslopenode->node->avmse * ((DISTTYPE) maxslopenode->node->count);

  /* when the very last nodes are added, the distortion is -0.0 */
  if (*distortion <= 0) *distortion = 0;

  /* the number of bits is the same as the old number plus count since
     this is just a replacement of one node by its two children */
  *bits += maxslopenode->node->count;
}

DISTTYPE slope(node)
     TreeNode *node;
{
  DISTTYPE distnode;
  DISTTYPE probleft = 0.0;
  DISTTYPE probright = 0.0;

  if(node->count < DEF_min_vectors) return( (DISTTYPE) 0.0 );

  probleft = ((DISTTYPE) (node->left_child->count)) /
    ((DISTTYPE) node->count);
  probright = ((DISTTYPE) (node->right_child->count)) /
    ((DISTTYPE) node->count);

  distnode = (node->avmse) -
    (probleft * (node->left_child->avmse)) -
      (probright * (node->right_child->avmse));

  /* slope is (change in distortion)/(change in rate) and (change in rate)
     is equal to count, so it is not necessary to multiply distnode by
     count */
  return(distnode);
}

BOOLEAN conditional_insert(node)
     TreeNode *node;

{
  if (slope(node) > 0) {
    return(forced_insert(node));
  }
  else {
    return(TRUE);
  }
}

BOOLEAN forced_insert(node)
     TreeNode *node;
{
  SlopeList *tempnode;

  if(!(tempnode = (SlopeList *) calloc(1,sizeof(SlopeList)))) {
    fprintf(stderr,"%s: %s\n",programname,NOMEMORY);
    return(FALSE);
  }

  /* insert the newest node at the beginning of the list */
  slist->next->previous = tempnode;
  tempnode->next=slist->next;
  slist->next = tempnode;
  tempnode->previous = slist;
  tempnode->slope = slope(node);
  tempnode->node = node;
  return(TRUE);
}

BOOLEAN initialize_slopelist()
{
  if(!(slist = (SlopeList *) calloc(1,sizeof(SlopeList)))) {
    fprintf(stderr,"%s: %s\n",programname,NOMEMORY);
    return(FALSE);
  }

  /* the slope of 0.0 ensures that there is always one entry in the list
     and that it will never be chosen */
  slist->previous = slist;
  slist->next = slist;
  slist->slope = 0.0;
  slist->node = NULL;
  return(TRUE);
}


void delete_slopelist(list_element)
     SlopeList *list_element;
{
  list_element->previous->next = list_element->next;
  list_element->next->previous = list_element->previous;
  free((char *) list_element);
}

SlopeList *find_maxslope()
{
  SlopeList *i;
  SlopeList *currentbest;
  DISTTYPE max;

  if(!(i = (SlopeList *) calloc(1,sizeof(SlopeList))) ||
     !(currentbest = (SlopeList *) calloc(1,sizeof(SlopeList)))) {
    fprintf(stderr,"%s: %s\n",programname,NOMEMORY);
    return(NULL);
  }

  /* the first node is a dummy node */
  max = 0.0;
  currentbest = slist;

  /* search the list for the largest slope */
  for (i = slist->next; i != slist; i = i->next) {
    if (i->slope > max)	{
      max = i->slope;
      currentbest = i;
    }
  }

  return(currentbest);
}

SlopeList *find_oldest_entry()
{
  /* this should be used for a balanced tree, return the oldest entry */
  return(slist->previous);
}
