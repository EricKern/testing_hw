#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <omp.h>
#include "chCommandLine.h"

template <typename Engine, class IterType>
static void fill_random(Engine &gen, IterType begin, IterType end)
{
    std::uniform_real_distribution<double> dist;

    const size_t n_elem = std::distance(begin, end);

    #pragma omp parallel for schedule(static)
    for(size_t i = 0; i < n_elem; ++i){
        // begin[i] = int(dist(gen) * 10);
        begin[i] = i;
    }
}

template <class Iter1, class OutIter>
void simple_cp(Iter1 inBegin, Iter1 inEnd, OutIter outBegin){
    size_t len = std::distance(inBegin, inEnd);
    #pragma omp parallel for schedule(static)
	for (size_t i = 0; i < len; ++i)
	{
		*(outBegin + i) = *(inBegin + i);
	}
}

using ValType = float;

int main(int argc, char *argv[])
{
    omp_set_num_threads(4);
    std ::random_device rd;  // get a seed for the rng
    std ::mt19937 gen(rd()); // seed mersenne twister engine

    int size = 0;
    chCommandLineGet<int>(&size, "s", argc, argv);

    std::vector<ValType> in_vec(size, 0);
    std::vector<ValType> out_vec(size, 0);
    fill_random(gen, in_vec.begin(), in_vec.end());
    fill_random(gen, out_vec.begin(), out_vec.end());

    auto start = std::chrono::steady_clock::now();

    simple_cp(in_vec.begin(), in_vec.end(), out_vec.begin());

    auto stop = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::milli> diff = stop - start;
    double exec_time = diff.count();

    size_t bytes = sizeof(ValType)*size*2;
    double Gbytes = static_cast<double>(bytes)/(1024*1024*1024);

    std::cout << "Copying " << Gbytes << " GB took " << exec_time << "ms\n";
    std::cout << bytes/(exec_time*1000000) << "GB/s\n";
    return 0;
}