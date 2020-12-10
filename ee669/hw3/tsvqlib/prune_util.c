/******************************************************************************
 * NAME
 *    prune_util.c
 *    J. R. Goldschneider
 *    May 1994
 *    Last Revision
 *
 * SYNOPSIS
 *    void     initialize_stat_tree(node)
 *    void     entropy_init_stat_tree(node)
 *    void     entropy_init(node)
 *    void     clear_count(node)
 *    TreeNode *find_min_slope(node)
 *    void     update_tree(node)
 *    DISTTYPE minimum(num1,num2,num3)
 *    void     prune(node,subtree_number)
 *    long     count_nodes(root)
 *    long     pruned_count(root)
 *
 * DESCRIPTION
 *    initialize_stat_tree does the initialization of the tree.  It finds
 *    delta_d, delta_r, lambda, and lambda_min for each node of the tree.
 *
 *    entropy_init_stat_tree does the initialization of the tree for entropy
 *    based pruning.  It finds delta_d, delta_r, lambda, and lambda_min for
 *    each node of the tree.
 *
 *    entropy_init is called by entropy_init_stat_tree.  It finds
 *    delta_d, delta_r, lambda, and lambda_min for each node of the tree.
 *
 *    clear_count is called by entropy_init_stat_tree.  It sets
 *    the node->count field of all nodes to zero.  This field is used
 *    to store the pruned subtree number since the counts of the
 *    vectors in each cell are not needed.
 *
 *    find_min_slope finds the node in the tree with the smallest slope.
 *
 *    update_tree corrects the delta_d, delta_r, lambda, and lambda_min
 *    values for the nodes from the given node to the root.
 *
 *    minimum returns the smallest number among the three numbers passed to it.
 *
 *    prune takes the codebook tree removes the descendents of any node
 *    that has a number between 1 and subtree_number.
 *
 *    countnodes counts the number of nodes in the tree.
 *
 *    pruned_count counts the number of nodes being pruned.
 *
 * RETURN VALUE
 *    find_min_slope returns a pointer to the node that has the
 *    smallest slope.
 *
 *    minimum returns the smallest number.
 *
 *    countnodes returns the number of nodes in the tree.
 *
 *    pruned_count returns the number of nodes being pruned.
 *
 * PARAMETERS
 *    node is the root of the codebook.
 *
 *    node in update_tree is the node with the smallest slope.
 *
 *    num1, num2, num3 are three numbers of type DISTTYPE
 *
 *    node is the root of the codebook.
 *    subtree_number is the subtree to create.
 *
 *    node is the root of the codebook.
 *
 *    node is the whose descendents are being pruned.
 *
 * CALLS
 *
 *****************************************************************************/
#include "tsvq.h"

DISTTYPE minimum();
char *programname;

void initialize_stat_tree(node)
     TreeNode *node; /* current node to initialize */
{
  /* the node is a leaf node */
  if (!(node->left_child)) {

    /* use the unnormalized squared error */
    node->avmse *= node->count;

    /* initialize leaf node values */
    node->data[DELTA_D] = 0.0;
    node->data[DELTA_R] = 0.0;
    node->data[LAMBDA] = HUGE;
    node->data[LAMBDA_MIN] = HUGE;

    /* use the node->count field to store the subtree_number */
    node->count = 0;
  }

  /* node is an internal node */
  else {
    initialize_stat_tree(node->left_child);
    initialize_stat_tree(node->right_child);

    /* use the unnormalized squared error */
    node->avmse *= node->count;

    /* initialize internal node */
    node->data[DELTA_D] =
      (node->right_child->avmse + node->left_child->avmse - node->avmse)
	+ node->left_child->data[DELTA_D] + node->right_child->data[DELTA_D];
    node->data[DELTA_R] = (double) (node->count)
      + node->left_child->data[DELTA_R]+ node->right_child->data[DELTA_R];
    node->data[LAMBDA] = -1.0 *  node->data[DELTA_D] / node->data[DELTA_R];
    node->data[LAMBDA_MIN] = minimum(node->data[LAMBDA],
				     node->left_child->data[LAMBDA_MIN],
				     node->right_child->data[LAMBDA_MIN]);

    /* use the node->count field to store the subtree_number */
    node->count = 0;
  }
}

void entropy_init_stat_tree(node)
     TreeNode *node; /* current node to initialize */
{
  entropy_init(node);
  clear_count(node);
}

void entropy_init(node)
     TreeNode *node; /* current node to initialize */
{
  /* the node is a leaf node */
  if (!(node->left_child)) {

    /* use the unnormalized squared error */
    node->avmse *= node->count;

    /* initialize leaf node values */
    node->data[DELTA_D] = 0.0;
    node->data[DELTA_R] = 0.0;
    node->data[LAMBDA] = HUGE;
    node->data[LAMBDA_MIN] = HUGE;
  }

  /* node is an internal node */
  else {
    entropy_init(node->left_child);
    entropy_init(node->right_child);

    /* use the unnormalized squared error */
    node->avmse *= node->count;

    /* initialize internal node for entropy based pruning */
    node->data[DELTA_D] =
      (node->right_child->avmse + node->left_child->avmse - node->avmse)
	+ node->left_child->data[DELTA_D] + node->right_child->data[DELTA_D];
    node->data[DELTA_R] = (node->count)*log((double) node->count)/M_LN2 -
      (node->left_child->count)*log((double) node->left_child->count)/M_LN2 -
	(node->right_child->count)*log((double) node->right_child->count)/M_LN2
	  + node->left_child->data[DELTA_R]+ node->right_child->data[DELTA_R];
    node->data[LAMBDA] = -1.0 *  node->data[DELTA_D] / node->data[DELTA_R];
    node->data[LAMBDA_MIN] = minimum(node->data[LAMBDA],
				     node->left_child->data[LAMBDA_MIN],
				     node->right_child->data[LAMBDA_MIN]);
  }
}

void clear_count(node)
     TreeNode *node; /* root node */
{
  /* use the node->count field to store the subtree_number */
  node->count = 0;
  if(node->left_child) {
    clear_count(node->left_child);
    clear_count(node->right_child);
  }
}

TreeNode *find_min_slope(node)
     TreeNode *node; /* root node */
{
  /* if the minimum slope is infinity, there is a problem */
  if (node->data[LAMBDA_MIN] == HUGE) {
    fprintf(stderr,"%s: %s\n",programname, NOTREE);
    return(NULL);
  }

  do {
    /* minimum slope is on left side of tree */
    if (node->data[LAMBDA_MIN] == node->left_child->data[LAMBDA_MIN]) {
      node = node->left_child;
    }

    /* minimum slope is on right side of tree */
    else if (node->data[LAMBDA_MIN] == node->right_child->data[LAMBDA_MIN]) {
      node = node->right_child;
    }

    /* minimum slope is at this node */
    else {
      return(node);
    }
  } while (node->left_child != NULL);

  /* there is a problem if a terminal leaf is reached */
  fprintf(stderr,"%s: %s\n",programname, NOTREE);
  return(NULL);
}

void update_tree(node)
     TreeNode *node;
{
  DISTTYPE delta_d,delta_r;

  /* find correction factors */
  delta_d = node->data[DELTA_D];
  delta_r = node->data[DELTA_R];

  /* make node a leaf node */
  node->data[DELTA_D] = 0.0;
  node->data[DELTA_R] = 0.0;
  node->data[LAMBDA] = HUGE;
  node->data[LAMBDA_MIN] = HUGE;

  /* update the internal nodes including the root */
  while (node->parent != NULL) {
    node = node->parent;
    node->data[DELTA_D] -= delta_d;
    node->data[DELTA_R] -= delta_r;
    node->data[LAMBDA] = -1.0 * node->data[DELTA_D] / node->data[DELTA_R];
    node->data[LAMBDA_MIN] = minimum(node->data[LAMBDA],
				     node->left_child->data[LAMBDA_MIN],
				     node->right_child->data[LAMBDA_MIN]);
  }
}

DISTTYPE minimum(num1,num2,num3)
     DISTTYPE num1,num2,num3;
{
  if (num1 < num2) {
    if (num1 < num3) {
      return(num1);
    }
    else {
      return(num3);
    }
  }

  if (num2 < num3) {
    return(num2);
  }
  else {
    return(num3);
  }
}

void prune(node,subtree_number)
     TreeNode *node;
     long     subtree_number;
{
  if ((node->count > 0) && (node->count <= subtree_number)) {
    node->left_child = NULL;
    node->right_child = NULL;
  }
  if (node->left_child != NULL) {
    prune(node->left_child,subtree_number);
    prune(node->right_child,subtree_number);
  }
}

long count_nodes(root)
     TreeNode *root;
{
  TreeNode *node;
  long     n;

  if(!(node = newnode())) {
    fprintf(stderr,"%s: %s\n",programname,NOMEMORY);
    return(0);
  }
  node = root;

  /* count the number of nodes */
  n = 0;
  for ( ; ; ) {

    /* determine node position and from this find the next node to use */
    if (node->left_child == NULL) { /* node is terminal, find next node */
      if (node->right_child != NULL) { /* test tree fidelity */
	fprintf(stderr,"%s: %s\n",programname,NOTREE);
	return(0);
      }

      n++; /* increment count of nodes */
      while ((node != root) && (node == node->parent->right_child)) {
	node = node->parent; /* continue with the leaf node's parent */
      }
      /* test tree structure fidelity */
      if (node == root) break;
      else if(node->parent->right_child == NULL) {
	fprintf(stderr,"%s: %s\n",programname,NOTREE);
	return(0);
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

  return(n);
}

long pruned_count(root)
     TreeNode *root;
{
  TreeNode *node;
  long     n;

  if(!(node = newnode())) {
    fprintf(stderr,"%s: %s\n",programname,NOMEMORY);
    return(0);
  }
  node = root;

  /* count the number of nodes just pruned */
  n = 0;
  for ( ; ; ) {

    /* determine node position and from this find the next node to use */

    /* node is terminal, find next node */
    if (node->data[LAMBDA_MIN] == HUGE) {
      n++; /* increment count of nodes */
      while ((node != root) && (node == node->parent->right_child)) {
	node = node->parent; /* continue with the leaf node's parent */
      }
      /* test tree structure fidelity */
      if (node == root) break;
      else if(node->parent->right_child == NULL) {
	fprintf(stderr,"%s: %s\n",programname,NOTREE);
	return(0);
      }
      else {
	node = node->parent->right_child;
      }
    }

    /* node is not a terminal node, go to the left child */
    else {
      /* test tree structure fidelity */
      if((node->left_child == NULL) || (node->right_child == NULL)) {
	fprintf(stderr,"%s: %s\n",programname,NOTREE);
	return(FALSE);
      }
      n++; /* increment count of nodes */
      node = node->left_child;
    }
  }

  return(n);
}
