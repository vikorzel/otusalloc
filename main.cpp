// Example program
#include <iostream>
#include <map>

template<typename T, size_t size>
struct map_smart_allocator {
	using value_type = T;
	using pointer = T*;
	using const_pointer = const T*;
	using reference = T&;
	using const_reference = const T&;
    int cntr = 0;    

	template<typename U>
	struct rebind {
		using other = map_smart_allocator<U,size>;
	};

	T *allocate(std::size_t n){
		std::cout << __FUNCTION__ << "[n=" << n << "]" << " size=" << size << std::endl;		
		static T* p;
		if(p==nullptr){		    
		    p = (T*)std::malloc( sizeof(T) * size );
		}
		if(!p){
			throw std::bad_alloc();
		}
		return reinterpret_cast<T*>(p+(cntr++));
	}

	void deallocate(T* p, std::size_t n){
		std::free(p);
	}

	template<typename U, typename ... Args>
	void construct(U* p, Args &&... args){
		std::cout<<__FUNCTION__<<std::endl;
		::new((void*)p) U(std::forward<Args>(args)...);
	}

	void destroy(T* p){
		std::cout<<__FUNCTION__<<std::endl;
		p->~T();
	}
};

int fact(int n){
    if (n < 2) return 1;
    int res = 1;
    for(int i = 1 ; i != n ; i++){
        res *= i;
    }
    return res;
} 


int main()
{
	auto m = std::map<int,int, std::less<int>, map_smart_allocator<std::pair<const int, int>,10>>{};
	for(int i = 0 ; i != 10 ; i++){
		m[i] = fact(i);
	}

}
 
