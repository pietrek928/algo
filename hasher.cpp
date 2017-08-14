/*
 *  by Piotr Jarosz<pietrek.j@gmail.com>
 * */

#include <cstdio>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <stdint.h>
#include <unordered_set>
#include <typeinfo>

#define WHILEs1( a... ) while ( (a) );
#define WHILEs2( a... ) while ( (a) && (a) );
#define WHILEs3( a... ) while ( (a) && (a) && (a) );
#define WHILEs4( a... ) while ( (a) && (a) && (a) && (a) );

using namespace std;

typedef uint32_t uint;

template<class T>
inline T *obj_malloc( uint sz ) {
    auto r = malloc( sz * sizeof(T) );
    return *(T**)&r;
}

template<class T>
inline T *obj_realloc( T *A, uint sz ) {
    auto r = realloc( A, sz * sizeof(T) );
    return *(T**)&r;
}

template<class T>
inline void obj_memzero( T *A, uint sz ) {
    memset( A, 0, sz * sizeof(T) ); // TODO: faster one from dsp module ?
}

/*template<class T>
class allocator { // TODO: varying size element allocation - eg. string
    void *t, *p, *e;
    void *add( T &v ) {
        auto n = v.size();
        return v.cp( p, n );
    }
}*/

template<typename T>
class hasher {
    struct E {
        T v;
        E *nxt;
        inline operator T() {return v;}
    } *tab, **enter_tab;
    struct Ei {
        E *i;
        inline Ei( E **v ) {
            i = *v;
        }
        inline auto operator++() {
            i = i->nxt;
            return *this;
        }
        inline auto &operator*() {
            return i->v;
        }
        inline operator bool() {
            return i;
        }
    };
    struct Ep {
        E *i;
        E **_i;
        inline Ep( E **v ) {
            _i = v;
            i = *_i;
        }
        inline auto &operator++() {
            _i = & i->nxt;
            i = *_i;
            return *this;
        }
        inline auto &operator*() {
            return i->v;
        }
        inline operator bool() {
            return i;
        }
    };
    uint mod;
    uint sz;
    uint n;
    public:
    
    template<class T2>
    inline T2 _find( decltype(T::key) k ) {
        T2 i = enter_tab + k.hash( mod );
        if ( i ) {
            while ( (*i).key != k && ++i );
        }
        return i;
    }

    inline void _resize( uint _sz, uint _mod ) {
        mod = _mod;
        sz = _sz;
        tab = obj_realloc( tab, sz );
        enter_tab = obj_realloc( enter_tab, mod );
        obj_memzero( enter_tab, mod );
        auto _e = tab+n;
        auto i = tab;
        if ( i < _e )
        do {
            i->nxt = NULL;
            auto p = _find<Ep>( i->v.key );
            *(p._i) = i;
        } while ( ++i < _e  );
    }

    __attribute__(( noinline ))
    void inc_size() {
        _resize( sz*2, mod*2+1 );
    }

    class _stat {
        T *tab;
        T **enter_tab;
        uint mod;
        uint n;

        template<class T2>
        inline void _init( T2 *_tab, T2 **_enter_tab ) {
            tab = obj_malloc<T>( n ); // TODO: single allocation ?
            enter_tab = obj_malloc<T*>( mod+1 );

            auto et = enter_tab;
            auto wsk = tab;
            auto _e = _enter_tab+mod;
            auto i = _enter_tab;
            do {
                Ei j = i;
                *(et++) = wsk;
                if ( j )
                do {
                    *(wsk++) = *j;
                } while ( ++j );
            } while ( ++i < _e );
            *et = wsk;
            tab = obj_realloc( tab, wsk-tab );
        }

        public:

        template<class Ts>
        inline _stat( Ts &s )
            :n(s.n), mod(s.mod) {
            _init( s.tab, s.enter_tab );
        }

        inline auto _find( typename T::key_t k ) {
            auto p = enter_tab + k.hash( mod );
            union {
                T *i;
                bool f;
            } r;
            r.i = p[0];
            auto e = p[1];
            if ( r.i < e ) {
                do {
                    if ( r.i->key == k ) {
                        r.f = 1;
                        return r;
                    }
                } while ( ++r.i < e );
            }
            r.f = 0;
            return r;
        }

        bool check( decltype(T::key) k ) {
            return _find( k ).f;
        }

        auto _get( decltype(T::key) k ) {
            return _find( k ).i;
        }

        auto get( decltype(T::key) k ) {
            auto r = _find( k );
            if ( r.f )
                return r.i->val;
            throw out_of_range("key");
        }

        auto &operator[]( decltype(T::key) k ) {
            auto r = _find( k );
            if ( r.f )
                return r.i->val;
            throw out_of_range("key");
        }

        ~_stat() {
            free( tab );
            free( enter_tab );
        }
    };

    public:

    hasher() { // TODO: initialize from data
        n = 0;
        sz = 3;
        mod = 2;

        tab = obj_malloc<E>( sz );
        enter_tab = obj_malloc<E*>( mod );
        obj_memzero( enter_tab, mod );
    }

    void add( T v ) { // TODO: when increase mod ?
        auto p = _find<Ep>( v.key );
        if ( p ) {
            (*p).val = v.val;
        } else {
            if ( n == sz )
                inc_size();
            auto vo = tab + (n++); // TODO: custom allocator, varying size
            vo->v = v;
            vo->nxt = NULL;
            *(p._i) = vo;
        }
    }

    auto immutable() {
        return _stat( *this );
    }

    /*void add2( T v ) {
        if ( n == sz )
            inc_size();
        auto _p = enter_tab + v.hash( mod );
        auto  p = *_p;
        if ( !p ) {
            auto vo = tab + (n++);
            vo->v = v;
            vo->nxt = NULL;
            *_p = vo;
        }
        do {
            //
        } while ( p );
    }*/

    bool check( decltype(T::key) k ) {
        return _find<Ei>( k );
    }

    auto _get( decltype(T::key) k ) {
        return &*_find( k );
    }

    auto get( decltype(T::key) k ) {
        auto r = _find<Ei>( k );
        if ( r )
            return (*r).val;
        throw out_of_range("key");
        /*decltype(T::val) v; // FIXME: exception ?
        v.set_none();
        return v;*/
    }

    auto &operator[]( decltype(T::key) k ) { // TODO: when increase mod ?
        auto p = _find<Ep>( k );
        if ( p ) {
            return (*p).val;
        } else {
            if ( n == sz )
                inc_size();
            auto vo = tab + (n++);
            vo->nxt = NULL;
            *(p._i) = vo;
            return vo->v.val;
        }
    }

    ~hasher() {
        free(tab);
        free(enter_tab);
    }
};

template<class T>
class ptr_wrap {
    public:
    class key_t {
        public:
        T *v;

        inline key_t( T* _v ) : v(_v) {}
        inline key_t() {}

        template< class Tn >
        inline uintptr_t hash( Tn n ) {
            return (*(uintptr_t*)(&v)) % n;
        }

        inline bool operator==( key_t k ){
            return v == k.v;
        }
        inline bool operator!=( key_t k ){
            return v != k.v;
        }

        inline operator bool() {
            return v;
        }

        inline auto &set_none() {
            v = NULL;
            return *this;
        }
    } key;
    class val_t {
        int a[0];
    } val;
    inline ptr_wrap( T* _key ) : key(_key) {}
    inline ptr_wrap() {}
    operator key_t() {return key;}
};

//hasher<ptr_wrap<int>> m;

auto aaaa( hasher<ptr_wrap<int>> *m, int *b ) {
    return m->add( b );
}

auto bbbb( hasher<ptr_wrap<int>> *m, int *b ) {
    return (*m).get( b );
}

auto cccc( hasher<ptr_wrap<int>>::_stat *m, int *b ) {
    return (*m)[ b ];
}

int main() { // speed tests
    //printf( "%d\n", sizeof(ptr_wrap<int>) );
    /*
    ptr_wrap<int> aa = (int*)4;
    ptr_wrap<int> bb;
    bb = aa;
    printf("eeeeeeeelo %p\n",bb.key.v);*/
    /*m.add( (int*)1 );
    m.add( (int*)2 );
    m.add( (int*)2 );
    m.add( (int*)2 );
    m.add( (int*)2 );
    m.add( (int*)2 );
    m.add( (int*)2 );
    printf("=%d\n",m.check((int*)2));//*/
    unordered_set<int*> s;
    hasher<ptr_wrap<int>> n;
    for ( int i=1; i<1000000; i++ ) {
            //printf("%d\n",i);
#ifdef MY_HASH
            n.add((int*)(i*101));
#else
            s.insert((int*)(i*101));
#endif
        }
#if defined MY_HASH && defined STATIC_HASH
    auto ni = n.immutable();
#endif
    for ( int j=0; j<50; j++ ) {
        //s.clear();
        for ( int i=1; i<1500000; i++ ) {
            //printf("%d\n",i);
#ifdef MY_HASH
#ifdef STATIC_HASH
            ni.check((int*)(i*101));
#else
            n.check((int*)(i*101));
#endif
#else
            s.find((int*)(i*101));
#endif
        }
        //auto v2 = n.immutable();
    }//*/
    return 0;
}

