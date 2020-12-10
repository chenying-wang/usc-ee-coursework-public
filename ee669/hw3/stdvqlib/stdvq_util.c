/******************************************************************************
 *
 * NAME
 *    stdvq_util.c
 *    J. R. Goldschneider 5/93
 *
 * MODIFICATIONS
 *    7/93 min_sup and max_inf added for constrained searches. JRG
 *    5/94 cleaned up two cast mistakes in min_sup and max_inf. JRG
 *
 * SYNOPSIS
 *    splitcodewords(codebook,oldsize,newsize)
 *    perturb(oldcodeword,newcodeword)
 *    min_sup(first, last, tempnorm, norm)
 *    max_inf(first, last, tempnorm, norm)
 *    writecodebook(codebook,size)
 *
 * DESCRIPTION
 *    see below
 *
 * RETURN VALUE
 *    see below
 *
 * PARAMETERS
 *    see below
 *
 * CALLS
 *
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "vq.h"
#include "stdvq.def"
#include "stdvq.h"

/******************************************************************************
 *
 *    splitcodewords takes a codebook and creates oldsize-newsize new codewords
 *    from the old codewords.  The old codewords are not modified.
 *
 *****************************************************************************/

void splitcodewords(codebook,oldsize,newsize,scale)
     double **codebook;
     long   oldsize;
     long   newsize;
     long   scale;

{
  int    i;
  void   perturb();

  /* create the new codewords */
  for (i = 0; i < newsize - oldsize; i++) {
    perturb(codebook[i],codebook[i+oldsize],scale);
  }
}

/******************************************************************************
 *
 *    perturb takes oldcodeword and changes is slightly to form
 *    newcodeword.  oldcodeword is not altered in the process.
 *    scale is available to change so that a codeword can be split many times
 *    if necessary and still have each resulting new codeword be different.
 *
 *****************************************************************************/

void perturb(oldcodeword,newcodeword,scale)
     double *oldcodeword;
     double *newcodeword;
     long   scale;
{
  int i;
  float  addoffset, muloffset;

  addoffset = offset_add / (pow(2.0, (double) scale));
  muloffset = offset_mul / (pow(2.0, (double) scale));

  for (i = 0; i < dimension; i++) {
    if (oldcodeword[i] == 0.0) {
      newcodeword[i] = addoffset*((double) rand())/2147483647.0;
    }
    else if (fabs(oldcodeword[i]) < 0.9*addoffset) {
      newcodeword[i]=oldcodeword[i]+addoffset*
	(fabs(oldcodeword[i])/oldcodeword[i])*((double) rand())/2147483647.0;
    }
    else {
      newcodeword[i]=oldcodeword[i]+muloffset*
	oldcodeword[i]*((double) rand())/2147483647.0;
    }
  }
}

/******************************************************************************
 *
 *    min_sup returns the index of the smallest element of norm which is
 *    greater than tempnorm.  If no element is found, then the last index
 *    is returned.
 *
 *    first is the lower index of the array.
 *    last is the upper index of the array.
 *    tempnorm is the comparison value.
 *    norm is the array to search.
 *
 *****************************************************************************/

long  min_sup(first, last, tempnorm, norm)
     long   first;
     long   last;
     double tempnorm;
     double *norm;
{
  if (first == last) return(first);
  if (tempnorm > norm[(first + last)/2] )
    return(min_sup( (first + last)/2 + 1, last, tempnorm, norm));
  else
    return(min_sup( first, (first + last)/2, tempnorm, norm));
}

/******************************************************************************
 *
 *    max_inf returns the index of the largest element of norm which is
 *    less than tempnorm.  If no element is found, then the lowest index
 *    is returned.
 *
 *    first is the lower index of the array.
 *    last is the upper index of the array.
 *    tempnorm is the comparison value.
 *    norm is the array to search.
 *
 *****************************************************************************/

long  max_inf(first, last, tempnorm, norm)
     long   first;
     long   last;
     double tempnorm;
     double *norm;
{
  if (first == last) return(first);
  /* greater than or equals to fixes a bug in the this function for the
     case where tempnrom == norm[ (first + last + 1)/2] 5/12/2000 */
  if (tempnorm >= norm[(first + last + 1)/2] )
    return(max_inf( (first + last + 1)/2, last, tempnorm, norm));
  else
    return(max_inf( first, (first + last - 1)/2, tempnorm, norm));
}

/******************************************************************************
 *
 *    writecodebook writes to a file a codebook of a given size.
 *    If it fails it returns FALSE, other wise TRUE is returned.
 *    The format is
 *        TYPE       SIZE            DESCRIPTION
 *        long       1               number of codewords (size)
 *        integer    1               vector dimension (dimension)
 *        double     size*dimension  codewords
 *
 *****************************************************************************/

BOOLEAN writecodebook(codebook,size)
     double **codebook;
     long   size;
{
  char tempfilename[NAME_MAXIMUM];
  FILE *tempfile;
  long i;

  /* determine the name of the codebook */
  if (write_all_codebooks) {
    sprintf(tempfilename,"%s.%d",codebookname,size);
  }
  else {
    sprintf(tempfilename,"%s",codebookname);
  }

  /* open the codebook */
  if((tempfile = fopen(tempfilename,"w")) == NULL){
    fprintf(stderr,"%s: %s: %s\n",programname,tempfilename,NOTFOUND);
    return(FALSE);
  }

  /* write the codebook size and dimension */
  if ((fwrite((char *) &size,sizeof(long),1,tempfile) != 1) ||
      ferror(tempfile) || feof(tempfile)) {
    fprintf(stderr,"%s: %s: %s\n",programname,tempfilename,NOWRITE);
    return(FALSE);
  }
  if ((fwrite((char *) &dimension,sizeof(int),1,tempfile) != 1) ||
      ferror(tempfile) || feof(tempfile)) {
    fprintf(stderr,"%s: %s: %s\n",programname,tempfilename,NOWRITE);
    return(FALSE);
  }

  /* write the codebook */
  for (i = 0; i < size; i++) {
    if(fwrite(codebook[i], sizeof(double), dimension, tempfile) != dimension) {
      fprintf(stderr,"%s: %s: %s\n",programname,tempfilename,NOWRITE);
      fclose(tempfile);
      return(FALSE);
    }
  }

  fclose(tempfile);
  return(TRUE);
}
