/******************************************************************************
 *
 * NAME
 *    stdvq.c
 *    J. R. Goldschneider 5/93
 *
 * MODIFICATIONS
 *    11/93 modified to display defaults values. The -s option was added so
 *    that three programs were no longer maintained.  Also a change made
 *    so that any time the distortion is found to be zero, the program
 *    exits gracefully. JRG
 *
 *    3/94 JRG changed codebook so it is smart. It now knows the codebook size
 *    and dimension.  The format is:
 *        TYPE       SIZE            DESCRIPTION
 *        long       1               number of codewords (size)
 *        integer    1               vector dimension (dimension)
 *        double     size*dimension  codewords
 *
 * SYNOPSIS
 *    stdvq -t f1 -c f2 -d dimension -f codebooksize -h threshold 
 *           -a addoffset -m muloffset -s speedup -W
 *
 * DESCRIPTION
 *    stdvq forms codebooks using the generalized lloyd algorithm.  It obtains
 *    user input, verifies the values, checks to see if the training file
 *    exists, and runs the GLA for codebooks of size 2^n n = 0,1,2,... 
 *    until the final size is reached. The final size can be any integer value.
 *    Each increase in the size of the codebook is done by splitting codewords 
 *    of the next smallest codebook, (perturbed versions of the old codewords).
 *    The GLA continues to run until the (percent) change in distortion
 *    is less than threshold. The GLA will abort if there are cells which 
 *    cannot be filled.  If there are empty cells, the lloyd iteration tries
 *    to split the most populous cells only, (individual cell distortion
 *    is not considered). There are three options to speed up the lloyd 
 *    algorithm. The first uses partial distortion for speedup.  The second two
 *    use constraints imposed by ordering the codewords.  See those programs
 *    for more detail.  All three versions use the mean square error as
 *    the metric.
 *
 * OPTIONS
 *    -t  training sequence file name (input)
 *    -c  codebook file name (output)
 *    -d  data vector dimension
 *    -f  final codebook size
 *    -h  convergence threshold
 *    -a  codeword split additive offset
 *    -m  codework split multiplicative offset
 *    -s  option to select type of constrained search for lloyd algorithm.
 *
 * FLAGS
 *    -W  write all codebooks of size power of two and final codebook size
 *
 *    See vq.h for definitions of default values
 *
 * CALLS
 *    lloyd(), splitcodewords(), writecodebook()
 *
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "vq.h"
#include "stdvq.def"
#include "stdvq.h"

extern double lloyd0();
extern double lloyd1();
extern double lloyd2();

int main(argc, argv) 
     int  argc;
     char *argv[];
   
{
  char   option;      /* used for command line interpretation */
  double **codebook;  /* codebook array */
  double (*method)(); /* pointer to functions lloyd0, lloyd1, and lloyd2 */
  double distortion;  /* ave. distortion between codebook and training set */
  long   i,j;         /* counters */
  int    speedup;     /* indicates which type of speedup to use */

  /* set default values */
  sprintf(trainingname,DEF_INNAME);
  sprintf(codebookname,DEF_OUTNAME);
  dimension = DEF_DIMENSION;
  codebooksize = DEF_CODEBOOKSIZE;
  threshold = DEF_THRESHOLD;
  offset_add = DEF_OFFSET_ADD;
  offset_mul = DEF_OFFSET_MUL;
  write_all_codebooks = FALSE;
  programname = *argv;
  speedup = DEF_SPEEDUP;

  /* if no options entered, list all of the defaults */
  if (argc == 1) {
    printf("%s %s %s\n",USAGE,programname,HOWTOUSE_STDVQ);
    printf("\nOPTIONS   DESCRIPTIONS                         DEFAULTS\n");
    printf("-t        training sequence                    %s\n",trainingname);
    printf("-c        codebook name                        %s\n",codebookname);
    printf("-d        vector dimension                     %d\n",dimension);
    printf("-f        codebook size                        %d\n",codebooksize);
    printf("-h        convergence threshold                %g\n",threshold);
    printf("-a        codeword split additive offset       %g\n",offset_add);
    printf("-m        codeword split multiplicative offset %g\n",offset_mul);
    printf("-W        write intermediate codebooks\n");
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
      if (option == 'W') {write_all_codebooks = TRUE;} /* examine the flag */
      else if (--argc && ++argv) { /* examine the option */
	switch(option) { /* examine the option letter */
	case 't':
	  strncpy(trainingname,*argv,NAME_MAX);
	  break;
	case 'c':
	  strncpy(codebookname,*argv,NAME_MAX);
	  break;
	case 'd':
	  sscanf(*argv,"%i",&dimension);
	  break;
	case 'f':
	  sscanf(*argv,"%d",&codebooksize);
	  break;
	case 'h':
	  sscanf(*argv,"%g",&threshold);
	  break;
	case 'a':
	  sscanf(*argv,"%g",&offset_add);
	  break;
	case 'm':
	  sscanf(*argv,"%g",&offset_mul);
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
	fprintf(stderr,"%s %s %s\n",USAGE,programname,HOWTOUSE_STDVQ);
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

  /* user did not enter an input file name */
  if (strlen(trainingname) == 0 ) { 
    fprintf(stderr,"%s %s %s\n",USAGE,programname,HOWTOUSE_STDVQ);
    exit(5);
  }
  
  /* user entered illegal value */
  if (dimension<=0 || codebooksize<=0 || threshold<=0 || 
      offset_add<=0 || offset_mul<=0){
    fprintf(stderr,"%s %s %s\n",USAGE,programname,HOWTOUSE_STDVQ);
    exit(6);
  }
  
  /* find speedup option */
  switch (speedup)
    {
    case 0:
      method = lloyd0;
      break;
    case 1:
      method = lloyd1;
      break;
    case 2:
      method = lloyd2;
      break;
    default:
      fprintf(stderr,"%s %s %s\n",USAGE,programname,HOWTOUSE_STDVQ);
      exit(7);
    }
  
  /* user entered an input name which is the same as the output name */
  if (strncmp(trainingname,codebookname,NAME_MAX) == 0) {
    fprintf(stderr,"%s: %s %s %s %s: %s\n",
	    programname,trainingname,AND,codebookname,ARESAME,ABORT_STDVQ);
    exit(8);
  }
  
  /* if a training file is given but not a codebook file, assign output name */
  if (strlen(codebookname) == 0) {
    sprintf(codebookname,"%s%s",trainingname,DEF_APPEND_CB);
  }
  
  /* display all of the user's input values */
  printf("\nOPTIONS   DESCRIPTIONS                         SETTINGS\n");
  printf("-t        training sequence                    %s\n",trainingname);
  printf("-c        codebook name                        %s\n",codebookname);
  printf("-d        vector dimension                     %d\n",dimension);
  printf("-f        codebook size                        %d\n",codebooksize);
  printf("-h        convergence threshold                %g\n",threshold);
  printf("-a        codeword split additive offset       %g\n",offset_add);
  printf("-m        codeword split multiplicative offset %g\n",offset_mul);
  printf("-s        constrained search choice            %d\n",speedup);
  if (write_all_codebooks) {
    printf("-W        write intermediate codebooks\n");
  }
  printf("\n");
  fflush(stdout);

  /* allocate memory for the codebook */
  if (!(codebook = (double **) calloc(codebooksize,sizeof(double *)))) {
    fprintf(stderr,"%s: %s\n",programname,NOMEMORY);
    exit(9);
  }

  /* allocate memory for the dimension of each codeword */ 
  for(i=0; i < codebooksize; i++) {
    if (!(codebook[i] = (double *) calloc(dimension,sizeof(double)))) {
      fprintf(stderr,"%s: %s\n",programname,NOMEMORY);
      exit(10);
    }
  }

  /* open the training file */
  if((trainingfile = fopen(trainingname,"r")) == NULL){
    fprintf(stderr,"%s: %s: %s\n",programname,trainingname,NOTFOUND);
    exit(11);
  }

  /* training file does not have any training vectors */
  rewind(trainingfile);
  if (!(fread(codebook[0], sizeof(DATA), dimension, trainingfile) == dimension)
      || feof(trainingfile) || ferror(trainingfile)) {
    fprintf(stderr,"%s: %s: %s\n",programname,trainingname,NOREAD);
    exit(12);
  }

  /* perform generalize lloyd algorithm on all codebook sizes */
  for(i = 1; i < codebooksize;) {

    /* run the GLA for codebook of size i */
    if ( (distortion = method(codebook,i)) < 0) {
      exit(13);
    }

    /* if distortion is zero, no need to continue. 
       note that lloyd can and will change codebooksize in such a case */
    if (distortion == 0) break;

    /* display the distortion of the training set to the codebook of size i */

    printf("%s %-7d:  %f\n",DISTORTION,i,distortion);
    fflush(stdout);

    /* write the codebook of size i if requested */
    if(write_all_codebooks) if(!writecodebook(codebook,i)) exit(14);


    /* find the number of new codewords that need to be made (j-i) */
    if ((j = 2*i) > codebooksize) j = codebooksize;

    /* split the codewords */
    splitcodewords(codebook,i,j,0);

    /* increment the codebook size */
    i = j;
  }

  /* it may be that distortion is 0, so we can exit early */
  if (distortion == 0) {
    printf("%s %-7d:  %f\n",DISTORTION,i,distortion);
    fflush(stdout);
    if(!writecodebook(codebook,codebooksize)) exit(15);
    fclose(trainingfile);
    exit(0);
  }

  /* do the final codebook */
  if ( (distortion = method(codebook,codebooksize)) < 0) exit(16);
  printf("%s %-7d:  %f\n",DISTORTION,codebooksize,distortion);
  fflush(stdout);
  if(!writecodebook(codebook,codebooksize)) exit(17);
  
  fclose(trainingfile);
  exit(0);
}
