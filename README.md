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



