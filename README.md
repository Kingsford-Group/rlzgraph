## RLZ-Graph: Constructing smaller genomes via string compression
This repository contains the RLZ-Graph software package described in the manuscript "Constructing smaller genome graph via string compression".

### Installation

#### Dependencies
This software package requires the following libraries:
1. Gurobi 
2. SDSL-lite (https://github.com/simongog/sdsl-lite)

After installing the required libraries, modify the first 5 lines of the Makefile in order to locate the libraries.
Then, run `make` to compile the program.

### Running RLZ-Graph

RLZ-Graph constructs a genome graph by first compressing the input sequences using the relative Lempel-Ziv algorithm. The input to RLZ-Graph is a set of complete genomic sequences stored in `.fasta` format.

```
-------------------------------------------------
|  USAGE: rlzgraph [OPTIONS] -i <input.fasta>   |
-------------------------------------------------
Required Arguments:
        -i      <input.fasta>       Fasta file containing complete genomic sequences. Each sequence will be treated as one complete genome.
                                    If reference fasta is missing, the first sequence of the input fasta file will be used as reference.
Optional Arguments:
        -ii     <ref idx>           The index of the reference sequence in input.fasta
        -n      <num seq>           Number of sequences to use in input.fasta
        -r      <ref.fasta>         Fasta file containing one complete genomic sequence that will be used as the reference
        -p      <out.phrases>       Output file that will contain all unique phrases. Results from different heuristics will be stored in "out.phrases_[heuristic]"
        -g      <out.gfa>         Output file that will contain the graph structure and the paths. This file will be in .gfa format
        -s      <out.sources>       Output file that will contain all sources.
        -c      <out.compressed>    Output file that will contain strings of phrases for each input sequence.
```

## Updated repository with input streaming implementation
Instead of loading the entire input string in memory as in the first version, the current implementation streams the input string during RLZ factorization. This change reduces the running time and the memory usage of the program. The updated running time and memory usage of RLZ-Graph on human chromosome 1 sequences are shown in the following table. Note that this is different from the results presented in the original paper.

The commit that records the first stable version of the program (described in the original paper) can be found in [this commit](https://github.com/Kingsford-Group/rlzgraph/commit/ef7f26c9d36dd9c3b8ba5f6e75c09d2a23d56dca).

|Number of sequences | Wall-clock running time (s) | Reident set size (RSS) (MB)|
|--------------------|-----------------------------|----------------------------|
|5                   | 776                         | 7373                       |
|25                  | 2498                        | 7374                       |
|50                  | 4647                        | 7374                       |
|75                  | 6755                        | 8091                       |
|100                 | 8873                        | 9993                       |


