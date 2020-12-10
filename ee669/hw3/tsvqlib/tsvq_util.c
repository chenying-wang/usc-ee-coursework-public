/******************************************************************************
 * NAME
 *    tsvq_util.c
 *    J. R. Goldschneider
 *    February 1994
 *    Last Revision:
 *
 * SYNOPSIS
 *    BOOLEAN initialize_tree(root,trsqfile,trsqname)
 *    BOOLEAN lloyd(node)
 *    void    split_node(node,leftcentroid,rightcentroid)
 *    void    split_alternate(node,leftcentroid,rightcentroid))
 *    long    tree_clean(root)
 *
 * DESCRIPTION
 *    initialize_tree reads the training vectors and stores them in the
 *    root node, finds the centroid of the training sequence,
 *    and assigns values to the root structure's members.
 *
 *    lloyd performs the Generalized Lloyd Algorithm on the node's data.
 *    It must create the node's children and find two initial codewords
 *    with a call to split_node.  If the node has zero distortion, lloyd
 *    will return at this point and the slope will be zero.  If the node
 *    has a non-zero distortion (count >= 1) then lloyd will try to
 *    split the node.  If lloyd finds that it cannot create two cells with
 *    non-zero counts when the distortion of the parent node is non-zero,
 *    it will call split_alternate as a second attempt to create two
 *    cells with non-zero count.  If this second attempt fails, then the
 *    program will not try any other splits, and it will exit the GLA loop.
 *    If the threshold is set to zero, we know that at each node the
 *    centroid of the training vectors is the codeword. Therefore the codeword
 *    of the non-zero count child cell is the same as parent cell.
 *    In this case the resulting slope will be zero, so the two children
 *    nodes will not be added to the tree.  If the threshold is greater
 *    than zero, then it may be that the centroid of the training vectors
 *    is NOT the codeword!  Therefore the codeword of the non-zero count
 *    child cell may be different from that of its parent and so that
 *    child's distortion may by less than that of its parent.  In this
 *    case the resulting slope is positive, so the two children may
 *    be added to the tree.  This would result in the addition of an
 *    empty cell into the codebook tree, i. e. a wasted bit. Since a user
 *    defined threshold is still an option, lloyd checks for empty cells
 *    and prints a warning to the user.  So the moral of this story is to
 *    use a threshold of zero.  Once the codewords are found, lloyd takes
 *    the training sequence of the parent node and divides it among its
 *    children.  The parent node's training sequence pointer is freed to
 *    save space.
 *
 *    split_node creates two codewords.  The left codeword is given
 *    the parent's data, and the right codeword is given a perturbed version
 *    of the parent's data. The perturbation is that the
 *    rightcentroid[i] = node->data[i] * (1 + mult_offset)
 *    where i = 0, 1, ..., dim.
 *
 *    split_alternate is called when the lloyd iteration has tried to split
 *    a node with a non-zero distortion, but that split has been unsuccessful.
 *    alternate split assigns the centroid as one initial codeword, and it
 *    finds the training vector that is furthest from the centroid and makes
 *    it the second initial codeword.
 *
 *    tree_clean removes all of the nodes from the tree that are NOT designed.
 *    It also counts the number of designed nodes and checks for tree
 *    structure errors. This should be called before write_codebook and
 *    write_stat.
 *
 * RETURN VALUE
 *    initialize_tree returns TRUE if successful.  If it not successful, then
 *    an error message is printed and FALSE is returned.
 *
 *    lloyd returns TRUE if there are no problems, otherwise it returns
 *    FALSE and displays an error message.
 *
 *    split_node does not return any value.  It does modify the node's
 *    children's data.
 *
 *    split_alternate does not return any value.  It does modify the node's
 *    children's data.
 *
 *    tree_clean returns the number of nodes in the tree.  If there is a
 *    tree structure error, then it returns a zero.
 *
 * PARAMETERS
 *    root is the root of the codebook tree.
 *
 *    root is the root node of the tree.
 *    trsqfile is the file containing the training sequence.
 *    trsqname is the name of the training sequence file.
 *
 *    node is the node to be split.
 *
 *    node is the node to use to find the initial codewords.
 *    leftcentroid is the initial codeword that is the same as the parent.
 *    rightcentroid is the different codeword.
 *
 * CALLS
 *    dist() newchild() newnode()
 *
 *****************************************************************************/
#include "tsvq.h"

char     *programname;
int      dim;
DISTTYPE mult_offset;
DISTTYPE thresh;

void split_node();
void split_alternate();

BOOLEAN initialize_tree(root,trsqfile,trsqname)
     TreeNode *root;       /* node to initialize */
     FILE     *trsqfile;   /* training sequence file */
     char     *trsqname;   /* training sequence name */
{
  DATATYPE *data_vector;  /* used to store one training vector */
  DISTTYPE distortion;    /* distortion of training vectors to centroid */
  long     count;         /* the number of training vectors */
  long     i,j;           /* counters */

  /* allocate memory for data_vector */
  if (!(data_vector = (DATATYPE *) calloc(dim,sizeof(DATATYPE)))) {
    fprintf(stderr,"%s: %s\n",programname,NOMEMORY);
    return(FALSE);
  }

  /* set the codeword to all zeros */
  for (i = 0; i < dim; i++) {
    root->data[i] = 0.0;
  }

  /* find the number of training vectors */
  rewind(trsqfile);
  for (count = 0; ; ) {
    if (fread((char *) data_vector,sizeof(DATATYPE),dim,trsqfile) != dim ||
	feof(trsqfile) || ferror(trsqfile)) {
      break;
    }
    count++;
  }

  if (count == 0) {
    fprintf(stderr,"%s: %s: %s\n",programname,trsqname,NODATA);
    return(FALSE);
  }

  /* allocate memory for the training vector array */
  if (!(root->trainseq = (DATATYPE **) calloc(count,sizeof(DATATYPE *)))) {
    fprintf(stderr,"%s: %s\n",programname,NOMEMORY);
    return(FALSE);
  }

  /* read the training vectors, and compute centroid */
  rewind(trsqfile);
  for (i = 0; i < count;  i++) {

    /* read in the next training vector */
    if (fread((char *) data_vector,sizeof(DATATYPE),dim,trsqfile) != dim ||
	feof(trsqfile) || ferror(trsqfile)) {
      fprintf(stderr,"%s: %s: %s\n",programname,trsqname,NOREAD);
      return(FALSE);
    }

    /* allocate memory for the training vector */
    if(!(root->trainseq[i] = (DATATYPE *) calloc(dim,sizeof(DATATYPE)))){
      fprintf(stderr,"%s: %s\n",programname,NOMEMORY);
      return(FALSE);
    }

    /* copy the training vector and add to the sum of training vectors */
    for (j = 0; j < dim; j++) {
      root->trainseq[i][j] = data_vector[j];
      root->data[j] += ((DISTTYPE) data_vector[j]);
    }
  }

  /* normalize the sum of training vectors */
  for (i = 0; i < dim; i++) {
    root->data[i] /= ((DISTTYPE) count);
  }

  root->count = count;
  root->designed = TRUE;

  /* compute the average distortion */
  distortion = 0.0;
  for (i = 0; i < count; i++) {
    distortion += dist(root->trainseq[i],root->data);
  }
  distortion /= ((DISTTYPE) root->count);
  root->avmse = distortion;

  /* release memory */
  free((char *) data_vector);

  return(TRUE);
}

BOOLEAN lloyd(node)
     TreeNode *node;
{
  long     leftcount, rightcount;
  DISTTYPE *leftcentroid, *rightcentroid;
  DISTTYPE leftdist, rightdist;
  DISTTYPE current_dist, past_dist;
  DISTTYPE current_leftdist, current_rightdist;
  long     i,j;
  BOOLEAN  alternate_flag = FALSE; /* avoid an infinite loop */

  /* allocate space centroids of the left and right nodes */
  if(!(leftcentroid = (DISTTYPE *) calloc(dim,sizeof(DISTTYPE))) ||
     !(rightcentroid = (DISTTYPE *) calloc(dim,sizeof(DISTTYPE)))) {
    fprintf(stderr,"%s: %s\n",programname,NOMEMORY);
    return(FALSE);
  }

  /* create the node's children */
  if(!(node->left_child = newchild(node)) ||
     !(node->right_child = newchild(node))) {
    fprintf(stderr,"%s: %s\n",programname,NOMEMORY);
    return(FALSE);
  }

  /* initialize the codebook */
  split_node(node,leftcentroid,rightcentroid);

  /* initialize the current distortion as some enormous number */
  current_dist = HUGE;

  /* if the node's distortion is zero, leave, the slope will be zero.
     this is also the appropriate exit for a node with a zero count */
  if(node->avmse <= 0) return(TRUE);

  do{

    /* assign the codewords and clear the centroids, counts, and distortions */
    for (j = 0; j < dim; j++) {
      node->left_child->data[j] = leftcentroid[j];
      leftcentroid[j] = 0.0;
      node->right_child->data[j] = rightcentroid[j];
      rightcentroid[j] = 0.0;
    }
    leftcount = 0;
    rightcount = 0;
    leftdist = 0.0;
    rightdist = 0.0;

    /* do a lloyd iteration */
    for (i = 0; i < node->count; i++) {
      current_leftdist = dist(node->trainseq[i],node->left_child->data);
      current_rightdist = dist(node->trainseq[i],node->right_child->data);
      if (current_leftdist < current_rightdist) {
	for (j = 0; j < dim; j++) {
	  leftcentroid[j] += ((DISTTYPE) node->trainseq[i][j]);
	}
	leftcount += 1;
	leftdist += current_leftdist;
      }
      else {
	for (j = 0; j < dim; j++) {
	  rightcentroid[j] += ((DISTTYPE) node->trainseq[i][j]);
	}
	rightcount += 1;
	rightdist += current_rightdist;
      }
    }

    /* find average mse distortion */
    if (leftcount != 0)
      leftdist /= ((DISTTYPE) leftcount);
    if (rightcount != 0)
      rightdist /= ((DISTTYPE) rightcount);

    /* save old distortion, and find new distortion */
    past_dist = current_dist;
    current_dist = (leftdist * ((DISTTYPE) leftcount)/((DISTTYPE) node->count))
      + (rightdist * ((DISTTYPE) rightcount) / ((DISTTYPE) node->count));

    /* find the centroids for the next iteration */
    for (j = 0; j < dim; j++) {
      if (leftcount != 0)
	leftcentroid[j] /= ((DISTTYPE) leftcount);
      if (rightcount != 0)
	rightcentroid[j] /= ((DISTTYPE) rightcount);
    }

    /* if the distortion is 0, stop trying to split the node */
    if (current_dist == 0.0) break;

    /* if alternate_flag is true, a forced split has already been attempted,
       do not try it again since an infinite loop would occur */
    if ((leftcount == 0 || rightcount == 0) && alternate_flag) {
      /* do nothing, will exit when current_dist = past_dist */
    }

    /* else if the distortion is not 0 and the split was not successful,
       force a better split */
    else if (leftcount == 0 || rightcount == 0) {
      split_alternate(node,leftcentroid,rightcentroid);
      alternate_flag = TRUE;
      past_dist = HUGE;
    }

  }  while ( !(((past_dist - current_dist)/past_dist) <= thresh) );
  /* finished with iterations */

  /* save the avmse and count of the node's children */
  node->left_child->avmse = leftdist;
  node->right_child->avmse = rightdist;
  node->left_child->count = leftcount;
  node->right_child->count = rightcount;

  /* prepare to store the training sequence among the children */
  if (!(node->left_child->trainseq =
	(DATATYPE **)calloc(leftcount,sizeof(DATATYPE *))) ||
      !(node->right_child->trainseq =
	(DATATYPE **)calloc(rightcount,sizeof(DATATYPE *)))) {
    fprintf(stderr,"%s: %s\n",programname,NOMEMORY);
    return(FALSE);
  }

  /* divide the training set among the node's children */
  leftcount = 0;
  rightcount = 0;
  for (i = 0; i < node->count; i++) {
    if (dist(node->trainseq[i],node->left_child->data) <
	dist(node->trainseq[i],node->right_child->data)) {
      node->left_child->trainseq[leftcount] = node->trainseq[i];
      leftcount += 1;
    }
    else {
      node->right_child->trainseq[rightcount] = node->trainseq[i];
      rightcount += 1;
    }
  }

  /* When the convergence threshold thresh is greater than zero,the codeword
     of a given voronoi region is not the average of the training vectors in
     the voronoi region.  Hence it is possible that the Lloyd iteration may
     stop with a voronoi region that has one or more identical training
     vectors, but that the codeword may not be the same as the training
     vector.  If a tree that completely resolves the training data is to be
     grown, i.e. one leaf for each distinct codeword, then it may not be
     possible to create a tree with zero distortion.  This is solved by
     allowing any node with a non-zero distortion to split, even if
     the node's count is only one, since there is still a positive slope
     that represents the goodness of the split.  Allowing the node
     to split may cause the growing algorithm to add an empty cell to
     the tree.

     There may be another problem when the convergence threshold is greater
     than zero.  It is possible that a node with a non-zero avmse has failed
     to split and has a child node with the parent node's training vectors
     and a lower avmse. This would result in a positive slope which may cause
     the growing algorithm to add an empty cell to the tree. This
     would happen rarely and only if the threshold is too large.

     Since the option of a changing threshold is still allowed, a test
     is inserted here to check for an empty cell problem.  If an
     empty cell is found, the user is notified of the problem and the
     use of a smaller threshold is recommended. An empty cell problem
     does not cause the program to terminate. */

  if ((rightcount == 0 && node->left_child->avmse < node->avmse) ||
      (leftcount == 0 && node->right_child->avmse < node->avmse)) {
    fprintf(stderr,"%s: %s: %s\n",programname,EMPTY_CELL,SMALL_THRESH);
  }

  free((char *) node->trainseq);
  free((char *) leftcentroid);
  free((char *) rightcentroid);
  return(TRUE);
}

void split_node(node,leftcentroid,rightcentroid)
     TreeNode *node;
     DISTTYPE *leftcentroid;
     DISTTYPE *rightcentroid;
{
  int i;

  for (i = 0; i < dim; i++) {
    leftcentroid[i] = node->data[i];
    rightcentroid[i] = (node->data[i]) * (1 + mult_offset);
  }
}

void split_alternate(node,leftcentroid,rightcentroid)
     TreeNode *node;
     DISTTYPE *leftcentroid;
     DISTTYPE *rightcentroid;
{
  long   i, j;
  double maxdist = 0.0;
  double tempdist;

  /* find the training vector that is the furthest from the centroid */
  j = 0;
  for (i = 0; i < node->count; i++) {
    if ((tempdist = dist(node->trainseq[i],node->data)) > maxdist) {
      j = i;
      maxdist = tempdist;
    }
  }

  /* copy new codewords */
  for (i = 0; i < dim; i++) {
    leftcentroid[i] = node->data[i];
    rightcentroid[i] = (DISTTYPE) node->trainseq[j][i];
  }
}

long tree_clean(root)
     TreeNode *root;
{
  TreeNode *node;
  long     n;

  if(!(node = newnode())) {
    fprintf(stderr,"%s: %s\n",programname,NOMEMORY);
    return(0);
  }
  node = root;

  /* count the number of designed nodes in the tree, and get rid of those
     nodes that are not designed */
  n = 0;
  for ( ; ; ) {
    /* determine node position and from this find the next node to use */
    if (node->left_child == NULL) { /* node is terminal */

      if(node->right_child != NULL) {  /* test tree fidelity */
        fprintf(stderr,"%s: %s\n",programname,NOTREE);
        return(0);
      }

      if(node->designed) n++; /* increment count of designed nodes */
      else { /* node is not designed, get rid of it and its sibling */
	node = node->parent;

	node->left_child = NULL;
	node->right_child = NULL;
      }

      /* find the next node to examine */
      while ((node != root) && (node == node->parent->right_child)) {
        node = node->parent; /* continue with the leaf node's parent */
      }

      /* tree search complete */
      if (node == root) break;

      else {
        node = node->parent->right_child;
      }
    }

    else { /* node has a child, go to left child */

      /* test tree fidelity, a non-terminal node should be designed and
         should have two childen */
      if(node->right_child == NULL || !(node->designed)) {
        fprintf(stderr,"%s: %s\n",programname,NOTREE);
        return(0);
      }
      n++; /* increment count of designed nodes */
      node = node->left_child; /* search the next node of the tree */
    }
  }

  /* return a count of the number of designed nodes */
  return(n);
}

