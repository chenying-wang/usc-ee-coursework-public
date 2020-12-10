/******************************************************************************
 * NAME
 *    node_util.c
 *    J. R. Goldschneider
 *    February 1994
 *    Last Revision:
 *
 * SYNOPSIS
 *    TreeNode newnode()
 *    TreeNode newchild(parentnode)
 *    TreeNode create_root()
 *
 * DESCRIPTION
 *    newnode creates a TreeNode.
 *    newchild creates a TreeNode that inherits its parentnode's properties.
 *    create_root creates the root node with depth zero.
 *
 * RETURN VALUE
 *    Both newnode and newchild return a pointer to a TreeNode. If memory
 *    cannot be allocated then NULL is returned.
 *
 * PARAMETERS
 *    parentnode is the node whose properties the new child node inherits.
 *
 * CALLS
 *
 *****************************************************************************/
#include "tsvq.h"

char *programname;
int  dim;

TreeNode *newnode()
{
  TreeNode *n;

  if(!(n = (TreeNode *) calloc(1,sizeof(TreeNode)))) {
    fprintf(stderr,"%s: %s\n",programname,NOMEMORY);
    return(NULL);
  }

  n->left_child = NULL;
  n->right_child = NULL;
  n->parent = NULL;
  n->data = NULL;
  n->count = 0;
  n->avmse = 0.0;
  n->designed = FALSE;
  n->depth = 0;
  return(n);
}


TreeNode *newchild(parentnode)
     TreeNode *parentnode;
{
  TreeNode *n;

  if(!(n = (TreeNode *) calloc(1,sizeof(TreeNode)))) {
    fprintf(stderr,"%s: %s\n",programname,NOMEMORY);
    return(NULL);
  }

  n->left_child = NULL;
  n->right_child = NULL;
  n->parent = parentnode;
  if(!(n->data = (DISTTYPE *) calloc(dim,sizeof(DISTTYPE)))) {
    fprintf(stderr,"%s: %s\n",programname,NOMEMORY);
    return(NULL);
  }
  n->count = 0;
  n->avmse = 0.0;
  n->designed = FALSE;
  n->depth = parentnode->depth + 1;
  return(n);
}

TreeNode *create_root()
{
  TreeNode *n;

  if(!(n = newnode())) {
    fprintf(stderr,"%s: %s\n",programname,NOMEMORY);
    return(NULL);
  }

  if(!(n->data = (DISTTYPE *) calloc(dim,sizeof(DISTTYPE)))) {
    fprintf(stderr,"%s: %s\n",programname,NOMEMORY);
    return(NULL);
  }

  return(n);
}


