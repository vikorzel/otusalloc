// sandbox.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <map>
#include <memory>
#include <vector>

template<typename T, size_t size>
struct map_smart_allocator {
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    int cntr = 0;
    T* p = nullptr;
    template<typename U>
    struct rebind {
        using other = map_smart_allocator<U,size>;
    };

    T *allocate(std::size_t){
        if(p==nullptr){
            p = (T*)std::malloc( sizeof(T) * size );
        }
        if(!p){
            throw std::bad_alloc();
        }
        return reinterpret_cast<T*>(p+(cntr++));
    }

    void deallocate(T*, std::size_t){        
        return;
    }

    ~map_smart_allocator(){
        if(p!=nullptr) free(p);
    }

    template<typename U, typename ... Args>
    void construct(U* p, Args &&... args){        
        ::new((void *)p) U(std::forward<Args>(args)...);
    }

    void destroy(T* p){        
        p->~T();
    }
};

int fact(int n){
    if (n < 2) return 1;
    int res = 1;
    for(int i = 1 ; i <= n ; i++){
        res *= i;
    }
    return res;
}

template <typename T, typename allocator = std::allocator<T>>
struct own_elements_container{

    class own_iterator : public std::iterator<std::forward_iterator_tag,T, long, T*, T>
    {
    protected:
        T val;
        bool is_end;
        own_iterator* next;

    public:
        own_iterator():is_end(true){}
        explicit own_iterator(T value):
            val(value),
            is_end(false),
            next(nullptr){  }
        own_iterator& operator++(){ 
            if(next != nullptr){
                this->val = this->next->val;
                this->next = this->next->next;
            }else{
                is_end = true;
            }
			return *this;			
        }

        bool operator==(own_iterator another) const{
            if(another.is_end){
                return is_end == another.is_end;
            }
            return next == another.next; }
        bool operator!=(own_iterator another){
            if(another.is_end){
                return is_end != another.is_end;
            }
            return next == another.next; }
        T operator*(){ return val; }
        friend struct own_elements_container<T,allocator>;

    };

    using iterator = own_iterator;
    using own_allocator_type = allocator;
    using element_allocator_type = typename allocator::template rebind<own_iterator>::other;
    element_allocator_type element_allocator;
    iterator* first = nullptr;
    iterator* last = nullptr;
    

    own_elements_container(){

    }

    own_elements_container( const own_elements_container& another){
        first = another.first;
        last = another.last;  
        element_allocator = another.element_allocator;      
    }

    own_elements_container( own_elements_container&& another ){
        std::swap(first,another.first);
        std::swap(last,another.last);
        std::swap(element_allocator, another.element_allocator);
    }

    template <typename ... Args>
    void add(Args&& ... args){		
		iterator* new_element = element_allocator.allocate(1);				
		element_allocator.construct(new_element,std::forward<Args>(args)...);
		if(first == nullptr) first = new_element;
		if(last == nullptr){
			last = new_element;
		}else{
			last->next=new_element;
			last = last->next;
		}
    }

    ~own_elements_container(){
        if(first==nullptr) return;
        own_iterator* it =first;
        own_iterator* for_delete;
        while(it != nullptr){
            for_delete = it;
            element_allocator.destroy(for_delete);
            element_allocator.deallocate(for_delete,1);
            it = it->next;
        }
    }

    iterator begin(){
        return *first; }
    iterator end(){ auto end = *last;
                    end.is_end = true;
                    return end;
     }
    const iterator cbegin(){ return const_cast<own_iterator const&>(this->begin()); }
    const iterator cend(){ return const_cast<own_iterator const&>(this->end()); }

};


int main()
{
    std::map<int,int> std_container_std_allocator{};
    for(int i = 0 ; i != 10 ; ++i){
        std_container_std_allocator[i] = fact(i);
    }

    std::map<int,int,std::less<int>, map_smart_allocator<std::pair<const int, int>,10>> std_container_own_allocator{};
    for(int i = 0 ; i != 10 ; ++i){
        std_container_own_allocator[i]= fact(i);
    }

    for(auto& element: std_container_own_allocator){
        std::cout<<element.first << " " << element.second << std::endl;
    }

    own_elements_container<int> own_container_std_allocator{};
    for(int i = 0 ; i != 10 ; ++i ){
        own_container_std_allocator.add(i);
    }

    own_elements_container<int,map_smart_allocator<int,10>> own_container_own_allocator{};

    for( int i = 0 ; i != 10 ; ++i ){ own_container_own_allocator.add(i); }
    for( auto element: own_container_own_allocator) {
        std::cout<<element<<std::endl;
    }    
}

