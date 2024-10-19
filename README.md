Created 10/18/2024

Bitonic sort is an in-place highly parallelizable sorting algorithm with O(nlog^2(n)) complexity.
***It requires that n be a power of 2. 
Practically, this limits its implementation, but one can work around this restriction by padding 
their array with elements either outside the range of their data so that those all get pushed to
one side of the array and the rest are contiguously sorted.

2 examples are provided in the main function of this file, one with a preprepared array, and 
the other using a randomly generated array.
\\
compile this code using the command (using whatever OpenMP compatible compiler works):
g++ -fopenmp [flags] -o bitonic_sort parallel_bitonic_sort.cpp

and run it with:
./bitonic_sort
