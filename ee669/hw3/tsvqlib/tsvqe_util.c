/******************************************************************************
 * NAME
 *    tsvqe_util.c
 *    J. R. Goldschneider
 *    February 1994
 *    Last Revision
 *
 * SYNOPSIS
 *    DISTTYPE image_encode(root,bits,maxbits,numpixels)
 *    BOOLEAN  encode(vector,node,bits,distortion)
 *    DISTTYPE dist(data,codeword)
 *    double   empirical_entropy(root,numnodes)
 *    void     find_maxdepth(node)
 *    void     normalize_avmse(node)
 *
 * DESCRIPTION
 *    image_encode encodes the entire image from inputfile.  It assumes
 *    that the image has already been blocked.
 *
 *    encode recursively searches the tree using vector to find the
 *    appropriate codeword.  It then writes the codeword to the output
 *    file.  If the user specified printrate then encode also writes the
 *    appropriate information to ratefile.  The information in ratefile
 *    is stretched to fit values from 0 to 255 using maxdepth.
 *
 *    dist calculates the squared error between two vectors.
 *
 *    empirical_entropy returns the entopy of the codebook.
 *
 *    find_maxdepth finds the maximum depth of the tree.
 *
 *    normalize_avmse divides the avmse by count.
 *
 * RETURN VALUE
 *    image_encode returns the total disortion.  It also passes pointers to
 *    bits and maxbits to the calling function. If there is an error when
 *    encoding, a negative number is returned.
 *
 *    encode returns TRUE if it is able to write all of the requested
 *    information, otherwise it returns FALSE.
 *
 *    dist returns the squared error distortion between two vectors.
 *
 *    empirical_entropy returns the entopy of the codebook.
 *
 * PARAMETERS
 *    root is the root of the codebook tree.
 *    bits is the total number of bits used to encode the image.
 *    maxbits is the length of the longest codeword.
 *    numpixels is the number of pixels encoded.
 *
 *    vector contains the data to be encoded.
 *    node is the node of the codebook tree from which to begin coding.
 *    bits is the number of bits used to encode the present vector.
 *    distortion is the distortion incurred in encoding the present vector
 *       plus the distortion incurred in encoding all of the previous vectors.
 *
 *    data is the data vector.
 *    codeword is a codeword vector
 *
 *    root is the root of the codebook tree.
 *    numnodes is the total number of nodes in the tree.
 *
 * CALLS
 *    newnode()
 *
 *****************************************************************************/
#include "tsvq.h"

void find_maxdepth();
void normalize_avmse();

FILE *inputfile;
FILE *outputfile;
FILE *ratefile;

int   dim;
char  *programname;
char  outputname[NAME_MAX];
char  ratename[NAME_MAX];

BOOLEAN printrate;

int   maxdepth;

DISTTYPE image_encode(root,bits,maxbits,numpixels)
     TreeNode  *root;            /* codebook tree */
     long      *bits;            /* number of bits used to encode image */
     int       *maxbits;         /* longest codeword used */
     long      *numpixels;       /* number of pixels encoded */
{
  DISTTYPE   distortion;      /* distortion of encoded image */
  DISTTYPE   tempdistortion;  /* distortion of current encoded vector */
  int        tempbits;        /* length of codeword to encode current vector */
  DATATYPE   *datavector;     /* vector used to store data to be encoded */

  /* allocate memory for data vector */
  if (!(datavector = (DATATYPE *) calloc(dim, sizeof(DATATYPE)))) {
    fprintf(stderr,"%s: %s\n",programname,NOMEMORY);
    return(-1.0);
  }

  /* find the maximum depth of the tree */
  find_maxdepth(root);

  /* begin encoding */
  distortion = 0.0;
  *bits = 0;
  *maxbits = 0;
  *numpixels = 0;

  while(fread((char *) datavector,sizeof(DATATYPE),dim,inputfile) == dim &&
	!feof(inputfile) && !ferror(inputfile)) {

    if(!encode(datavector,root,&tempbits,&tempdistortion)) {
      return(-1.0);
    }

    *bits += tempbits;
    distortion += tempdistortion;
    if (tempbits > *maxbits) {
      *maxbits = tempbits;
    }
    *numpixels += dim;
  }

  normalize_avmse(root);

  free((char *) datavector);
  return(distortion);
}

BOOLEAN encode(vector,node,bits,distortion)
     DATATYPE *vector;       /* vector to encode */
     TreeNode *node;         /* codebook tree */
     int      *bits;         /* length of the codeword */
     DISTTYPE *distortion;   /* distortion of encoded vector */

{
  int      i; /* counter */
  DISTTYPE tempdist;

  tempdist = dist(vector,node->data);
  node->count += 1;
  node->avmse += tempdist;

  if((node->left_child == NULL) || (node->right_child == NULL)) {
    *distortion = tempdist;
    *bits = node->depth;

    /* round the codeword */
    for (i = 0; i < dim; i++) { /* for double data take the floor out */
      vector[i] = (DATATYPE) ( floor ( node->data[i] + 0.5));
      /* vector[i] = (DATATYPE) node->data[i]; */
    }

    /* write the data to the output file */
    if (fwrite((char *) vector,sizeof(DATATYPE),dim,outputfile) != dim) {
      fprintf(stderr,"%s: %s: %s\n",programname,outputname,NOWRITE);
      return(FALSE);
    }

    if(printrate) { /* output the rate of the codeword */
      for (i = 0; i < dim; i++) {
	vector[i] = (DATATYPE)
	  (floor( (float) 255.0 * node->depth / (float) maxdepth));
      }
      if (fwrite((char *) vector,sizeof(DATATYPE),dim,ratefile) != dim) {
	fprintf(stderr,"%s: %s: %s\n",programname,ratename,NOWRITE);
	return(FALSE);
      }
    }

    return(TRUE);
  }

  else {
    /* find the next node to go to */
    if(dist(vector,node->left_child->data) <
       dist(vector,node->right_child->data)) {
      return(encode(vector,node->left_child,bits,distortion));
    }
    else {
      return(encode(vector,node->right_child,bits,distortion));
    }
  }
}


DISTTYPE dist(data,codeword)
     DATATYPE *data;
     DISTTYPE *codeword;
{
  int      i;
  DISTTYPE distortion;

  distortion = 0.0;
  for (i = 0; i < dim; i++) {
    distortion +=
      (( (DISTTYPE) data[i]) - codeword[i]) *
	(( (DISTTYPE) data[i]) - codeword[i]);
  }

  return(distortion);
}

double empirical_entropy(root,numnodes)
     TreeNode *root;
     long     numnodes;
{
  TreeNode *node;
  long     i, n;
  double   entropy;
  long     numbervectors;

  if(!(node = newnode())) {
    fprintf(stderr,"%s: %s\n",programname,NOMEMORY);
    return(-1,0);
  }
  node = root;

  entropy = 0.0;
  numbervectors = 0;
  n = 0;
  for (i = 0; i < numnodes; i++) {
    /* determine node position and from this find the next node to use */
    if (node->left_child == NULL) { /* node is terminal */
      if(node->right_child != NULL) {  /* test tree fidelity */
        fprintf(stderr,"%s: %s\n",programname,NOTREE);
        return(-1.0);
      }

      n++;
      numbervectors += node->count;
      if (node->count > 0) {
	entropy += ( (double) (node->count)) * log( (double) node->count );
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

      /* test tree fidelity, a non-terminal node should have two childen */
      if(node->right_child == NULL) {
        fprintf(stderr,"%s: %s\n",programname,NOTREE);
        return(-1.0);
      }
      n++;
      node = node->left_child; /* search the next node of the tree */
    }
  }

  if (node != root || n != numnodes) {
    fprintf(stderr,"ccc%s: %s\n",programname,NOTREE);
    return(-1.0);
  }

  /* return the entropy */
  entropy = (log( (double) numbervectors) - (entropy/numbervectors)) / M_LN2;
  return(entropy);
}

void find_maxdepth(node)
     TreeNode *node;
{
  if (node->left_child == NULL) {
    if (node->depth > maxdepth) maxdepth = node->depth;
  }
  else {
    find_maxdepth(node->left_child);
    find_maxdepth(node->right_child);
  }
}

void normalize_avmse(node)
     TreeNode *node;
{
  if (node->count > 0) {
    node->avmse /= (DISTTYPE) node->count;
  }
  if (node->left_child != NULL) {
    normalize_avmse(node->left_child);
    normalize_avmse(node->right_child);
  }
}
