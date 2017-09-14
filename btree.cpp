template<class T>
class btree {
    class node {
        node *l,*r;
        T v;
    } *head = NULL;

    struct Ep {
        E *i;
        E **_i;
        inline Ep( E **v ) {
            _i = v;
            i = *_i;
        }
        inline auto &operator*() {
            return i->v;
        }
        inline operator bool() {
            return i;
        }
    };

    template<class Ti>
    inline int _optimize( node **_v ) {
        int nl, nr;
        auto v = *_v;
        if ( v->l ) {
            nl = _optimize( & v->l );
        } else nl = 0;
        if ( v->r ) {
            nr = _optimize( & v->r );
        } else nr = 0;
        if ( nr > nl ) {
            if ( (nr>>1) >= nl  )
                rotl( _v );
        } else {
            if ( (nl>>1) >= nr )
                rotr( _v );
        }
        return nl + nr + 1;
    }

    template<class Ti>
    inline void iter_e( node *v, Ti &e, void(*proc_func)(T&) ) {
        do {
            if (v->v < e) {
                proc_func( v->v );
                if ( v->l ) {
                    if ( v->r ) iter_e( v->r, e, proc_func );
                    v = v->l;
                } else if ( v->r ) {
                    v = v->r;
                } return;
            } else {
                if ( v->l ) {
                    v = v->l
                } else return;
            }
        } while ( 1 );
    }

    template<class Ti> // WARNING: unordered iteration
    void iter( Ti &b, Ti &e, void(*proc_func)(T&) ) {
        auto v = head;
        if ( !v ) return;
        do {
            if (v->v < b)
                v = v->r;
            else {
                if (v->v < e) {
                    proc_func( v->v );
                    if ( v->r )
                        iter_e( e, proc_func );
                }
                v = v->l;
            }
        } while ( v );
    }

    template<class T2, class Ti>
    inline auto lower_bound( Ti &x ) {
        T2 v = head;
        if ( !v ) return v;
        do {
            if ( (*v).v < x ) {
                if ( (*v).r ) {
                    v = (*v).r;
                    continue;
                }
                return v;
            }
            if ( (*v).l ) {
                v = (*v).l;
                continue;
            }
            return v;
        } while ( 1 );
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

    void del( node **_v ) {
        auto v1 = *_v;
        *_v = v1->l;
        auto v2 = v1->r;
        free( v1 );// TODO: array allocation ?
        do {
            auto v3 = v1->r;
            v1->r = v2;
            v1 = v2;
            v2 = v3;
        } while ( v2 );
    }

    void add( T v ) {
        auto p = lower_bound_place<Ep>( v );
        *(p._i) = new node( v ); // TODO: array allocation ?
    }

    template<class Ti>
    T &get( Ti &x ) {
        auto p = lower_bound<node*>( x );
        if ( !p ) throw out_of_range("x");
        return (*p).v;
    }
};

