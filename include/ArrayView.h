// This file is designed to be allowed to be included multiple times
#ifndef UTILS_ARRAY_VIEW_H
#define UTILS_ARRAY_VIEW_H


// © Visse 2019
// Lisense: Public Domain




// To disable iterators define UTILS_ARRAY_VIEW_NO_ITERATORS before including this file
// Iterators can later be enabled by including this file again, with UTILS_ARRAY_VIEW_NO_ITERATORS undefined
/*
#define UTILS_ARRAY_VIEW_NO_ITERATORS
#include "ArrayView.h"

// Later...

#undef UTILS_ARRAY_VIEW_NO_ITERATORS
#include "ArrayView.h"
*/


#include <cstdint>

#ifndef UTILS_ARRAY_VIEW_ASSERT
#include <cassert>
#define UTILS_ARRAY_VIEW_ASSERT(cond, msg) assert (cond && msg);
#endif

namespace Utils 
{
    using size_t = std::size_t;
    
    static const constexpr size_t RuntimeSize = 0;
    
    namespace internal
    {
        struct size_construction_tag {};
        struct array_construction_tag {};
        
        template<typename T, size_t ... Dim >
        struct Index {
            constexpr size_t stride() const;
            constexpr size_t size() const;
            constexpr size_t size(size_t dim) const;
        };
    }
    
    
    template< typename T, size_t ... Dim >
    struct ArrayView {
    public:
        ArrayView() = default;
        ArrayView( const ArrayView& ) = default;
        ArrayView& operator = ( const ArrayView& ) = default;
        
        // Construct from an array, size must be manually provided for RuntimeSize slices
        template< typename... Sizes >
        explicit ArrayView( T *array, Sizes... sizes );
        
        explicit operator bool () const;
        
        template< size_t D, size_t ...S >
        struct ArrayViewSlice {
            using type = ArrayView<T,S...>;
        };
        
        typename ArrayViewSlice<Dim...>::type operator [] ( size_t i ) const;
        
        size_t size() const;
        
        struct iterator;
        iterator begin();
        iterator end();
    };
    
    template< typename T, size_t Dim >
    struct ArrayView<T,Dim> :
        public internal::Index<T,Dim>
    {        
        using Index = internal::Index<T,Dim>;
        
        ArrayView() = default;
        
        explicit ArrayView( T *array ) :
            mArray(array)
        {}
        ArrayView( T *array, Index index ) :
            Index(index),
            mArray(array)
        {}
        
        explicit operator bool () const {
            return mArray != nullptr;
        }
        
        T& operator [] (size_t i) {
            UTILS_ARRAY_VIEW_ASSERT (mArray, "ArrayView::operator [] - array is null");
            UTILS_ARRAY_VIEW_ASSERT (i < Index::size(), "ArrayView::operator [] - index out of range");
            return mArray[i];
        }
        
        size_t size() {
            return Index::size();
        }
        
        size_t size( size_t dim ) {
            UTILS_ARRAY_VIEW_ASSERT(dim < 1, "ArrayView::size - dimension out of range");
            if (!mArray) return 0;
            return size();
        }
        
        T* begin() {
            return mArray;
        }
        T* end() {
            return mArray + size();
        }
        
    private:
        T *mArray = nullptr;
    };
    
    template< typename T, size_t Dim, size_t ...Tail>
    struct ArrayView<T, Dim, Tail...>  :
        public internal::Index<T,Dim,Tail...>
    {
        using Index = internal::Index<T,Dim,Tail...>;
        
        template< typename, size_t ...>
        friend struct ArrayView;
        
        ArrayView() = default;
        
        template< typename... Args >
        explicit ArrayView( T *array, Args... sizes ) :
            Index(internal::size_construction_tag(), sizes...),
            mArray(array)
        {}
        
        template< size_t ...Sizes >
        ArrayView( const ArrayView<T,Sizes...> &copy ) :
            Index(static_cast<const internal::Index<T,Sizes...>&>(copy)),
            mArray(copy.mArray)
        {}
        
        template< typename TT, typename = typename std::enable_if<std::is_array<TT>::value>::type >
        ArrayView( TT &array ) :
           Index(internal::array_construction_tag(), array),
           mArray(reinterpret_cast<T*>(array))
        {}
        
        explicit operator bool () const {
            return mArray != nullptr;
        }
        
        ArrayView<T,Tail...> operator [] ( size_t i ) const {
            UTILS_ARRAY_VIEW_ASSERT (mArray, "ArrayView::operator [] - array is null");
            UTILS_ARRAY_VIEW_ASSERT (i < Index::size(), "ArrayView::operator [] - index out of range");
            
            return ArrayView<T, Tail...>(
                mArray + Index::stride() * i,
                static_cast<const internal::Index<T,Tail...>&>(*this)
            );
        }
        
        size_t size() const {
            if (!mArray) return 0;
            return Index::size();
        }
        
        size_t size( size_t dim ) {
            UTILS_ARRAY_VIEW_ASSERT(dim <= sizeof...(Tail), "ArrayView::size - dimension out of range");
            if (!mArray) return 0;
            return Index::size(dim);
        }
        
        struct iterator;
        iterator begin() {
            return iterator(*this);
        }
        iterator end() {
            return iterator(*this, size());
        }
    private:
        T *mArray = nullptr;
    };

    namespace internal 
    {
        template<typename T>
        struct Index<T> {
            Index() = default;
            Index( const Index& ) = default;
            Index& operator = ( const Index& ) = default;
            
            Index(size_construction_tag) {}
            
            Index( array_construction_tag, T& ) {}
            
            constexpr size_t stride() const {
                return 1;
            }
            constexpr size_t size() const {
                return 1;
            }
            constexpr size_t size(size_t dim) const {
                return 1;
            }
        };
        
        template<typename T, size_t Dim, size_t ...Tail>
        struct Index<T, Dim, Tail...> :
            public Index<T, Tail...>
        {
            using Base = Index<T,Tail...>;
            
            Index() = default;
            
            template< typename ...Args >
            Index( size_construction_tag tag, Args... args ) :
                Base(tag, args...)
            {}
            
            template< typename TT, size_t D >
            Index( array_construction_tag tag, TT (&array)[D] ) :
                Base(tag, *array)
            {
                static_assert(D == Dim, "Dimension missmatch!");
            }
            
            template< size_t ...Sizes >
            Index( const Index<T, Dim, Sizes...> &copy ) :
                Base(static_cast<const Index<T,Sizes...>&>(copy))
            {}
            
            Index( const Index& ) = default;
            Index& operator = ( const Index& ) = default;
            
            constexpr size_t stride() const {
                return Base::size() * Base::stride();
            }
            constexpr size_t size() const {
                return Dim;
            }
            constexpr size_t size(size_t dim) const {
                return (dim == 0) ? Dim : Base::size(dim-1);
            }
        };
        
        template<typename T, size_t ...Tail>
        struct Index<T, RuntimeSize, Tail...> :
            public Index<T, Tail...>
        {
            using Base = Index<T,Tail...>;
            
            size_t Dim = 0;
            
            Index() = default;
            Index( const Index& ) = default;
            Index& operator = ( const Index& ) = default;
            
            template< size_t ...Sizes >
            Index( const Index<T,RuntimeSize,Sizes...> &copy ) :
                Base(static_cast<const Index<T,Sizes...>&>(copy)),
                Dim(copy.Dim)
            {}
            
            template< size_t D, size_t ...Sizes >
            Index( const Index<T,D,Sizes...> &copy ) :
                Base(static_cast<const Index<T,Sizes...>&>(copy)),
                Dim(D)
            {}
            
            template< typename ...Args >
            Index( size_construction_tag tag, size_t size_, Args... args ) :
                Base(tag, args...),
                Dim(size_)
            {
                UTILS_ARRAY_VIEW_ASSERT (Dim > 0, "ArrayView::ArrayView - RuntimeSize must be greater than 0");
            }
            
            template< typename TT, size_t D >
            Index( array_construction_tag tag,  TT (&array)[D]) :
                Base(tag, *array),
                Dim(D)
            {
            }

            constexpr size_t stride() const {
                return Base::size() * Base::stride();
            }
            constexpr size_t size() const {
                return Dim;
            }
            constexpr size_t size(size_t dim) const {
                return (dim == 0) ? Dim : Base::size(dim-1);
            }
        };
    }
    
}

#endif // UTILS_ARRAY_VIEW_H

#ifndef UTILS_ARRAY_VIEW_NO_ITERATORS
#ifndef UTILS_ARRAY_VIEW_ITERATORS_H
#define UTILS_ARRAY_VIEW_ITERATORS_H

// the iterator header is huge (~29k lines)
#include <iterator>

namespace Utils 
{
    template< typename T, size_t Dim, size_t ...Tail>
    struct ArrayView<T,Dim, Tail...>::iterator {
        using value_type = ArrayView<T,Tail...>;
        using pointer = value_type*;
        using reference = value_type&;
        
        using distance = ptrdiff_t;
        using iterator_category = std::random_access_iterator_tag;
        
    public:
        iterator( ArrayView &view, size_t idx=0 ) :
            mView(&view),
            mIdx(idx)
        {}
        
        iterator( const iterator& ) = default;
        iterator& operator = ( const iterator& ) = default;
        
        void advance( distance dist ) {
            mIdx += dist;
        }
        reference dereference() const {
            mValue = (*mView)[mIdx];
            return mValue;
        }
        
        reference operator * () const {
            return dereference();
        }
        pointer operator -> () const {
            return &dereference();
        }
        
        bool operator == ( const iterator &other ) const {
            if (mView != other.mView) return false;
            if (mIdx != other.mIdx) return false;
            return true;
        }
        bool operator != ( const iterator &other ) const {
            return !(*this == other);
        }
        bool operator < ( const iterator &other ) const {
            return mIdx < other.mIdx;
        }
        bool operator <= ( const iterator &other ) const {
            return mIdx <= other.mIdx;
        }
        bool operator > ( const iterator &other ) const {
            return mIdx > other.mIdx;
        }
        bool operator >= ( const iterator &other ) const {
            return mIdx >= other.mIdx;
        }
        
        
        iterator& operator ++ () {
            advance(1);
            return *this;
        }
        iterator operator ++ (int) {
            iterator copy = *this;
            advance(1);
            return copy;
        }
        
        iterator& operator -- () {
            advance(-1);
            return *this;
        }
        iterator operator -- (int) {
            iterator copy = *this;
            advance(-1);
            return copy;
        }
        
        friend iterator operator + ( const iterator &lhs, distance rhs ) {
            iterator copy(lhs);
            copy.advance(rhs);
            return copy;
        }
        friend iterator operator + ( distance lhs, const iterator &rhs ) {
            iterator copy(rhs);
            copy.advance(lhs);
            return copy;
        }
        friend iterator operator - ( const iterator &lhs, distance rhs ) {
            iterator copy(lhs);
            copy.advance(-rhs);
            return copy;
        }
        friend distance operator - ( const iterator &lhs, const iterator &rhs ) {
            return distance(lhs.mIdx) - distance(rhs.mIdx);
        }
        
        iterator& operator += ( distance dist ) {
            advance(dist);
            return *this;
        }
        iterator& operator -= ( distance dist ) {
            advance(-dist);
            return *this;
        }
        
        value_type operator [] ( distance dist ) {
            size_t idx = mIdx + dist;
            return (*mView)[idx];
        }
        
    private:
        ArrayView *mView;
        size_t mIdx = 0;
        // Cache to allow returning reference
        mutable value_type mValue;
    };
}
#endif // UTILS_ARRAY_VIEW_ITERATORS_H
#endif // UTILS_ARRAY_VIEW_NO_ITERATORS
