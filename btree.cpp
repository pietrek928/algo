#include <cstdio>
#include <set>

using namespace std;

template<class T>
class Default_ArrayAlloc {
public:
    template<class ... Args>
    T *operator()( Args... args ) {
        return new T( args... );
    }
    void del( T *p ) {
        delete p;
    }
};

template<class T, template<class Tn> class Allocator = Default_ArrayAlloc>
class btree {
    public:
    struct node {
        node *l,*r;
        T v;
        node( T &v ) : v(v), l(NULL), r(NULL) {}
    } *head = NULL;

    Allocator<node> A;

    template<class Tp>
    struct Ep {
        Tp *i;
        Tp **_i;
        inline Ep( Tp *&v ) {
            _i = &v;
            i = v;
        }
        inline Ep( Tp **v ) {
            _i = v;
            i = *_i;
        }
        inline auto &operator*() {
            return *i;
        }
        inline operator bool() {
            return i;
        }
        inline operator Tp**() {
            return _i;
        }
    };

    template<void proc_func (T&)>
    inline void iter_all( node *v ) {
        do {
            proc_func( v->v );
            if ( v->r ) iter_all<proc_func>( v->r );
            v = v->l;
        } while ( v );
    }

    template<void proc_func (T&), class Ti>
    inline void iter_e( node *v, Ti &e ) {
        do {
            if (v->v < e) {
                proc_func( v->v );
                if ( v->l ) iter_all<proc_func>( v->l );
                v = v->r;
            } else {
                v = v->l;
            }
        } while ( v );
    }

    inline int _optimize( node **_v ) {
        const static int rot_set = 2; // TODO: tune
        const static int rot_diff = 2;

        int nl, nr;
        auto v = *_v;
        if ( v->l ) {
            nl = _optimize( & v->l );
        } else nl = 0;
        if ( v->r ) {
            nr = _optimize( & v->r );
        } else nr = 0;
        int r = (nr>nl ? nr : nl) + 1;
        if ( nr > nl ) {
            if ( nr > nl+rot_diff )
                {rotl( _v );r=rot_set;}
        } else {
            if ( nl > nr+rot_diff )
                {rotr( _v );r=rot_set;}
        }
        return r;
    }

    template<class T2, class Ti>
    inline auto lower_bound_place( Ti &x ) {
        T2 v = head;
        if ( !v ) return v;
        do {
            if ( (*v).v < x ) v = (*v).r;
                else v = (*v).l;
        } while ( v );
        return v;
    }

    inline auto rotr( node **_v ) {
        auto v = *_v;
        auto w = *_v = v->l;
        v->l = w->r;
        w->r = v;
        return w;
    }
    inline auto rotl( node **_v ) {
        auto v = *_v;
        auto w = *_v = v->r;
        v->r = w->l;
        w->l = v;
        return w;
    }

    void del_node( node **_v ) {
        auto v1 = *_v;
        *_v = v1->l;
        auto v2 = v1->r;
        A.del( v1 );
        v1 = *_v;
        do {
            auto v3 = v1->r;
            v1->r = v2;
            v1 = v2;
            v2 = v3;
            if ( !v2 ) return;
            v3 = v1->l;
            v1->l = v2;
            v1 = v2;
            v2 = v3;
        } while ( v2 );
    }

    public:

    template<void proc_func (T&), class Ti> // WARNING: unordered iteration TODO: ordered variant
    void iter( Ti &b, Ti &e ) {
        auto v = head;
        if ( !v ) return;
        do {
            if (v->v < b)
                v = v->r;
            else {
                if (v->v < e) {
                    proc_func( v->v );
                    if ( v->r )
                        iter_e<proc_func>( v->r, e );
                }
                v = v->l;
            }
        } while ( v );
    }

    void optimize() {
        _optimize( &head );
    }

    template<class T2, class Tr=T2, class Ti>
    auto upper_bound( Ti &x ) {
        T2 v = head;
        Tr r = v;
        if ( !v ) return r;
        do {
            auto &_v = *v;
            if ( _v.v < x ) {
                v = _v.r;
            } else {
                r = v;
                v = _v.l;
            }
        } while ( v );
        return r;
    }

    template<class Ts>
    void del( Ts &x ) {
        auto p = upper_bound<Ep<node>,node**>( x ); // search prev value ?
        if (!( *p && x==(*p)->v )) throw out_of_range("x");
        del_node( p );
    }

    void add( T v ) {
        auto p = lower_bound_place<Ep<node>>( v );
        *(p._i) = A( v );
    }

    template<class Ti>
    T &get( Ti &x ) {
        auto p = upper_bound<node*>( x );
        if ( !p ) throw out_of_range("x");
        return (*p).v;
    }

    void _show( node *v, int tab ) {
        if (!v) return;
        _show( v->l, tab+1 );
        printf("%.*s%d\n", tab, "                               ",v->v);
        _show( v->r, tab+1 );
    }
    void show() {_show(head,0);}
};

inline void aa( int &a ) {
    printf("%d ",a);
}

int main() {
    /*Default_ArrayAlloc<int> a;
    auto b = a( 1 );
    a.del( b ); //*/
    btree<int> t;
    for ( int i=0; i<30; i++ ) t.add(i);
    t.optimize();
    t.optimize();
    t.optimize();
    t.optimize();
    t.optimize();
    t.optimize();
    t.optimize();
    t.optimize();//*/
    t.show(); printf("\n");
    int a=5, b=25, c=7;
    t.del( c );
    t.show(); printf("\n");
    t.iter<aa>( a, b ); printf("\n");
    return 0;
}

