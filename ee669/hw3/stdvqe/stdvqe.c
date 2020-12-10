/******************************************************************************
 *
 * NAME
 *    stdvqe.c
 *    J. R. Goldschneider 5/93
 *
 * MODIFICATIONS
 *    11/93 modified to display defaults values. The -s option was added so
 *    that three programs were no longer maintained. JRG
 *
 *    3/94 JRG changed codebook so it is smart. It know knows the codebook size
 *    and dimension.  The format is:
 *        TYPE       SIZE            DESCRIPTION
 *        long       1               number of codewords (size)
 *        integer    1               vector dimension (dimension)
 *        double     size*dimension  codewords
 *
 *    12/95 JRG changed log2() to log()/M_LN2()
 *
 *    1/3/96 JRG changed one line in annulus() and sphere_annulus()
 *    to correct for an error in the sorting routine that ordered the
 *    codebook based on the norm.
 *
 *    9/4/97 JRG changed one line in annulus() and sphere_annulus()
 *    to correct for an index error in accessing the codeword.
 *
 * SYNOPSIS
 *    stdvqe -c codebook -i input -o f3 -s speedup -D
 *
 * DESCRIPTION
 *    stdvqe encodes file f1 using codewords from the codebook f2 that yield
 *    the lowest mean squared error.  The reproduction file is placed in f3.
 *    There are three options to speed up the search.  The first uses
 *    partial distortion for speedup.  The second two use constraints
 *    imposed by ordering the codewords. See  Huang, Bi, Stiles,
 *    and Harris in the IEEE Transactions on Image Processing July 1992.
 *    All three versions use the mean square error as the metric.
 *
 * OPTIONS
 *    -i  input file name
 *    -c  codebook file name
 *    -o  output file name
 *    -s  type of constrained search or speedup
 *
 * FLAGS
 *    -D  print cell counts and distortions
 *
 *    See vq.h for definitions of default values
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

FILE    *inputfile;        /* image to encode */
FILE    *outputfile;       /* reproduction image */
char    outputname[NAME_MAXIMUM]; /* name of reproduction image */

double  fullsearch();
double  annulus();
double  sphere_annulus();

int main(argc, argv)
     int  argc;
     char *argv[];

{
  char    option;          /* used for command line interpretation */
  char    inputname[NAME_MAXIMUM]; /* name of input image */
  FILE    *codebookfile;   /* pointer to codebook file */
  double  **codebook;      /* codewords stored [index][dimension] */
  double  *celldistortion; /* distortion of each voronoi region */
  double (*method)();      /* pointer to function */
  double  distortion;      /* total average distortion of encoded image */
  double  entropy;         /* entropy of encoded image */
  long    *count;          /* number of cells in each voronoi region */
  long    numbervectors;   /* number of vectors encoded */
  long    i;               /* index */
  int     speedup;         /* indicates which type of speedup to use */

  /* set default values */
  display_info = FALSE;
  programname = *argv;
  speedup = DEF_SPEEDUP;

  sprintf(inputname,DEF_INNAME);
  sprintf(outputname,DEF_OUTNAME);
  sprintf(codebookname,DEF_INNAME);

  /* if no options entered, list all of the defaults */
  if (argc == 1) {
    printf("%s %s %s\n",USAGE,programname,HOWTOUSE_STDVQE);

    printf("\nOPTIONS   DESCRIPTIONS                         DEFAULTS\n");
    printf("-c        codebook name                        %s\n",codebookname);
    printf("-i        input name                           %s\n",inputname);
    printf("-o        output name                          %s\n",outputname);
    printf("-D        print cell counts and distortions\n");
    printf("-s        constrained search choice            %d\n",speedup);
    printf("          0 for partial distortion only\n");
    printf("          1 for p.d. and constrained search\n");
    printf("          2 for p.d. and more constrained search\n");
    printf("\n");
    fflush(stdout);
    exit(0);
  }

  /* read and interpret command line arguments */
  while (--argc && ++argv) {
    if (*argv[0]=='-' && strlen(*argv)==2) { /* each option has 1 letter */
      option = *++argv[0];
      if (option == 'D') {display_info = TRUE;}/* examine the flag */
      else if (--argc && ++argv) { /* examine the option */
	switch(option) { /* examine the option letter */
	case 'i':
	  strncpy(inputname,*argv,NAME_MAX);
	  break;
	case 'c':
	  strncpy(codebookname,*argv,NAME_MAX);
	  break;
	case 'o':
	  strncpy(outputname,*argv,NAME_MAX);
	  break;
	case 's':
          sscanf(*argv,"%d",&speedup);
          break;
	default:
	  fprintf(stderr,"%s: %c: %s\n",programname,option,NOTOPTION);
	  exit(1);
	  break;
	}
      }
      else {
	fprintf(stderr,"%s %s %s\n",USAGE,programname,HOWTOUSE_STDVQE);
	exit(2);
      }
    }

    else if (*argv[0] == '-') { /* user entered unknown option */
      ++argv[0];
      fprintf(stderr,"%s: %s: %s\n",programname,*argv,NOTOPTION);
      exit(3);
    }
    else { /* user entered unknown string */
      fprintf(stderr,"%s: %s: %s\n",programname,*argv,NOTOPTION);
      exit(4);
    }
  }

  /* user did not enter an input file name or a codebook file name */
  if (strlen(inputname) == 0 || strlen(codebookname) == 0 ) {
    fprintf(stderr,"%s %s %s\n",USAGE,programname,HOWTOUSE_STDVQE);
    exit(5);
  }

  /* find speedup option */
  switch (speedup)
    {
    case 0:
      method = fullsearch;
      break;
    case 1:
      method = annulus;
      break;
    case 2:
      method = sphere_annulus;
      break;
    default:
      fprintf(stderr,"%s %s %s\n",USAGE,programname,HOWTOUSE_STDVQ);
      exit(6);
    }

  /* user entered an input name which is the same as the output name */
  if (strncmp(inputname,outputname,NAME_MAX) == 0) {
    fprintf(stderr,"%s: %s %s %s %s: %s\n",
	    programname,inputname,AND,outputname,ARESAME,ABORT_STDVQE);
    exit(7);
  }

  /* user entered a codebook name which is the same as the output name */
  if (strncmp(codebookname,outputname,NAME_MAX) == 0) {
    fprintf(stderr,"%s: %s %s %s %s: %s\n",
	    programname,codebookname,AND,outputname,ARESAME,ABORT_STDVQE);
    exit(8);
  }

  /* user entered an input name which is the same as the codebook name */
  if (strncmp(inputname,codebookname,NAME_MAX) == 0) {
    fprintf(stderr,"%s: %s %s %s %s: %s\n",
	    programname,inputname,AND,codebookname,ARESAME,ABORT_STDVQE);
    exit(9);
  }

  /* if an input file is given but not an output file, assign output name */
  if (strlen(outputname) == 0) {
    sprintf(outputname,"%s%s",inputname,DEF_APPEND_VQ);
  }

  /* open the files */
  if((inputfile = fopen(inputname,"r")) == NULL){
    fprintf(stderr,"%s: %s: %s\n",programname,inputname,NOTFOUND);
    exit(10);
  }
  if((codebookfile = fopen(codebookname,"r")) == NULL){
    fprintf(stderr,"%s: %s: %s\n",programname,codebookname,NOTFOUND);
    exit(11);
  }
  if((outputfile = fopen(outputname,"w")) == NULL){
    fprintf(stderr,"%s: %s: %s\n",programname,outputname,NOTFOUND);
    exit(12);
  }

  /* allocate memory for each codeword, read the codeword, and
     initialize its count and distortion */
  rewind(codebookfile);

  /* find the codebooksize */
  if (fread(&codebooksize,sizeof(long),1,codebookfile) != 1) {
    fprintf(stderr,"%s: %s: %s\n",programname,codebookname,NOREAD);
    exit(13);
  }

  /* find the dimension */
  if (fread(&dimension,sizeof(int),1,codebookfile) != 1) {
    fprintf(stderr,"%s: %s: %s\n",programname,codebookname,NOREAD);
    exit(14);
  }

  /* allocate memory for the codebook, cell counts, and cell distortions */
  if (!(codebook = (double **) calloc(codebooksize,sizeof(double *))) ||
      !(count = (long *) calloc(codebooksize,sizeof(long))) ||
      !(celldistortion  = (double *) calloc(codebooksize,sizeof(double)))) {
    fprintf(stderr,"%s: %s\n",programname,NOMEMORY);
    exit(15);
  }

  for(i = 0; i < codebooksize; i++) {
    if (!(codebook[i] = (double *) calloc(dimension,sizeof(double)))) {
      fprintf(stderr,"%s: %s\n",programname,NOMEMORY);
      exit(16);
    }
    if (fread(codebook[i],sizeof(double),dimension,codebookfile)!=dimension ||
	feof(codebookfile) || ferror(codebookfile) ) {
      fprintf(stderr,"%s: %s: %s\n",programname,codebookname,NOREAD);
      exit(17);
    }
    count[i] = 0;
    celldistortion[i] = 0.0;
  }
  fclose(codebookfile);

  /* encode the data */
  if((distortion = method(codebook,count,celldistortion)) < 0) {
    exit(18);
  }

  /* normalize the distortions and find the entropy */
  numbervectors = 0;
  entropy = 0.0;
  for (i = 0; i < codebooksize; i++) {
    if (count[i] != 0) {
      celldistortion[i] = celldistortion[i] / (double) count[i];
      entropy += (count[i]) * log( (double) count[i] );
    }
    numbervectors += count[i];
  }
  if (numbervectors != 0) {
    distortion /= (double) numbervectors;
    entropy = (log( (double) numbervectors) - entropy/numbervectors) / M_LN2;
  }

  /* output the statistics */
  printf("\n");
  printf("File to encode:       %s\n", inputname);
  printf("Codebook file:        %s\n", codebookname);
  printf("Number of Codewords:  %d\n", codebooksize);
  printf("Encoded file:         %s\n", outputname);
  printf("Vectors encoded:      %d\n", numbervectors);
  printf("Average distortion:   %f\n", distortion);
  printf("Rate (bpv):           %f\n", (log((double) codebooksize)/M_LN2));
  printf("Empirical entropy:    %f\n\n",entropy);

  /* Print the cell information if requested */
  if (display_info) {
    printf("\nCodeword  Count     Distortion\n\n");
    for (i = 0; i < codebooksize; i++) {
      printf("%-10d%-10d%-10f\n",i,count[i],celldistortion[i]);
    }
  }

  fflush(stdout);
  fclose(inputfile);
  fclose(outputfile);
  exit(0);
}

double fullsearch(codebook,count,celldistortion)
     double **codebook;
     long   *count;
     double *celldistortion;
{
  DATA    *tempvector;    /* vector to be encoded */
  double  distortion;     /* total image distortion */
  double  bestdistortion; /* distortion between vector and best codeword */
  double  tempdistortion; /* temporary variable */
  double  temp;           /* temporary variable */
  long    bestcodeword;   /* index of closest codeword */
  long    i,j;            /* counters and indices */

  /* allocate memory for the temporary vector */
  if(!(tempvector = (DATA *) calloc(dimension,sizeof(DATA)))) {
    fprintf(stderr,"%s: %s\n",programname,NOMEMORY);
    return(-1.0);
  }

  /* encode the data and write them to the output file, also find statistics */
  rewind(inputfile);
  distortion = 0.0;
  while (fread(tempvector, sizeof(DATA), dimension, inputfile) == dimension
	 && !feof(inputfile) && !ferror(inputfile) ) {
    bestdistortion = HUGE; /* keep convention that ties go to lower index */
    bestcodeword = 0;
    /* find the best codeword */
    for (i = 0; i < codebooksize; i++) {

      tempdistortion = 0.0;
      for (j = 0; j < dimension; j++) {
	temp = ( (double) tempvector[j]) - codebook[i][j];
	tempdistortion += temp*temp;
	if (tempdistortion > bestdistortion) j = dimension;
      }

      if (tempdistortion < bestdistortion) {
	bestdistortion = tempdistortion;
	bestcodeword = i;
      }

      /* if bestdistortion is 0 then the best codeword has been found */
      if (bestdistortion == 0.0) i = codebooksize;
    } /* the best codeword has been found */

    count[bestcodeword]++;
    celldistortion[bestcodeword] += bestdistortion;
    distortion += bestdistortion;
    /* round the codeword */
    for (i = 0; i < dimension; i++) { /* for double data remove floor */
      tempvector[i] = (DATA) (floor(codebook[bestcodeword][i] + 0.5));
      /* tempvector[i] = (DATA) codebook[bestcodeword][i]; */
    }
    /* write the data to the output file */
    if (fwrite(tempvector,sizeof(DATA),dimension,outputfile) != dimension) {
      fprintf(stderr,"%s: %s: %s\n",programname,outputname,NOWRITE);
      return(-1.0);
    }
  } /* all vectors have been encoded */

  return(distortion);
}

double annulus(codebook,count,celldistortion)
     double **codebook;
     long   *count;
     double *celldistortion;
{
  DATA    *tempvector;    /* vector to be encoded */
  double  *norm;          /* array of norms for each codeword */
  double  tempdouble;     /* temporary variable */
  double  tempnorm;       /* temporary variable used to sort codebook */
  double  distortion;     /* total image distortion */
  double  bestdistortion; /* distortion between vector and best codeword */
  double  temp;           /* temporary variable */
  long    *index;         /* array to store sorted codeword order */
  long    bestcodeword;   /* index of closest codeword */
  long    i,j;            /* counters and indices */
  long    imin,imax;      /* indices */
  long    bestnorm;       /* used as an index for sorting norm */

  /* allocate memory for tempvector, norm, and index */
  if(!(tempvector = (DATA *) calloc(dimension,sizeof(DATA))) ||
     !(norm = (double *) calloc(codebooksize, sizeof(double))) ||
     !(index = (long *) calloc(codebooksize, sizeof(long)))) {
    fprintf(stderr,"%s: %s\n",programname,NOMEMORY);
    return(-1.0);
  }

  /* initialize index order */
  for (i = 0; i < codebooksize; i++) {
    index[i] = i;
  }

  /* find the norm of each codeword and store in an array */
  for (i = 0; i < codebooksize; i++) {
    norm[i] = 0.0;
    for (j = 0; j < dimension; j++) {
      norm[i] += codebook[i][j]*codebook[i][j];
    }
    norm[i] = sqrt(norm[i]);
  }

  /* reorder the codebook by ascending norm */
  for (i = 0; i < codebooksize; i++) {
    bestnorm = i;
    /* find the lowest norm */
    for ( j = i; j < codebooksize; j++) {
      if (norm[j] < norm[bestnorm]) bestnorm = j;
    }
    /* record index change */
    j = index[i];
/*    index[i] = bestnorm; this in in error and is changed 1/3/96 */
    index[i] = index[bestnorm];
    index[bestnorm] = j;

    /* reorder the norm arrray */
    tempdouble = norm[i];
    norm[i] = norm[bestnorm];
    norm[bestnorm] = tempdouble;
  }

  /* encode the data and write them to the output file, also find statistics */
  rewind(inputfile);
  distortion = 0.0;
  while (fread(tempvector, sizeof(DATA), dimension, inputfile) == dimension
	 && !feof(inputfile) && !ferror(inputfile) ) {

    /* compute the norm of the input vector */
    tempnorm = 0.0;
    for (j = 0; j < dimension; j++) {
      tempnorm += (double) tempvector[j]*tempvector[j];
    }
    tempnorm = sqrt(tempnorm);

    /* find the codeword with a norm closest to the input vector */
    bestcodeword = min_sup((long) 0,codebooksize-1,tempnorm,norm);

    /* compute the distance between the codeword and the input vector that
       has the closest norm */
    tempdouble = 0.0;
    for (j = 0; j < dimension; j++) {
      temp = ( (double) tempvector[j]) - codebook[index[bestcodeword]][j];
      tempdouble += temp*temp;
    }
    tempdouble = sqrt(tempdouble);

    /* identify the subset of the codewords to search such that
       norm(input)-distance <= norm(codeword) <= norm(input)+distance */
    imin = min_sup((long) 0, codebooksize-1, tempnorm-tempdouble, norm);
    imax = max_inf(imin, codebooksize-1, tempnorm+tempdouble, norm);

    /* find the bestcodeword using partial distortion method */
    bestdistortion = HUGE; /* keep convention that ties go to lower index */
    /* find the best codeword */
    for (i = imin; i <= imax; i++) {
      tempdouble = 0.0;
      for (j = 0; j < dimension; j++) {
	temp = ( (double) tempvector[j]) - codebook[index[i]][j];
	tempdouble += temp*temp;
	if (tempdouble > bestdistortion) j = dimension; /* abort loop */
      }

      if (tempdouble < bestdistortion) {
	bestdistortion = tempdouble;
	bestcodeword = i;
      }

      /* if bestdistortion is 0 then the best codeword has been found */
      if (bestdistortion == 0.0) i = codebooksize;
    } /* the best codeword has been found */

    count[index[bestcodeword]]++;
    celldistortion[index[bestcodeword]] += bestdistortion;
    distortion += bestdistortion;
    /* round the codeword */
    for (i = 0; i < dimension; i++) { /* for double data remove floor */
/* tempvector[i] = (DATA) (floor(codebook[bestcodeword][i] + 0.5));
   this in in error and is changed 9/4/97 */
      tempvector[i] = (DATA) (floor(codebook[index[bestcodeword]][i] + 0.5));
      /* tempvector[i] = (DATA) codebook[index[bestcodeword]][i]; */
    }
    /* write the data to the output file */
    if (fwrite(tempvector,sizeof(DATA),dimension,outputfile) != dimension) {
      fprintf(stderr,"%s: %s: %s\n",programname,outputname,NOWRITE);
      return(-1.0);
    }
  } /* all vectors have been encoded */

  return(distortion);
}


double sphere_annulus(codebook,count,celldistortion)
     double **codebook;
     long   *count;
     double *celldistortion;
{
  DATA    *tempvector;    /* vector to be encoded */
  double  **dist_matrix;  /* array listing distances between all codewords */
  double  *norm;          /* array of norms for each codeword */
  double  tempdouble;     /* temporary variable */
  double  tempnorm;       /* temporary variable used to sort codebook */
  double  distortion;     /* total image distortion */
  double  bestdistortion; /* distortion between vector and best codeword */
  double  temp;           /* temporary variable */
  long    *index;         /* array to store sorted codeword order */
  long    bestcodeword;   /* index of closest codeword */
  long    i,j,k;          /* counters and indices */
  long    imin,imax;      /* indices */
  long    bestnorm;       /* used as an index for sorting norm */

  /* allocate memory for tempvector, norm, index and dist_matrix */
  if(!(tempvector = (DATA *) calloc(dimension,sizeof(DATA))) ||
     !(norm = (double *) calloc(codebooksize, sizeof(double))) ||
     !(index = (long *) calloc(codebooksize, sizeof(long))) ||
    !(dist_matrix  = (double **) calloc(codebooksize,sizeof(double *)))) {
    fprintf(stderr,"%s: %s\n",programname,NOMEMORY);
    return(-1.0);
  }
  for (i = 0; i < codebooksize; i++) {
    if (!(dist_matrix[i] = (double *) calloc(codebooksize,sizeof(double)))) {
      fprintf(stderr,"%s: %s\n",programname,NOMEMORY);
      return(-1);
    }
  }

  /* initialize index order */
  for (i = 0; i < codebooksize; i++) {
    index[i] = i;
  }

  /* find the norm of each codeword and store in an array */
  for (i = 0; i < codebooksize; i++) {
    norm[i] = 0.0;
    for (j = 0; j < dimension; j++) {
      norm[i] += codebook[i][j]*codebook[i][j];
    }
    norm[i] = sqrt(norm[i]);
  }

  /* reorder the codebook by ascending norm */
  for (i = 0; i < codebooksize; i++) {
    bestnorm = i;
    /* find the lowest norm */
    for ( j = i; j < codebooksize; j++) {
      if (norm[j] < norm[bestnorm]) bestnorm = j;
    }
    /* record index change */
    j = index[i];
/*    index[i] = bestnorm; this in in error and is changed 1/3/96 */
    index[i] = index[bestnorm];
    index[bestnorm] = j;

    /* reorder the norm arrray */
    tempdouble = norm[i];
    norm[i] = norm[bestnorm];
    norm[bestnorm] = tempdouble;
  }

  /* find the table of distortions between all of the codewords */
  for (i = 0; i < codebooksize; i++) {
    for (j = i; j < codebooksize; j++) {
      tempdouble = 0.0;
      for (k = 0; k < dimension; k++) {
	temp = ( (double) codebook[index[i]][k]) - codebook[index[j]][k];
	tempdouble += temp*temp;
      }
      tempdouble = sqrt(tempdouble);
      dist_matrix[i][j] = tempdouble;
      dist_matrix[j][i] = tempdouble;
    }
  }

  /* encode the data and write them to the output file, also find statistics */
  distortion = 0.0;
  rewind(inputfile);
  while (fread(tempvector, sizeof(DATA), dimension, inputfile) == dimension
	 && !feof(inputfile) && !ferror(inputfile) ) {
    /* compute the norm of the input vector */
    tempnorm = 0.0;
    for (j = 0; j < dimension; j++) {
      tempnorm += (double) tempvector[j]*tempvector[j];
    }
    tempnorm = sqrt(tempnorm);

    /* find the codework with a norm closest to the input vector */
    bestcodeword = min_sup((long) 0,codebooksize-1,tempnorm,norm);

    /* compute the distance between the codeword and the input vector that
       has the closest norm */
    tempdouble = 0.0;
    for (j = 0; j < dimension; j++) {
      temp = ( (double) tempvector[j]) - codebook[index[bestcodeword]][j];
      tempdouble += temp*temp;
    }
    tempdouble = sqrt(tempdouble);

    /* identify the subset of the codewords to search such that
       norm(input)-distance <= norm(codeword) <= norm(input)+distance */
    imin = min_sup((long) 0, codebooksize-1, tempnorm-tempdouble, norm);
    imax = max_inf(imin, codebooksize-1, tempnorm+tempdouble, norm);

    /* find the bestcodeword using partial distortion method */
    bestdistortion = HUGE; /* keep convention that ties go to lower index */
    /* find the best codeword */
    for (i = imin; i <= imax; i++) {
      if (dist_matrix[i][bestcodeword] <= 2*sqrt(bestdistortion)) {
	tempdouble = 0.0;
	for (j = 0; j < dimension; j++) {
	  temp = ( (double) tempvector[j]) - codebook[index[i]][j];
	  tempdouble += temp*temp;
	  if (tempdouble > bestdistortion) j = dimension; /* abort loop */
	}

	if (tempdouble < bestdistortion) {
	  bestdistortion = tempdouble;
	  bestcodeword = i;
	}

	/* if bestdistortion is 0 then the best codeword has been found */
	if (bestdistortion == 0.0) i = codebooksize;
      }
    } /* the best codeword has been found */

    count[index[bestcodeword]]++;
    celldistortion[index[bestcodeword]] += bestdistortion;
    distortion += bestdistortion;
    /* round the codeword */
    for (i = 0; i < dimension; i++) { /* for double data remove floor */
/* tempvector[i] = (DATA) (floor(codebook[bestcodeword][i] + 0.5));
   this in in error and is changed 9/4/97 */
      tempvector[i] = (DATA) (floor(codebook[index[bestcodeword]][i] + 0.5));
      /* tempvector[i] = (DATA) codebook[index[bestcodeword]][i]; */
    }
    /* write the data to the output file */
    if (fwrite(tempvector,sizeof(DATA),dimension,outputfile) != dimension) {
      fprintf(stderr,"%s: %s: %s\n",programname,outputname,NOWRITE);
      return(-1.0);
    }
  } /* all vectors have been encoded */

  return(distortion);
}
