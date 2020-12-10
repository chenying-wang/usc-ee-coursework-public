/******************************************************************************
 *
 * NAME
 *    tsvq.h
 *    J. R. Goldschneider 2/94
 *
 * DESCRIPTION
 *    Macros used in the programs tsvq and tsvqe and prune and select.
 *
 *****************************************************************************/
#include <stdio.h>
#include <math.h>

#define DATATYPE         unsigned char
#define DISTTYPE         double
#define BOOLEAN          int
#define TRUE             1
#define FALSE            0
#define NAME_MAX         100

/* defaults for tsvq and tsvqe */
#define DEF_dim          4
#define DEF_mult_offset  .01
#define DEF_thresh       0.0
#define DEF_rate         0.0
#define DEF_min_vectors  1 /* the smallest value is 1 */

/* defaults for prune */
#define NODE_DATA_SIZE   4
#define DELTA_D          0
#define DELTA_R          1
#define LAMBDA           2
#define LAMBDA_MIN       3

/* defaults for voronoi_ts */
#define DEF_rows         512
#define DEF_cols         512

#define USAGE            "Usage:"
#define HOWTOUSE_TSVQ    "-t trainingsequence -c codebook -s statfile\n   -d dimension -r rate -m offset -h threshold -B"
#define HOWTOUSE_TSVQE   "-c codebook -i input -o output -R -D"
#define HOWTOUSE_PRUNE   "-c codebook -s statfile -o nested_subtree -E"
#define HOWTOUSE_SELECT  "-c codebook -s nested_subtree -n selection -o sub_codebook"
#define HOWTOUSE_VORONOI "-c codebook -o output -r rows -l columns"

#define NOTFOUND         "not found"
#define NOTOPTION        "unknown option"
#define NOMEMORY         "insufficient memory"
#define NOREAD           "error reading data"
#define NOWRITE          "error writing data"
#define NOTREE           "tree structure error"
#define NODATA           "insufficient data"
#define AND              "and"
#define ARESAME          "are identical"

#define ABORT_TSVQ       "codebook not generated"
#define ABORT_TSVQE      "image not encoded"
#define ABORT_PRUNE      "nested tree file not generated"
#define ABORT_SELECT     "sub-codebook file not generated"
#define ABORT_VORONOI    "voronoi diagram not generated"
#define HALT_TSVQ        "unable to extend codebook tree"
#define EMPTY_CELL       "empty cell may be added to codebook tree"
#define SMALL_THRESH     "use a smaller threshold"

#define DEF_APPEND_CDBK  ".cdbk"
#define DEF_APPEND_STAT  ".stat"
#define DEF_APPEND_TSVQ  ".tsvq"
#define DEF_APPEND_PRUNE ".nest"
#define DEF_APPEND_VOR   ".vor"

#define DEF_trsqname     ""
#define DEF_codebookname ""
#define DEF_statname     ""
#define DEF_inputname    ""
#define DEF_outputname   ""
#define DEF_subtreename  ""
#define DEF_ratename     "rate.dat"

/******************************************************************************
 *
 *    definitions of structures and function calls
 *
 *****************************************************************************/

typedef struct tree_struct
{
  struct tree_struct  *left_child;
  struct tree_struct  *right_child;
  struct tree_struct  *parent;
  DISTTYPE            *data;
  long                count;
  DISTTYPE            avmse;
  BOOLEAN             designed;
  int                 depth;
  DATATYPE            **trainseq;
} TreeNode;

typedef struct slope_struct
{
  struct slope_struct *previous;
  struct slope_struct *next;
  TreeNode            *node;
  DISTTYPE            slope;
} SlopeList;

/* node_util.c */
extern TreeNode  *newnode();
extern TreeNode  *newchild();
extern TreeNode  *create_root();

/* tsvq_util.c */
extern BOOLEAN   initialize_tree();
extern BOOLEAN   lloyd();
extern long      tree_clean();

/* slope_util.c */
extern void      update_rate();
extern BOOLEAN   conditional_insert();
extern BOOLEAN   forced_insert();
extern BOOLEAN   initialize_slopelist();
extern void      delete_slopelist();
extern SlopeList *find_maxslope();
extern SlopeList *find_oldest_entry();

/* tsvqe_util.c */
extern DISTTYPE  image_encode();
extern DISTTYPE  dist();
extern double    empirical_entropy();

/* read_util.c */
extern BOOLEAN   construct_tree();
extern BOOLEAN   construct_stat_tree();
extern BOOLEAN   construct_subtree();

/* write_util.c */
extern BOOLEAN   write_codebook();
extern BOOLEAN   write_stat();
extern BOOLEAN   write_nested_subtree();

/* prune_util.c */
extern void      initialize_stat_tree();
extern void      entropy_init_stat_tree();
extern void      entropy_init();
extern void      clear_count();
extern TreeNode  *find_min_slope();
extern void      update_tree();
extern void      prune();
extern long      count_nodes();
extern long      pruned_count();

/* voronoi_util.c */
extern void     label_leaf_nodes();
extern void     normalize_tree();
extern BOOLEAN  voronoi_diagram();
