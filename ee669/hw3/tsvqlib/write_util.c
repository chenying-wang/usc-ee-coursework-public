/******************************************************************************
 * NAME
 *    write_util.c
 *    J. R. Goldschneider
 *    February 1994
 *    Last Revision
 *
 * SYNOPSIS
 *    BOOLEAN  write_codebook(root,numnodes,codebookfile,codebookname)
 *    BOOLEAN  write_stat(root,numnodes,statfile,statname)
 *    BOOLEAN  write_nested_subtree(root,numnodes,subtreefile,subtreename)
 *
 * DESCRIPTION
 *    write_codebook writes the codebook tree to the codebook file. The
 *    format is:
 *        TYPE       SIZE          DESCRIPTION
 *        long       1             number of nodes in the tree (numnodes)
 *        integer    1             vector dimension (dim)
 *        short      numnodes      tree description array
 *        DISTTYPE   numnodes*dim  codewords
 *    The format of the tree description array is that a 1 is a node that is
 *    not a terminal node and a 0 is a terminal node. It is a preorder list.
 *
 *    write_stat writes the counts and distortions to the stat file.
 *    The order is a preorder listing as is the codebook.  The format is
 *    two values per node.  One long is used for the count, and one double
 *    is used for the distortion.
 *
 *    write_nested_subtree writes the subtree number to the subtree file.
 *    The order is a preorder listing as is the codebook.  The format is
 *    one values per node, the subtree number is a long int.
 *
 * RETURN VALUE
 *    write_codebook returns TRUE if there are no problems writing the
 *    codebook.  Otherwise an error message is printed and FALSE is returned.
 *
 *    write_stat returns TRUE if there are no problems, otherwise FALSE is
 *    returned and an error message is printed.
 *
 *    write_nested_subtree returns TRUE if there are no problems,
 *    otherwise FALSE is returned and an error message is printed.
 *
 * PARAMETERS
 *    root is the root of the codebook tree.
 *    numnodes is the total number of nodes in the tree.
 *    codebookfile is the file in which the codebook is written.
 *    codebookname is the name of the codebookfile.
 *
 *    root is the root of the codebook tree.
 *    numnodes is the total number of nodes in the tree.
 *    statfile is the file in which the information is written.
 *    statnmae is the name of the statfile.
 *
 *    root is the root of the codebook tree.
 *    numnodes is the total number of nodes in the tree.
 *    subtreefile is the file in which the information is written.
 *    subtreenmae is the name of the nested subtree.
 *
 * CALLS
 *    newnode()
 *
 *****************************************************************************/
#include "tsvq.h"

int  dim;
char *programname;

BOOLEAN write_codebook(root,numnodes,codebookfile,codebookname)
     TreeNode *root;
     long     numnodes;
     FILE     *codebookfile;
     char     *codebookname;
{
  long      i,n;           /* counters */
  TreeNode *node;          /* node used to point to the tree */
  short    *tree_descrip;  /* array containing tree structure (in bytes) */

  /* allocate memory for tree description */
  if(!(tree_descrip = (short *) calloc(numnodes,sizeof(short)))) {
    fprintf(stderr,"%s: %s\n",programname,NOMEMORY);
    return(FALSE);
  }

  /* create a node to search the tree */
  if(!(node = newnode())) {
    fprintf(stderr,"%s: %s\n",programname,NOMEMORY);
    return(FALSE);
  }
  node = root;

  /* create the tree description array */
  n = 0; /* count the number of nodes */
  for (i = 0; i < numnodes; i++) {
    /* determine node position and from this find the next node to use */
    if (node->left_child == NULL) { /* node is terminal */
      if(node->right_child != NULL) {  /* test tree fidelity */
        fprintf(stderr,"%s: %s: %s\n",programname,codebookname,NOTREE);
        return(FALSE);
      }
      tree_descrip[n] = 0; /* enter a terminal node in tree description */
      n++; /* increment count of nodes */

      /* find next node to examine */
      while ((node != root) && (node == node->parent->right_child)) {
        node = node->parent; /* continue with the leaf node's parent */
      }
      if (node == root) break;
      else {
        node = node->parent->right_child;
      }
    }

    else { /* node has a child, go to left child */
      if(node->right_child == NULL) { /* test tree fidelity */
        fprintf(stderr,"%s: %s: %s\n",programname,codebookname,NOTREE);
        return(FALSE);
      }
      tree_descrip[n] = 1; /* enter a non-terminal node in tree description */
      n++; /* increment count of nodes */
      node = node->left_child;
    }
  }

  /* when done constructing the tree, node should be the root */
  if (node != root || n != numnodes) {
    fprintf(stderr,"%s: %s: %s\n",programname,codebookname,NOTREE);
    return(FALSE);
  }

  /* write numnodes, dim and tree_descrip */
  if ((fwrite((char *) &numnodes,sizeof(long),1,codebookfile) != 1) ||
      ferror(codebookfile) || feof(codebookfile)) {
    fprintf(stderr,"%s: %s: %s\n",programname,codebookname,NOWRITE);
    return(FALSE);
  }
  if ((fwrite((char *) &dim,sizeof(int),1,codebookfile) != 1) ||
      ferror(codebookfile) || feof(codebookfile)) {
    fprintf(stderr,"%s: %s: %s\n",programname,codebookname,NOWRITE);
    return(FALSE);
  }
  if (fwrite((char *) tree_descrip,sizeof(short), (int) numnodes,codebookfile)
      != numnodes || ferror(codebookfile) || feof(codebookfile)){
    fprintf(stderr,"%s: %s: %s\n",programname,codebookname,NOWRITE);
    return(FALSE);
  }

  /* free allocated space */
  free((char *) tree_descrip);

  /* now write the codewords */
  node = root;
  n = 0; /* count the number of nodes */
  for (i = 0; i < numnodes; i++) {

    /* write the node's data */
    if (fwrite((char *) node->data,sizeof(DISTTYPE),dim,codebookfile)!= dim ||
	ferror(codebookfile) || feof(codebookfile)){
      fprintf(stderr,"%s: %s: %s\n",programname,codebookname,NOWRITE);
      return(FALSE);
    }

    /* determine node position and from this find the next node to use */
    if (node->left_child == NULL) { /* node is terminal, find next node */
      if(node->right_child != NULL) {  /* test tree fidelity */
        fprintf(stderr,"%s: %s: %s\n",programname,codebookname,NOTREE);
        return(FALSE);
      }
      n++; /* increment count of nodes */

      /* find the next node */
      while ((node != root) && (node == node->parent->right_child)) {
        node = node->parent; /* continue with the leaf node's parent */
      }
      if (node == root) break;
      else {
        node = node->parent->right_child;
      }
    }

    else { /* node has a child, go to left child */
      if(node->right_child == NULL) { /* test tree fidelity */
        fprintf(stderr,"%s: %s: %s\n",programname,codebookname,NOTREE);
        return(FALSE);
      }
      n++; /* increment count of nodes */
      node = node->left_child;
    }
  }

  /* when done constructing the tree, node should be the root */
  if (node != root || n != numnodes) {
    fprintf(stderr,"%s: %s: %s\n",programname,codebookname,NOTREE);
    return(FALSE);
  }

  fflush(codebookfile);
  return(TRUE);
}

BOOLEAN write_stat(root,numnodes,statfile,statname)
     TreeNode *root;
     long     numnodes;
     FILE     *statfile;
     char     *statname;
{
  long      i,n;           /* counters */
  TreeNode *node;          /* node used to point to the tree */

  if(!(node = newnode())) {
    fprintf(stderr,"%s: %s\n",programname,NOMEMORY);
    return(FALSE);
  }
  node = root;

  n = 0; /* count the number of nodes */
  for (i = 0; i < numnodes; i++) {

    /* write the node's count */
    if (fwrite((char *) &(node->count),sizeof(long),1,statfile)!= 1 ||
	ferror(statfile) || feof(statfile)){
      fprintf(stderr,"%s: %s: %s\n",programname,statname,NOWRITE);
      return(FALSE);
    }
    if (fwrite((char *) &(node->avmse),sizeof(double),1,statfile)!= 1 ||
	ferror(statfile) || feof(statfile)){
      fprintf(stderr,"%s: %s: %s\n",programname,statname,NOWRITE);
      return(FALSE);
    }

    /* determine node position and from this find the next node to use */
    if (node->left_child == NULL) { /* node is terminal, find next node */
      if(node->right_child != NULL) {  /* test tree fidelity */
        fprintf(stderr,"%s: %s: %s\n",programname,statname,NOTREE);
        return(FALSE);
      }
      n++; /* increment count of nodes */

      /* find the next node to use */
      while ((node != root) && (node == node->parent->right_child)) {
        node = node->parent; /* continue with the leaf node's parent */
      }
      if (node == root) break;
      else {
        node = node->parent->right_child;
      }
    }

    else { /* node has a child, go to left child */
      if(node->right_child == NULL) { /* test tree fidelity */
        fprintf(stderr,"%s: %s: %s\n",programname,statname,NOTREE);
        return(FALSE);
      }
      n++; /* increment count of nodes */
      node = node->left_child;
    }
  }

  /* when done constructing the tree, node should be the root */
  if (node != root || n != numnodes) {
    fprintf(stderr,"%s: %s: %s\n",programname,statname,NOTREE);
    return(FALSE);
  }

  fflush(statfile);
  return(TRUE);
}

BOOLEAN write_nested_subtree(root,numnodes,subtreefile,subtreename)
     TreeNode *root;
     long     numnodes;
     FILE     *subtreefile;
     char     *subtreename;
{
  long      i,n;           /* counters */
  TreeNode *node;          /* node used to point to the tree */

  if(!(node = newnode())) {
    fprintf(stderr,"%s: %s\n",programname,NOMEMORY);
    return(FALSE);
  }
  node = root;

  n = 0; /* count the number of nodes */
  for (i = 0; i < numnodes; i++) {

    /* write the node's count */
    if (fwrite((char *) &(node->count),sizeof(long),1,subtreefile)!= 1 ||
	ferror(subtreefile) || feof(subtreefile)){
      fprintf(stderr,"%s: %s: %s\n",programname,subtreename,NOWRITE);
      return(FALSE);
    }

    /* determine node position and from this find the next node to use */
    if (node->left_child == NULL) { /* node is terminal, find next node */
      if(node->right_child != NULL) {  /* test tree fidelity */
        fprintf(stderr,"%s: %s: %s\n",programname,subtreename,NOTREE);
        return(FALSE);
      }
      n++; /* increment count of nodes */

      /* find the next node to use */
      while ((node != root) && (node == node->parent->right_child)) {
        node = node->parent; /* continue with the leaf node's parent */
      }
      if (node == root) break;
      else {
        node = node->parent->right_child;
      }
    }

    else { /* node has a child, go to left child */
      if(node->right_child == NULL) { /* test tree fidelity */
        fprintf(stderr,"%s: %s: %s\n",programname,subtreename,NOTREE);
        return(FALSE);
      }
      n++; /* increment count of nodes */
      node = node->left_child;
    }
  }

  /* when done constructing the tree, node should be the root */
  if (node != root || n != numnodes) {
    fprintf(stderr,"%s: %s: %s\n",programname,subtreename,NOTREE);
    return(FALSE);
  }

  fflush(subtreefile);
  return(TRUE);
}

