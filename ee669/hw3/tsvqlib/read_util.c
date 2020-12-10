/******************************************************************************
 * NAME
 *    read_util.c
 *    J. R. Goldschneider
 *    March 1994
 *    Last Revision
 *
 * SYNOPSIS
 *    BOOLEAN construct_tree(root,numnodes,codebookfile,codebookname)
 *    BOOLEAN construct_stat_tree(root,numnodes,codebookfile,codebookname,
 *                                statfile,statname)
 *    BOOLEAN construct_subtree(root,numnodes,subtreefile,subtreename)

 * DESCRIPTION
 *    construct_tree constructs the codebook tree using root as the root node.
 *    Only the codewords are read.
 *
 *    construct_tree constructs the tree used for pruning where root is
 *    the root node. The codewords are not used for pruning, so this
 *    just reads the count and average distortion for each node.
 *
 *    construct_subtree reads the values from the subtree file
 *    and stores them in node->count.
 *
 * RETURN VALUE
 *    construct_tree returns TRUE if the tree was built successfully. If
 *    there was a problem constructing the tree an appropriate error
 *    message is displayed and FALSE is returned.
 *
 *    construct_stat_tree returns TRUE if the tree was built successfully. If
 *    there was a problem constructing the tree an appropriate error
 *    message is displayed and FALSE is returned.
 *
 *    construct_subtree returns TRUE if the subtree was read successfully. If
 *    there was a problem reading the subtrees an appropriate error
 *    message is displayed and FALSE is returned.
 *
 * PARAMETERS
 *    root is the root node of the tree.
 *    numnodes is the number of nodes in the tree.
 *    codebookfile is the file containing the treestructure and codewords.
 *    codebookname is the codebookfile's name.
 *    statfile is the file containing the counts and average squared errors.
 *    statname is the name of the statfile.
 *
 * CALLS
 *    newnode() newchild()
 *
 *****************************************************************************/
#include "tsvq.h"

int   dim;
char  *programname;

BOOLEAN construct_tree(root,numnodes,codebookfile,codebookname)
     TreeNode *root;
     long     numnodes;    /* number of nodes in the tree */
     FILE     *codebookfile;
     char     *codebookname;
{
  short    *tree_descrip;  /* array containing tree structure (in bytes) */
  long     i,n;            /* counters */
  TreeNode *node;          /* node used to build tree */

  /* allocate memory for the tree descriptor */
  if(!(tree_descrip = (short *) calloc(numnodes,sizeof(short)))) {
    fprintf(stderr,"%s: %s\n",programname,NOMEMORY);
    return(FALSE);
  }

  /* read in tree structure */
  if (fread((char *) tree_descrip,sizeof(short),(int) numnodes,codebookfile)
      != numnodes || ferror(codebookfile) || feof(codebookfile)) {
    fprintf(stderr,"%s: %s: %s\n",programname,codebookname,NOREAD);
    return(FALSE);
  }

  if(!(node = newnode())) {
    fprintf(stderr,"%s: %s\n",programname,NOMEMORY);
    return(FALSE);
  }
  node = root;

  n = 0; /* count the number of words read in */

  /* build the codebook given a node and its depth */
  for (i = 0; i < numnodes; i++) {

    /* read in node information */
    if((fread((char *) node->data,sizeof(DISTTYPE),dim,codebookfile) != dim) ||
	ferror(codebookfile) || feof(codebookfile)) {
      fprintf(stderr,"%s: %s: %s\n",programname,codebookname,NOREAD);
      return(FALSE);
    }

    /* determine node position and from this find the next node to use */
    if (tree_descrip[i] == 0) { /* node is terminal, find next node */
      n++; /* increment count of nodes */
      while ((node != root) && (node == node->parent->right_child)) {
	node = node->parent; /* continue with the leaf node's parent */
      }
      /* test tree structure fidelity */
      if (node == root) break;
      else if(node->parent->right_child != NULL) {
	fprintf(stderr,"%s: %s: %s\n",programname,codebookname,NOTREE);
	return(FALSE);
      }
      else {
	node = node->parent;
	if(!(node->right_child = newchild(node))) return(FALSE);
	node = node->right_child;
      }
    }

    else { /* node is not a terminal node, create the left child */
      /* test tree structure fidelity, this node should have no children */
      if((node->left_child != NULL) || (node->right_child != NULL)) {
	fprintf(stderr,"%s: %s: %s\n",programname,codebookname,NOTREE);
	return(FALSE);
      }
      n++; /* increment count of nodes */
      if(!(node->left_child = newchild(node))) return(FALSE);
      node = node->left_child;
    }
  }
  /* when done constructing the tree, node should be the root */
  if (node != root || n != numnodes) {
    fprintf(stderr,"%s: %s: %s\n",programname,codebookname,NOTREE);
    return(FALSE);
  }

  free((char *) tree_descrip);
  return(TRUE);
}

BOOLEAN construct_stat_tree(root,numnodes,codebookfile,codebookname,
			    statfile,statname)
     TreeNode *root;
     long     numnodes;    /* number of nodes in the tree */
     FILE     *codebookfile;
     char     *codebookname;
     FILE     *statfile;
     char     *statname;
{
  short    *tree_descrip;  /* array containing tree structure (in bytes) */
  long     i,n;            /* counters */
  TreeNode *node;          /* node used to build tree */

  /* allocate memory for the tree descriptor */
  if(!(tree_descrip = (short *) calloc(numnodes,sizeof(short)))) {
    fprintf(stderr,"%s: %s\n",programname,NOMEMORY);
    return(FALSE);
  }

  /* read in tree structure */
  if (fread((char *) tree_descrip,sizeof(short),(int) numnodes,codebookfile)
      != numnodes || ferror(codebookfile) || feof(codebookfile)) {
    fprintf(stderr,"%s: %s: %s\n",programname,codebookname,NOREAD);
    return(FALSE);
  }

  if(!(node = newnode())) {
    fprintf(stderr,"%s: %s\n",programname,NOMEMORY);
    return(FALSE);
  }
  node = root;

  n = 0; /* count the number of words read in */

  /* build the codebook given a node and its depth */
  for (i = 0; i < numnodes; i++) {

    /* read in node count and distortion information */
    if((fread((char *) &(node->count),sizeof(long),1,statfile) != 1) ||
	ferror(statfile) || feof(statfile)) {
      fprintf(stderr,"%s: %s: %s\n",programname,statname,NOREAD);
      return(FALSE);
    }
    if((fread((char *) &(node->avmse),sizeof(DISTTYPE),1,statfile) != 1) ||
	ferror(statfile) || feof(statfile)) {
      fprintf(stderr,"%s: %s: %s\n",programname,statname,NOREAD);
      return(FALSE);
    }

    /* determine node position and from this find the next node to use */
    if (tree_descrip[i] == 0) { /* node is terminal, find next node */
      n++; /* increment count of nodes */
      while ((node != root) && (node == node->parent->right_child)) {
	node = node->parent; /* continue with the leaf node's parent */
      }
      /* test tree structure fidelity */
      if (node == root) break;
      else if(node->parent->right_child != NULL) {
	fprintf(stderr,"%s: %s: %s\n",programname,codebookname,NOTREE);
	return(FALSE);
      }
      else {
	node = node->parent;
	if(!(node->right_child = newchild(node))) return(FALSE);
	node = node->right_child;
      }
    }

    else { /* node is not a terminal node, create the left child */
      /* test tree structure fidelity, this node should have no children */
      if((node->left_child != NULL) || (node->right_child != NULL)) {
	fprintf(stderr,"%s: %s: %s\n",programname,codebookname,NOTREE);
	return(FALSE);
      }
      n++; /* increment count of nodes */
      if(!(node->left_child = newchild(node))) return(FALSE);
      node = node->left_child;
    }
  }
  /* when done constructing the tree, node should be the root */
  if (node != root || n != numnodes) {
    fprintf(stderr,"%s: %s: %s\n",programname,codebookname,NOTREE);
    return(FALSE);
  }

  free((char *) tree_descrip);
  return(TRUE);
}

BOOLEAN construct_subtree(root,numnodes,subtreefile,subtreename)
     TreeNode *root;
     long     numnodes;    /* number of nodes in the tree */
     FILE     *subtreefile;
     char     *subtreename;
{
  TreeNode *node;          /* node used to build tree */
  long     i,n;            /* counters */

  if(!(node = newnode())) {
    fprintf(stderr,"%s: %s\n",programname,NOMEMORY);
    return(FALSE);
  }
  node = root;

  n = 0; /* count the number of words read in */

  /* read in the subtree numbers */
  for (i = 0; i < numnodes; i++) {

    /* read in node count and distortion information */
    if((fread((char *) &(node->count),sizeof(long),1,subtreefile) != 1) ||
	ferror(subtreefile) || feof(subtreefile)) {
      fprintf(stderr,"%s: %s: %s\n",programname,subtreename,NOREAD);
      return(FALSE);
    }

    /* determine node position and from this find the next node to use */
    if (node->left_child == NULL) { /* node is terminal, find next node */
      n++; /* increment count of nodes */
      while ((node != root) && (node == node->parent->right_child)) {
	node = node->parent; /* continue with the leaf node's parent */
      }
      /* test tree structure fidelity */
      if (node == root) break;
      else if(node->parent->right_child == NULL) {
	fprintf(stderr,"%s: %s\n",programname,NOTREE);
	return(FALSE);
      }
      else {
	node = node->parent->right_child;
      }
    }

    else { /* node is not a terminal node, go to the left child */
      /* test tree structure fidelity */
      if((node->left_child == NULL) || (node->right_child == NULL)) {
	fprintf(stderr,"%s: %s\n",programname,NOTREE);
	return(FALSE);
      }
      n++; /* increment count of nodes */
      node = node->left_child;
    }
  }
  /* when done constructing the tree, node should be the root */
  if (node != root || n != numnodes) {
    fprintf(stderr,"%s: %s\n",programname,NOTREE);
    return(FALSE);
  }

  return(TRUE);
}
