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
    for(int i = 1 ; i != n ; i++){
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
        unsigned char res[3];
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
            std::cout<<"compare"<<std::endl;
            if(another.is_end){
                return is_end == another.is_end;
            }
            return val == another.val; }
        bool operator!=(own_iterator another){
            if(another.is_end){
                return is_end != another.is_end;
            }
            return val != another.val; }
        T operator*(){ return val; }
        friend struct own_elements_container<T,allocator>;

    };

    using iterator = own_iterator;
    using own_allocator_type = allocator;
    using element_allocator_type = typename allocator::template rebind<own_iterator>::other;
    element_allocator_type element_allocator;
    iterator* first = nullptr;
    iterator* end_element = nullptr;
    iterator* last = nullptr;
    

    own_elements_container(){
        end_element = element_allocator.allocate(1);
        element_allocator.construct(end_element);
    }

    void add(T val){		
		iterator* new_element = element_allocator.allocate(1);				
		element_allocator.construct(new_element,val);
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
        std::vector<own_iterator*> iterators;
        own_iterator* it =first;
        while(it != nullptr){
            iterators.push_back( it );
            it = it->next;
        }
        for( auto it = iterators.rbegin() ; it != iterators.rend() ; ++it ){
            element_allocator.destroy( *it );
            element_allocator.deallocate(*it,1);
        }
        element_allocator.destroy(end_element);
        element_allocator.deallocate(end_element,1);
    }

    iterator begin(){
        return *first; }
    iterator end(){ return *end_element; }
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
    for( int i = 0 ; i != 10 ; ++i ){
        own_container_own_allocator.add(i);
    }

    for( auto element:  own_container_own_allocator)
    {
        std::cout<<element<<std::endl;
    }
}

