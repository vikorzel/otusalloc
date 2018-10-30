#include <iostream>
#include <map>
#include <memory>

template<typename T, size_t size>
struct map_smart_allocator {
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    int cntr = 0;
    T* p;
    template<typename U>
    struct rebind {
        using other = map_smart_allocator<U,size>;
    };

    T *allocate(std::size_t n){
        std::cout << __FUNCTION__ << "[n=" << n << "]" << " size=" << size << std::endl;
        if(p==nullptr){
            p = (T*)std::malloc( sizeof(T) * size );
        }
        if(!p){
            throw std::bad_alloc();
        }
        return reinterpret_cast<T*>(p+(cntr++));
    }

    void deallocate(T*, std::size_t){
        if(p != nullptr){
            std::free(p);
            p=nullptr;
        }
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
        explicit own_iterator(T value):val(value), is_end(false), next(nullptr) {  }
        own_iterator& operator++(){ if(next!=nullptr) return *next;
            is_end=true;
            return *this;
                              }
        bool operator==(own_iterator another) const{
            if(is_end){
                return is_end == another.is_end;
            }
            return val == another.val; }
        bool operator!=(own_iterator another){
            if(is_end){
                return is_end != another.is_end;
            }
            return val != another.val; }
        T operator*(){ return val; }
        friend void own_elements_container<T,allocator>::add(T);

    };

    using iterator = own_iterator;
    using own_allocator_type = allocator;
    using element_allocator_type = typename allocator::template rebind<iterator>::other;
    element_allocator_type element_allocator;
    std::shared_ptr<iterator> head_element;
    std::shared_ptr<iterator> last_element;
    iterator* tail_element;

    own_elements_container(){
        last_element.reset(element_allocator.allocate(1));
        element_allocator.construct(last_element.get());
    }

    void add(T val){
        if(!head_element){
         head_element.reset(element_allocator.allocate(1));
         element_allocator.construct(head_element.get(),val);
         return;
        }
        if(nullptr==tail_element){
            tail_element = element_allocator.allocate(1);
            element_allocator.construct(tail_element,val);
            head_element.get()->next=tail_element;
            return;
        }


    }

    iterator begin(){ return *(head_element.get()); }
    iterator end(){ return *(last_element.get()); }
    const iterator cbegin(){ return const_cast<T const&>(this->begin()); }
    const iterator cend(){ return const_cast<T const&>(this->end()); }

};


int main()
{

    own_elements_container<int,map_smart_allocator<int,10>> own_container{};
    std::cout<<typeid(own_container.element_allocator).name()<<std::endl;
    for(int i = 0 ; i != 10 ; ++i){
        own_container.add(i*2);
    }

    for( auto element : own_container ){
        std::cout<<element<<std::endl;

    }



    /*
    auto m_regular = std::map<int, int>();
    auto m_ownalloc = std::map<int,int, std::less<int>, map_smart_allocator<std::pair<const int, int>,10>>{};
    for(int i = 0 ; i != 10 ; i++){
        int fact_r = fact(i);
        m_ownalloc[i] = fact_r;
        m_regular[i] = fact_r;
    }
    for(int i = 0 ; i != 10 ; ++i){
        std::cout << i << " - " << m_ownalloc[i] << std::endl;
        std::cout << i << " - " << m_regular[i] << std::endl;
    }*/


}

