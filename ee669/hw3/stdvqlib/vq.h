/******************************************************************************
 *
 * NAME
 *    vq.h
 *    J. R. Goldschneider 5/93
 *
 * DESCRIPTION
 *    Macros used in all of the programs block.c unblock.c stdvq.c stdvqe.c
 *    are defined here.
 *
 *****************************************************************************/

#define DATA             /* short int */ unsigned char
#define BOOLEAN          int
#define TRUE             1
#define FALSE            0

#define NAME_MAXIMUM     100
#define NAME_MAX         NAME_MAXIMUM - 8
#define DEF_INNAME       ""
#define DEF_OUTNAME      ""
#define DEF_APPEND_CB    ".CB"
#define DEF_APPEND_VQ    ".VQ"
#define DEF_APPEND_TS    ".TS"
#define DEF_APPEND_RS    ".RS"
#define DEF_APPEND_VOR   ".VOR"

/* defaults for the blocking programs */
#define DEF_ROWS           512
#define DEF_COLS           512
#define DEF_BLOCKWIDTH     4
#define DEF_BLOCKHEIGHT    4

/* defaults for the VQ programs */
#define DEF_DIMENSION    16
#define DEF_CODEBOOKSIZE 256
#define DEF_THRESHOLD    0.01
#define DEF_OFFSET_ADD   0.01
#define DEF_OFFSET_MUL   0.01
#define DEF_SPEEDUP      1
#define MAX_SPLIT_ATTEMPTS   10

#define USAGE            "Usage:"
#define HOWTOUSE_BLOCK   "-i input -o output -r rows -l columns -h blockheight -w blockwidth"
#define HOWTOUSE_STDVQ   "-t trainingsequence -c codebook -d dimension -f codebooksize \n     -h threshold -a addoffset -m multoffset -s speedup -W"
#define HOWTOUSE_STDVQE  "-c codebook -i input -o output -s speedup -D"
#define HOWTOUSE_VOR     "-c codebook -o output -r rows -l columns"
#define NOTFOUND         "not found"
#define NOTOPTION        "unknown option"
#define NOMEMORY         "insufficient memory"
#define NOREAD           "error reading data"
#define NOWRITE          "error writing data"
#define AND              "and"
#define ARESAME          "are identical"
#define ABORT_BLOCK      "image not blocked"
#define ABORT_UNBLOCK    "image not unblocked"
#define ABORT_STDVQ      "codebook not generated"
#define ABORT_STDVQE     "data not encoded"
#define DISTORTION       "Codebook distortion of size"
#define NOFILL           "Unable to fill"
#define TRYFILL          "Attempting to fill"
#define EMPTYCELLS       "empty cells of codebook"
#define EMPTYCELL        "empty cell of codebook"
#define STOP             "stopping with codebook"
#define OVERFLOWED       "distortion overflow"
