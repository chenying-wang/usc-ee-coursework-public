/******************************************************************************
 *
 * NAME
 *    tsvqe.c
 *    Jill R. Goldschneider
 *    February 1994
 *    Last Revision:
 *
 * SYNOPSIS
 *    tsvqe -c codebook -i inputfile -o outputfile -R -D
 *
 * DESCRIPTION
 *    This program should be used to encode a blocked raw image using a
 *    tree-structured VQ. The inputs are the input image and the codebook.
 *    The output is the decoded image.  There are two optional outputs.
 *    A rate image can be created by choosing the -R flag, and the counts
 *    and distortions can be written using the -D flag. The counts
 *    and distortions file has the same format as the statistics file
 *    produced by tsvq.c.  The format of the codebook is:
 *        TYPE       SIZE          DESCRIPTION
 *        long       1             number of nodes in the tree (numnodes)
 *        integer    1             vector dimension (dim)
 *        short      numnodes      tree description array
 *        DISTTYPE   numnodes*dim  codewords
 *    The format of the tree description array is that a 1 is a node that
 *    is not a terminal node and a 0 is a terminal node. The array is a
 *    preorder list.  In addition, tsvqe prints the following information
 *    to stdout: average rate per vector, average distortion per vector,
 *    entropy of the vectors, and maximum codeword length.  Type "tsvqe"
 *    to see default values.
 *
 * OPTIONS
 *    -c  codebook file                        DEF_codebookname
 *    -i  input image file                     DEF_inputname
 *    -o  output image file                    DEF_outputname
 *    -R  output a rate file                   DEF_ratename
 *    -D  output a stat file                   DEF_statname
 *
 * CALLS
 *    create_root(), construct_tree(), image_encode(), write_stat()
 *    empirical_entropy()
 *
 *****************************************************************************/
#include "tsvq.h"

FILE *inputfile;
FILE *outputfile;
FILE *ratefile;

char inputname[NAME_MAX];
char outputname[NAME_MAX];
char ratename[NAME_MAX];

char *programname;
int  dim;

BOOLEAN  printrate;

main(argc, argv)
     int argc;
     char *argv[];
{
  TreeNode  *root;      /* root of the codebook tree */
  char      *cp;        /* character pointer */
  char      option;     /* used for command line interpretation */
  char      codebookname[NAME_MAX];
  char      statname[NAME_MAX];
  FILE      *codebookfile;
  FILE      *statfile;
  long      numnodes;   /* number of nodes in the tree */
  long      bits;       /* total number of bits used to encode image */
  int       maxbits;    /* highest rate used in encoding image */
  long      numpixels;  /* the number of pixels encoded */
  DISTTYPE  rate,distortion; /* encoding rate and distortion (squared error) */
  double    entropy;    /* empirical entropy */
  BOOLEAN   statflag;   /* flag to create a statistics file */

  /* assign default values */
  strcpy(codebookname, DEF_codebookname);
  strcpy(inputname, DEF_inputname);
  strcpy(outputname, DEF_outputname);
  strcpy(ratename, DEF_ratename);
  strcpy(statname, DEF_statname);

  dim = DEF_dim;
  programname = *argv;
  printrate = FALSE;
  statflag = FALSE;

  /* if no options entered, list all of the defaults */
  if (argc == 1) {
    printf("%s %s %s\n",USAGE,programname,HOWTOUSE_TSVQE);
    printf("\nOPTIONS   DESCRIPTIONS                         DEFAULTS\n");
    printf("-c        codebook                             %s\n",codebookname);
    printf("-i        input image                          %s\n",inputname);
    printf("-o        output image                         %s\n",outputname);
    printf("-R        output a rate file\n");
    printf("-D        output a count and distortion file\n");
    printf("\n");
    fflush(stdout);
    exit(0);
  }

  /* read and interpret command line arguments */
  while (--argc && ++argv) {
    if (*argv[0]=='-' && strlen(*argv)==2) { /* each option has 1 letter */
      option = *++argv[0];
      if (option == 'R') {printrate = TRUE;} /* examine the flag */
      else if (option == 'D') {statflag = TRUE;} /* examine the flag */
      else if (--argc && ++argv) { /* examine the option */
        switch(option) { /* examine the option letter */
        case 'c':
          strncpy(codebookname,*argv,NAME_MAX);
          break;
        case 'i':
          strncpy(inputname,*argv,NAME_MAX);
          break;
        case 'o':
          strncpy(outputname,*argv,NAME_MAX);
          break;
        default:
          fprintf(stderr,"%s: %c: %s\n",programname,option,NOTOPTION);
          exit(1);
          break;
        }
      }
      else {
        fprintf(stderr,"%s %s %s\n",USAGE,programname,HOWTOUSE_TSVQE);
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
  if (strlen(codebookname) == 0 || strlen(inputname) == 0) {
    fprintf(stderr,"%s %s %s\n",USAGE,programname,HOWTOUSE_TSVQE);
    exit(5);
  }

  /* user entered an input name which is the same as the output name */
  if (strncmp(cp = codebookname,outputname,NAME_MAX) == 0 ||
      strncmp(cp = inputname,outputname,NAME_MAX) == 0) {
    fprintf(stderr,"%s: %s %s %s %s: %s\n",
	    programname,cp,AND,outputname,ARESAME,ABORT_TSVQE);
    exit(6);
  }

  /* user entered the same input names */
  if (strncmp(inputname,codebookname,NAME_MAX) == 0) {
    fprintf(stderr,"%s: %s %s %s %s: %s\n",
	    programname,inputname,AND,codebookname,ARESAME,ABORT_TSVQE);
    exit(7);
  }

  /* assign the default output names if necessary */
  if (strlen(outputname) == 0) {
    sprintf(outputname,"%s%s",inputname,DEF_APPEND_TSVQ);
  }

  /*  open the files */
  if(!(codebookfile = fopen(cp = codebookname,"r")) ||
     !(inputfile = fopen(cp = inputname,"r")) ||
     !(outputfile = fopen(cp = outputname, "w"))) {
    fprintf(stderr,"%s: %s: %s\n",programname,cp,NOTFOUND);
    exit(8);
  }

  if (printrate) {
    if(!(ratefile = fopen(cp = ratename,"w"))) {
      fprintf(stderr,"%s: %s: %s\n",programname,cp,NOTFOUND);
      exit(9);
    }
  }

  /* find the number of nodes */
  if (fread((char *) &numnodes,sizeof(long),1,codebookfile) != 1) {
    fprintf(stderr,"%s: %s: %s\n",programname,codebookname,NOREAD);
    exit(10);
  }

  /* find the dimension */
  if (fread((char *) &dim,sizeof(int),1,codebookfile) != 1) {
    fprintf(stderr,"%s: %s: %s\n",programname,codebookname,NOREAD);
    exit(11);
  }

  /* construct the tree */
  if(!(root = create_root())) {
    exit(12);
  }
  if(!construct_tree(root,numnodes,codebookfile,codebookname)) {
    exit(13);
  }

  fclose(codebookfile);

  /* encode the image */
  if((distortion = image_encode(root,&bits,&maxbits,&numpixels)) < 0) {
    exit(14);

  }

  if(numpixels == 0) {
    fprintf(stderr,"%s: %s: %s\n",programname,inputname,NODATA);
    exit(15);
  }

  /* write the counts and distortion if requested */
  if (statflag) {
    /* assign the default output names if necessary */
    if (strlen(statname) == 0) {
      sprintf(statname,"%s%s",outputname,DEF_APPEND_STAT);
    }
    if(!(statfile = fopen(cp = statname,"w"))) {
      fprintf(stderr,"%s: %s: %s\n",programname,cp,NOTFOUND);
      exit(16);
    }
    if(!(write_stat(root,numnodes,statfile,statname))) {
      exit(17);
    }
  }

  /* compute the distortion and rate */
  distortion /= ((DISTTYPE) numpixels / dim);
  rate = ((DISTTYPE) bits) / ((DISTTYPE) numpixels / dim);
  if( (entropy = empirical_entropy(root,numnodes)) < 0) {
    exit(18);
  }

  /* output statistics */
  printf("\n");
  printf("Codebook file:             %s\n", codebookname);
  printf("Vector dimension:          %d\n",dim);
  printf("Number of Nodes:           %ld\n",numnodes);
  printf("Image to encode:           %s\n", inputname);
  printf("Encoded file:              %s\n", outputname);
  printf("Number of pixels encoded:  %d\n",numpixels);
  printf("Average rate:              %f\n",rate);
  printf("Empirical entropy:         %lf\n",entropy);
  printf("Average distortion:        %f\n",distortion);
  printf("Maximum codeword length:   %d\n",maxbits);
  if(printrate) {
    printf("Rate file:                 %s\n",ratename);
  }
  if(statflag) {
    printf("Statistics file:           %s\n",statname);
  }
  printf("\n");

  fclose(inputfile);
  fclose(outputfile);
  exit(0);
}
