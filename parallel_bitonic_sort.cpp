// idea for the pseudocode influenced by https://www.youtube.com/watch?v=uEfieI0MumY

#include <iostream>
#include <algorithm>
#include <math.h>
#include <omp.h>
#include <random>
#include <chrono>

using namespace std;

void bitonic_split(int* data, size_t chunk_start, size_t chunk_size, int direction);
void bitonic_merge(int* data, size_t chunk_start, size_t chunk_size, int direction);
void bitonic_driver(int* data, size_t n, size_t p);
int* generate_random_array(size_t n);
void weak_scaling();
void print_data(int* data, size_t n);
void strong_scaling();


int main() {
    // NOTE: n MUST be a power of 2 for bitonic sort to work
    
    // EXAMPLE 1: 8 element array
    size_t n = 8;
    int data[n] = {3,6,5,7,4,1,8,2};
    int p = 8;
    bitonic_driver(data,n,p);
    print_data(data, n);

    // // EXAMPLE 2: random array
    // size_t n = pow(2,4);
    // int p = 8;
    // int* data = generate_random_array(n);

    // print_data(data, n);
    // auto start = chrono::high_resolution_clock::now();

    // bitonic_driver(data,n,p);

    // auto end = chrono::high_resolution_clock::now();
    // float duration = chrono::duration<float>(end - start).count();
    // print_data(data, n);

    // cout << "process took " << duration << " seconds" << endl;

    // delete [] data;

    return 0;
}


// returns a random array of size n. 
// array is on heap memory, so be sure to delete when done to avoid memory leaks.
int* generate_random_array(size_t n) {
    static thread_local mt19937 generator(random_device{}());
    uniform_int_distribution<int> distribution(-1e6,1e6);
    int *data = new int[n];

    for (size_t i = 0; i < n; i++) {
        data[i] = distribution(generator);
    }
    return data;
}

// driver program to do a bitonic sort given number of nodes and processors
// n is the size of the array. p is the desired number of processors
void bitonic_driver(int* data, size_t n, size_t p) {
    omp_set_num_threads(p); 

    #pragma omp parallel
    {
        #pragma omp single
        {
            bitonic_split(data, 0, n, 1);
        }
    }
}

void weak_scaling() {
    int p_start = 1;
    size_t n_start = 1024*8;

    // keep the ratio of work to processors the same and check duration
    for (int i = 1; i < 33; i*=2) {
        size_t n = n_start * i;
        size_t p = i;

        // create the dataset
        int *data = generate_random_array(n);

        auto start = chrono::high_resolution_clock::now();
        bitonic_driver(data, n, p);
        auto end = chrono::high_resolution_clock::now();
        float duration = chrono::duration<float>(end - start).count();

        cout << "(n,p) = (" << n << "," << p << ") took " << duration << " seconds." << endl;
        delete [] data;
    }
}

void strong_scaling() {
    size_t n = 1024*2024;

    // keep the data the seem but increase the number of processors each time
    for (int i = 1; i < 33; i*=2) {
        // redefine data every time because the driver ends up sorting in place
        int *data = generate_random_array(n);
        size_t p = i;

        auto start = chrono::high_resolution_clock::now();
        bitonic_driver(data, n, p);
        auto end = chrono::high_resolution_clock::now();
        float duration = chrono::duration<float>(end - start).count();

        cout << "(n,p) = (" << n << "," << p << ") took " << duration << " seconds." << endl;
        delete [] data;
    }
}

void print_data(int* data, size_t n) {
    cout << "[";
    for (size_t i = 0; i < n; i++) {
        cout << data[i] << ", ";
    }
    cout << "]\n";
}

// direction of 1 means sort up. direction of 0 means sort down
void bitonic_split(int* data, size_t chunk_start, size_t chunk_size, int direction) {
    if (chunk_size > 1) {
        #pragma omp task shared(data)
        bitonic_split(data, chunk_start, chunk_size / 2, 1);

        #pragma omp task shared(data)
        bitonic_split(data, chunk_start + chunk_size / 2, chunk_size / 2, 0);

        #pragma omp taskwait

        bitonic_merge(data, chunk_start, chunk_size, direction);
    }
}

// direction of 1 means sort up. direction of 0 means sort down
void bitonic_merge(int* data, size_t chunk_start, size_t chunk_size, int direction) {
    // if not, do nothing cuz 1 element array is automatically sorted
    if (chunk_size > 1) {
        int temp_swap;
        size_t k = chunk_size / 2;

        #pragma omp parallel for shared(data)
        for (size_t i = chunk_start; i < chunk_start + k; i++) {
            // make upper list have biggest values
            if (direction == 1 and data[i] > data[i + k]) {
                temp_swap = data[i];
                data[i] = data[i + k];
                data[i + k] = temp_swap;
            }
            // make lower list have biggest values
            else if (direction == 0 and data[i] < data[i + k]) {
                temp_swap = data[i];
                data[i] = data[i + k];
                data[i + k] = temp_swap;
            }
        }

        // left and right are now strictly have smaller or bigger elements than each other.
        // call merge on each of them to ensure that each half is in sorted order
        bitonic_merge(data, chunk_start, k, direction);
        bitonic_merge(data, chunk_start + k, k, direction);
    }
}