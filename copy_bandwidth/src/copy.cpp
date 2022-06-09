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

template <typename Engine, class IterType>
static void fill_random(Engine &gen, IterType begin, IterType end)
{
    std::uniform_real_distribution<double> dist;

    const size_t n_elem = std::distance(begin, end);

    #pragma omp parallel for schedule(guided)
    for(size_t i = 0; i < n_elem; ++i){
        // begin[i] = int(dist(gen) * 10);
        *(begin+i) = i;
    }
}

template <class Iter1, class OutIter>
void simple_cp(Iter1 inBegin, Iter1 inEnd, OutIter outBegin){
    size_t len = std::distance(inBegin, inEnd);
    #pragma omp parallel for schedule(guided)
	for (size_t i = 0; i < len; ++i)
	{
		*(outBegin + i) = *(inBegin + i);
	}
}

using ValType = float;

int main(int argc, char *argv[])
{
    int iterations = 20;
    std ::random_device rd;  // get a seed for the rng
    std ::mt19937 gen(rd()); // seed mersenne twister engine

    int size = 0;
    chCommandLineGet<int>(&size, "s", argc, argv);

    // std::vector<ValType> in_vec(size, 0);
    // std::vector<ValType> out_vec(size, 0);
    std::vector<ValType, default_init_allocator<ValType>> in_vec(size);
    std::vector<ValType, default_init_allocator<ValType>> out_vec(size);
    
    fill_random(gen, in_vec.begin(), in_vec.end());
    fill_random(gen, out_vec.begin(), out_vec.end());

    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < iterations; i++)
    {
        simple_cp(in_vec.begin(), in_vec.end(), out_vec.begin());
    }
    
    auto stop = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::milli> diff = stop - start;
    double exec_time = diff.count();
    exec_time = exec_time/iterations;

    size_t bytes = sizeof(ValType)*size*2;
    double Mbytes = static_cast<double>(bytes)/(1024*1024);

    std::cout << "Copying " << Mbytes << " MB took " << exec_time << "ms\n";
    std::cout << bytes/(exec_time*1000) << "MB/s\n";
    return 0;
}