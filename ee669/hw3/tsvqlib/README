To create the programs tsvq, tsvqe, prune, select , and voronoi_ts
type "make all" To remove unnecessary files, type "make clean"
To change the type of data that the programs will process, change the
definition of DATA in the file vq.h.


PROGRAM
    tsvq.c

SYNOPSIS
    tsvq -t trainingsequence -c codebook -s statistics
          -d dimension -r rate -m offset -h threshold -B

DESCRIPTION

    tsvq forms an unbalanced tree structured VQ codebook. Use the -B option to
    create a balanced tree structured VQ codebook.  The input should be one
    training sequence that has already been blocked.  Merge multiple training
    sequences into one file.  Blocking programs are in the stdvq directory.
    The output consists of two files.  One is a codebook file which has the
    format:
        TYPE       SIZE          DESCRIPTION
        long       1             number of nodes in the tree (numnodes)
        integer    1             vector dimension (dim)
        short      numnodes      tree description array
        DISTTYPE   numnodes*dim  codewords
    The tree description array is a preorder list where 0 indicates that
    a node is terminal and 1 indicates that a node is not terminal.  The
    second output file is also a preorder list with two values per tree
    node where 1 long is used for the count and 1 DISTTYPE is used for
    the distortion.

    This program should produce any tree from (rate 0, highest distortion)
    to (highest rate, 0 distortion), where the rate is the average number of
    bits per vector and the distortion the the average mean squared error per
    vector.  To do this, the lloyd subroutine will try two different methods
    to split a cell.  The first attempted split is to use the centroid and a
    slightly perturbed version of the centroid.  Should this fail, then a
    second attempt to split is made by using the centroid and the vector that
    is the furthest distance from the centroid. To create different trees,
    try playing with the mult_offset parameter.  The minimum number of training
    vectors that must be present in order for a codeword to be split is
    controlled by DEF_min_vectors in the tsvq.h file.  I do not recommend
    playing with the threshold parameter (defaulted to zero, but it can
    take values up to 1) since a non-zero threshold violates the local
    optimality condition that is assumed by the tree growing algorithm.
    This can lead to empty cells on the tree and wasted bits. See
    tsvq_util.c under lloyd for more details about the possible side
    effects.  This code does not try to correct any of the side effects
    caused by using a non-zero threshold, but it does detect empty cells
    and prints a warning to the user.  In addition, tsvq prints the following
    information to stdout:  rate and distortion as the tree grows, the number
    of nodes in the tree, and the empirical entropy of the codewords.
    Type "tsvq" to see default values.


PROGRAM
    tsvqe.c

SYNOPSIS
    tsvqe -c codebook -i input -o output -R -D

DESCRIPTION
    tsvqe encodes a blocked input file using a tree-structured VQ codebook
    created by tsvq. The inputs are the input image and the codebook.
    The output is the decoded image, not the codewords themselves.  There
    are two optional outputs.  An instantaneous rate image can be created
    by choosing the -R flag, and the counts and distortions can be written
    using the -D flag.  The counts and distortions file has the same format
    as the statistics file produced by tsvq.c.  In addition, tsvqe prints
    the following information to stdout: average rate per vector, average
    distortion per vector, entropy of the vectors, and maximum codeword
    length.  Type "tsvqe" to see default values.


PROGRAM
    prune.c

SYNOPSIS
    prune -c codebook -s statfile -o nested_subtree -E

DESCRIPTION
    prune is used to prune a codebook created by tsvq.c.  It uses the
    codebook file to find the tree structure, and the statistics file to
    find the count and average distortion for each node of the tree.
    See pages 683-684 of _Vector Quantization and Signal Compression_
    by Gersho & Gray for the pruning algorithm.  The -E option implements
    entropy constrained pruning.  The output of the program is a nested
    subtree or preorder list (like the codebook) with one long int used for
    each node. If the value is zero, it means that nothing was pruned at
    that node.  If the value is non-zero, then there was a pruning done at
    that node.  The number n stored indicates that that node was the nth
    pruning done. Prune prints to stdout for each subtree: the subtree
    number, the minimum lambda of that tree, the average rate and distortion,
    and the number of nodes in the tree.  Use the program "select" to choose
    a given subtree.  Type "prune" to see default values.

PROGRAM
    select.c

SYNOPSIS
    select -c codebook -s nested_subtree -n selection -o sub_codebook

DESCRIPTION
    To get tree number N, just remove all of the nodes that
    descend from any node that has  a label of 1,2,...N.
    select is used to create a pruned TSVQ codebook from the original
    TSVQ codebook.  The input of the program is the codebook file and
    the nested subtree file created by prune. The nested subtree file
    is a preorder list with one long int used for each node. If the value
    is zero, it means that nothing was pruned at that node.  If the value
    is non-zero, then a pruning is done at that node.  The number n stored
    indicates that the nth pruning pass was done at that node. To get
    tree number N, all of the nodes that descend from any
    node that has a label of 1,2,...N are pruned.


PROGRAM
    voronoi_ts.c

SYNOPSIS
    voronoi_ts -c codebook -o output -r rows -l columns

DESCRIPTION
    voronoi_ts takes a TSVQ codebook that has two dimensions and
    creates an output image that shows the edges of the voronoi
    regions.  The codewords are scaled to fit 90% of the region
    defined by the user as rows by columns.
