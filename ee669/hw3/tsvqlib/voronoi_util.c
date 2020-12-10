/******************************************************************************
 * NAME
 *    voronoi_util.c
 *    Jill Goldschneider and Preeti Trivedi
 *    March 1994
 *
 * SYNOPSIS
 *    void     normalize_tree(root)
 *    void     find_maxima(node,maxx,maxy,minx,miny)
 *    void     normalize_data(node,dx,dy)
 *    void     label_leaf_nodes(node,number,numleaves)
 *    BOOLEAN  voronoi_diagram(root)
 *    BOOLEAN  find_cell(vector,node)
 *    DISTTYPE int_dist(data,codeword)
 *
 * PARAMETERS
 *    root is the root of the codebook tree.
 *    node is the node of the codebook tree from which to begin coding.
 *    maxx and maxy represent maximum numbers out of given tree data points.
 *    minx and miny represent minimum numbers out of given tree data points.
 *    dx is maxx-minx, from given data.
 *    dy is maxy-miny, from given data.
 *    number is used to help label leaf nodes.
 *    numleaves is the number of leaves.
 *    data is the integer type data vector.
 *    codeword is a DISTTYPE type codeword vector
 *
 *****************************************************************************/
#include "tsvq.h"

char     *programname;
int      dim;
int      rows;
int      cols;

FILE     *outputfile;
char     outputname[NAME_MAX];

void     find_maxima();
void     normalize_data();
int      find_cell();
DISTTYPE int_dist();

void normalize_tree(root)
     TreeNode  *root;
{
  DISTTYPE maxx,maxy,minx,miny;

  /* initialize points */
  maxx = root->data[0];
  maxy = root->data[1];
  minx = root->data[0];
  miny = root->data[1];

  find_maxima(root,&maxx,&maxy,&minx,&miny);
  normalize_data(root,minx,miny,(maxx-minx),(maxy-miny));
}

void find_maxima(node,maxx,maxy,minx,miny)
     TreeNode *node;
     DISTTYPE *maxx,*maxy,*minx,*miny;
{
  if(node->data[0] < *minx)
    *minx=node->data[0];
  else if (node->data[0] > *maxx)
    *maxx=node->data[0];

  if(node->data[1] < *miny)
    *miny=node->data[1];
  else if (node->data[1] > *maxy)
    *maxy=node->data[1];

  if(node->left_child == NULL)
    return;
  else{
    find_maxima(node->left_child, maxx,maxy,minx,miny);
    find_maxima(node->right_child, maxx,maxy,minx,miny);
  }
}

void normalize_data(node,x,y,dx,dy)
     TreeNode *node;
     DISTTYPE x,y;
     DISTTYPE dx,dy;
{
  /* shift the points */
  node->data[0]-=x;
  node->data[1]-=y;

  /* fit points to cols by rows image with a border */
  node->data[0]=(node->data[0]/dx)*(rows - 2 * ((int) rows/20)) + rows/20;
  node->data[1]=(node->data[1]/dy)*(cols - 2 * ((int) cols/20)) + cols/20;

  if(node->left_child != NULL) {
    normalize_data(node->left_child,x,y,dx,dy);
    normalize_data(node->right_child,x,y,dx,dy);
  }
}


void label_leaf_nodes(node,number,numleaves)
     TreeNode *node;
     int *number;
     int numleaves;
{
  if(node->left_child == NULL) {
    node->count = *number;
    *number += 1;
    return;
  }
  else{
    label_leaf_nodes(node->left_child, number,numleaves);
    label_leaf_nodes(node->right_child,number,numleaves);
  }
}

BOOLEAN voronoi_diagram(root)
     TreeNode  *root;
{
  int  i,j;
  int  *datavector;
  int  **region_image;
  unsigned char **edge_image;

  /* allocate memory */
  if(!(datavector = (int *) calloc(dim,sizeof(int))) ||
     !(edge_image = (unsigned char **) calloc(rows,sizeof(unsigned char *)))) {
    fprintf(stderr,"%s: %s\n",programname,NOMEMORY);
    return(FALSE);
  }
  if(!(region_image = (int **) calloc((rows+1),sizeof(int *))) ||
     !(edge_image = (unsigned char **) calloc(rows,sizeof(unsigned char *)))) {
    fprintf(stderr,"%s: %s\n",programname,NOMEMORY);
    return(FALSE);
  }
  for (i = 0; i <= rows; i++) {
    if(!(region_image[i] = (int *) calloc((cols+1),sizeof(int)))) {
       fprintf(stderr,"%s: %s\n",programname,NOMEMORY);
       return(FALSE);
     }
  }
  for (i = 0; i < rows; i++) {
    if(!(edge_image[i]=(unsigned char *) calloc(cols,sizeof(unsigned char)))) {
       fprintf(stderr,"%s: %s\n",programname,NOMEMORY);
       return(FALSE);
     }
  }

  /* begin encoding blank image*/
  for (i = 0; i <= rows; i++) {
    for (j = 0; j <= cols; j++) {
      datavector[0] = i;
      datavector[1] = j;
      region_image[i][j] = find_cell(datavector,root);
    }
  }

  /* do edge detection */
  for (i = 0; i < rows; i++) {
    for (j = 0; j < cols; j++) {
      if((region_image[i][j] != region_image[i][j+1]) ||
	 (region_image[i][j] != region_image[i+1][j]) ||
	 (region_image[i][j] != region_image[i+1][j+1])) {
	edge_image[i][j] = 0;
      }
      else {
	edge_image[i][j] = 255;
      }
    }
  }

  /* write the voronoi image */
  for (i = 0; i < rows; i++) {
    if (fwrite(edge_image[i],sizeof(unsigned char),cols,outputfile) != cols) {
      fprintf(stderr,"%s: %s: %s\n",programname,outputname,NOWRITE);
      return(FALSE);
    }
  }
  return(TRUE);
}

int find_cell(vector,node)
     int *vector;
     TreeNode *node;
{
  DISTTYPE rightdistortion;
  DISTTYPE leftdistortion;

  if((node->left_child == NULL) && (node->right_child == NULL)) {

    /* write the data to the output file */
    return(node->count);
  }

  do {
    /* find the next node to go to */
    leftdistortion = int_dist(vector,node->left_child->data);
    rightdistortion = int_dist(vector,node->right_child->data);

    if( leftdistortion < rightdistortion) {
      node = node->left_child;
    }
    else {
      node = node->right_child;
    }
  } while(node->left_child != NULL);

  return(node->count);
}

/* cannot use dist in this program since its data type is char */

DISTTYPE int_dist(data,codeword)
     int      *data;
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
