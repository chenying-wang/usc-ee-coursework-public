To create the programs block, unblock, stdvq, stdvqe , and voronoi_fs
type "make all" To remove unnecessary files, type "make clean"
To change the type of data that the programs will process, change the
definition of DATA in the file vq.h.


PROGRAM
    block.c

SYNOPSIS
    block -i inputfile -o outputfile -r rows -l columns 
          -h blockheight -w blockwidth

DESCRIPTION
    block takes a raw image inputfile and creates outputfile
    which is a list of vectors where each vector is a block 
    from inputfile. The inputfile has dimensions rows and columns,
    and the block has dimensions blockheight and blockwidth.
    If no outputfile is named, then the name of the outputfile defaults 
    to inputfile.TS.  Type "block" to see default values


PROGRAM
    unblock.c

SYNOPSIS
    unblock -i inputfile -o outputfile -r rows -l columns 
	    -h blockheight -w blockwidth

DESCRIPTION
    block takes a blocked image inputfile and creates outputfile
    which is a raw image made from the blocks of inputfile.
    The inputfile has dimensions rows and columns,
    and the block has dimensions blockheight and blockwidth.
    If no outputfile is named, then the name of the outputfile defaults 
    to inputfile.RS. Type "unblock" to see default values.


PROGRAM
    stdvq.c

SYNOPSIS
    stdvq -t trainingsequence -c codebook -d dimension -f codebooksize 
          -h threshold -a addoffset -m muloffset -s speedup -W

DESCRIPTION
    stdvq forms codebooks using the generalized lloyd algorithm (GLA).
    It runs the GLA for codebooks of size 2^n n = 0,1,2,... 
    until the final size is reached. The final size can be any integer value.
    Each increase in the size of the codebook is done by splitting codewords 
    from the next smallest codebook, (perturbed versions of the old codewords).
    The GLA continues to run until the change in distortion
    is less than threshold. The GLA will abort if there are cells which 
    cannot be filled.  If there are empty cells, the lloyd iteration tries
    to split the most populous cells only, (individual cell distortion
    is not considered). There are three options to speed up the lloyd 
    algorithm. The first uses partial distortion for speedup.  The second two
    use constraints imposed by ordering the codewords.  See Huang, Bi, Stiles,
    and Harris in the IEEE Transactions on Image Processing July 1992
    for more detail.  All three versions use the mean square error.
    There is one flag (-W). If the flag is not specified then only the final 
    codebook is written. If the flag is specified, then all intermediary 
    codebooks are written as well.  Each codebook has the following format:
        TYPE       SIZE            DESCRIPTION
        long       1               number of codewords (size)
        integer    1               vector dimension (dimension)
        double     size*dimension  codewords
    Type "stdvq" to see default values.


PROGRAM
    stdvqe.c

SYNOPSIS
    stdvqe -c codebook -i inputfile -o outputfile -s speedup -D

DESCRIPTION
    stdvqe encodes the inputfile using the codeword from the codebook that 
    yields the lowest mean squared error.  The reproduction file is placed 
    in outputfile. There are three options to speed up the search.  
    The first uses partial distortion for speedup. The second two use 
    constraints imposed by ordering the codewords. See  Huang, Bi, Stiles, 
    and Harris in the IEEE Transactions on Image Processing July 1992 for 
    more details.  There is one flag (-D).  If the flag is specified, the 
    number of vectors in each cell and the cell distortion is reported. 
    Type "stdvqe" to see default values.


PROGRAM
    voronoi_fs.c

SYNOPSIS
    voronoi_fs -c codebook -o output -r rows -l columns 

DESCRIPTION
    voronoi_fs takes a full search codebook that has two dimensions and
    creates an output image that shows the edges of the voronoi
    regions.  The codewords are scaled to fit 90% of the region
    defined by the user as rows by columns.
    
    

