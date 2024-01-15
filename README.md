# SIGHP
A hypergraph partitioner designed around the principles of information theory, capable of handling extremely large hypergraphs with up to billions of vertices using a single thread. The source code is implemented in C++.

## How to build
To build SIGHP, make sure you have CMake(>=3.16) and a C++17 compatible compiler, such as clang or gcc installed. Tested were GCC9.4 in Ubuntu 20.04.

Then run the following to build SIGHP:
```
$ git clone https://github.com/SIGHyperPartitioner/SIGHP
$ cd SIGHP/
$ mkdir build && cd build
$ cmake ..
$ make
```

## How to Use
After building project you can run SIGHP by:
```
./SIGHP -n <vertices-counts> -m <hyperedge-counts> -i {input-file} -p {partition-count} -sheild {hubedge-sheild} -save {scheme-save-path} -e {imbalance-factory}
```

|Parameter|Effect|
|---|---|
|-i|(nessesary) Path for input hypergraph dataset|
|-n|(nessesary) Number of vertices in the input dataset|
|-m|(nessesary) Number of hyperedge in the input dataset|
|-p|(optional) Number of partition blocks, default as 16|
|-sheild|(optional) Hubedge shielding ratio, default as 0.2|
|-save|(optional) Partition Scheme Save Path; If not set, no output will be generated|
|-e|(optional) Imbalance factory, default as 0|

So for example a valid command is:
```
./SIGHP -i ../data/out.github -n 56530 -m 120869 
```

## Input Formate
SIGHP takes a bipartite graph as the input format and constructs hyperedges through the edge list. Regard input file as :
```
vid eid
vid eid
...
vid eid
```
Ensure that vid is less than the input parameter n, and eid is less than the input parameter m.
## License

Specify the license information for your project here.
