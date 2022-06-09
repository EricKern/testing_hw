#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <omp.h>
#include "chCommandLine.h"

template <typename T, typename A = std::allocator<T>>
class default_init_allocator : public A {
	// Implementation taken from https://stackoverflow.com/a/21028912.
   public:
	using A::A;

	template <typename U>
	struct rebind {
		using other = default_init_allocator<
		    U,
		    typename std::allocator_traits<A>::template rebind_alloc<U>>;
	};

	template <typename U>
	void construct(U* ptr) noexcept(
	    std::is_nothrow_default_constructible<U>::value) {
		::new (static_cast<void*>(ptr)) U;
	}
	template <typename U, typename... ArgsT>
	void construct(U* ptr, ArgsT&&... args) {
		std::allocator_traits<A>::construct(
		    static_cast<A&>(*this), ptr, std::forward<ArgsT>(args)...);
	}
};

// template <typename Engine, class IterType>
// static void fill_random(Engine &gen, IterType begin, IterType end)
// {
//     std::uniform_real_distribution<double> dist;

//     const size_t n_elem = std::distance(begin, end);

//     #pragma omp parallel for schedule(runtime)
//     for(size_t i = 0; i < n_elem; ++i){
//         // begin[i] = int(dist(gen) * 10);
//         *(begin+i) = i;
//     }
// }

template <typename Container>
void matMul_init(Container &in_mat,
                Container &in_vec,
                Container &out_vec){
    // expect a row-major matrix
    const size_t inMat_cols = in_vec.size();
    const size_t inMat_rows = in_mat.size() / inMat_cols;

    #pragma omp parallel for schedule(runtime)
    for (size_t y = 0; y < inMat_rows; ++y) // over rows
    {
        for (size_t x = 0; x < inMat_cols; ++x) // over cols + inner product
        {
            in_mat[y * inMat_cols + x] = y * inMat_cols + x;    // correct first touch
        }
        in_vec[y] = y;  // wrong first touch but has to be shared
        out_vec[y] = 0; // correct first touch
    }
}

template <typename Container>
// expect out_vec to be initialized to 0 values
void matMul(Container &in_mat,
            Container &in_vec,
            Container &out_vec){
    // expect a row-major matrix
    const size_t inMat_cols = in_vec.size();
    const size_t inMat_rows = in_mat.size() / inMat_cols;

    #pragma omp parallel for schedule(runtime)
    for (size_t y = 0; y < inMat_rows; ++y) // over rows
    {
        for (size_t x = 0; x < inMat_cols; ++x) // over cols + inner product
        {
            out_vec[y] += in_mat[y * inMat_cols + x] * in_vec[x];
        }
    }
}

using ValType = double;

int main(int argc, char *argv[])
{
    std ::random_device rd;  // get a seed for the rng
    std ::mt19937 gen(rd()); // seed mersenne twister engine

    // parse comandline arguments
    int size_m = 1000;
    chCommandLineGet<int>(&size_m, "m", argc, argv);
    int size_n = 1000;
    chCommandLineGet<int>(&size_n, "n", argc, argv);

    std::vector<ValType, default_init_allocator<ValType>> in_mat(size_m * size_n);
    std::vector<ValType, default_init_allocator<ValType>> in_vec(size_n);
    std::vector<ValType, default_init_allocator<ValType>> out_vec(size_m);
    
    matMul_init(in_mat, in_vec, out_vec);

    auto start = std::chrono::steady_clock::now();

    matMul(in_mat, in_vec, out_vec);

    
    auto stop = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::milli> diff = stop - start;
    double exec_time = diff.count();
    exec_time = exec_time;

    size_t bytes = sizeof(ValType)*(size_m * size_n + size_n + size_m);
    double Mbytes = static_cast<double>(bytes)/(1024*1024);

    size_t flops = (2*size_n-1)*size_m;
    double Gflops = static_cast<double>(flops)/(1000*1000*1000);

    std::cout << "Copying " << Mbytes << " MB took " << exec_time << "ms\n";
    std::cout << Mbytes/(exec_time/1000) << "MB/s\n";
    std::cout << Gflops/(exec_time/1000) << "GFLOPS/s\n";
    return 0;
}