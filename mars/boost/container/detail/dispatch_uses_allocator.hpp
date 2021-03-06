//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2015-2015. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_CONTAINER_DISPATCH_USES_ALLOCATOR_HPP
#define BOOST_CONTAINER_DISPATCH_USES_ALLOCATOR_HPP

#if defined (_MSC_VER)
#  pragma once
#endif

#include <boost/container/detail/config_begin.hpp>
#include <boost/container/detail/workaround.hpp>

#include <boost/container/allocator_traits.hpp>
#include <boost/container/uses_allocator.hpp>

#include <boost/container/detail/addressof.hpp>
#include <boost/container/detail/mpl.hpp>
#include <boost/container/detail/pair.hpp>
#include <boost/container/detail/type_traits.hpp>

#if defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES)
#include <boost/move/detail/fwd_macros.hpp>
#endif
#include <boost/move/utility_core.hpp>

#include <boost/core/no_exceptions_support.hpp>

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace container {

namespace container_detail {


// Check if we can detect is_convertible using advanced SFINAE expressions
#if !defined(BOOST_NO_CXX11_DECLTYPE) && !defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES)

   //! Code inspired by Mathias Gaunard's is_convertible.cpp found in the Boost mailing list
   //! http://boost.2283326.n4.nabble.com/type-traits-is-constructible-when-decltype-is-supported-td3575452.html
   //! Thanks Mathias!

   //With variadic templates, we need a single class to implement the trait
   template<class T, class ...Args>
   struct is_constructible
   {
      typedef char yes_type;
      struct no_type
      { char padding[2]; };

      template<std::size_t N>
      struct dummy;

      template<class X>
      static decltype(X(mars_boost::move_detail::declval<Args>()...), true_type()) test(int);

      template<class X>
      static no_type test(...);

      static const bool value = sizeof(test<T>(0)) == sizeof(yes_type);
   };

   template <class T, class InnerAlloc, class ...Args>
   struct is_constructible_with_allocator_prefix
      : is_constructible<T, allocator_arg_t, InnerAlloc, Args...>
   {};

#else    // #if !defined(BOOST_NO_SFINAE_EXPR) && !defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES)

   //Without advanced SFINAE expressions, we can't use is_constructible
   //so backup to constructible_with_allocator_xxx

   #if !defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES)

   template <class T, class InnerAlloc, class ...Args>
   struct is_constructible_with_allocator_prefix
      : constructible_with_allocator_prefix<T>
   {};

   template <class T, class InnerAlloc, class ...Args>
   struct is_constructible_with_allocator_suffix
      : constructible_with_allocator_suffix<T>
   {};

   #else    // #if !defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES)

   template <class T, class InnerAlloc, BOOST_MOVE_CLASSDFLT9>
   struct is_constructible_with_allocator_prefix
      : constructible_with_allocator_prefix<T>
   {};

   template <class T, class InnerAlloc, BOOST_MOVE_CLASSDFLT9>
   struct is_constructible_with_allocator_suffix
      : constructible_with_allocator_suffix<T>
   {};

   #endif   // #if !defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES)

#endif   // #if !defined(BOOST_NO_SFINAE_EXPR)

#if !defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES)

template < typename ConstructAlloc
         , typename ArgAlloc
         , typename T
         , class ...Args
         >
inline typename container_detail::enable_if_and
   < void
   , container_detail::is_not_pair<T>
   , container_detail::not_< uses_allocator<T, ArgAlloc> >
   >::type dispatch_uses_allocator
   ( ConstructAlloc & construct_alloc, BOOST_FWD_REF(ArgAlloc) arg_alloc, T* p, BOOST_FWD_REF(Args)...args)
{
   (void)arg_alloc;
   allocator_traits<ConstructAlloc>::construct(construct_alloc, p, ::mars_boost::forward<Args>(args)...);
}

// allocator_arg_t
template < typename ConstructAlloc
         , typename ArgAlloc
         , typename T
         , class ...Args
         >
inline typename container_detail::enable_if_and
   < void
   , container_detail::is_not_pair<T>
   , uses_allocator<T, ArgAlloc>
   , is_constructible_with_allocator_prefix<T, ArgAlloc, Args...>
   >::type dispatch_uses_allocator
   ( ConstructAlloc& construct_alloc, BOOST_FWD_REF(ArgAlloc) arg_alloc, T* p, BOOST_FWD_REF(Args) ...args)
{
   allocator_traits<ConstructAlloc>::construct
      ( construct_alloc, p, allocator_arg
      , ::mars_boost::forward<ArgAlloc>(arg_alloc), ::mars_boost::forward<Args>(args)...);
}

// allocator suffix
template < typename ConstructAlloc
         , typename ArgAlloc
         , typename T
         , class ...Args
         >
inline typename container_detail::enable_if_and
   < void
   , container_detail::is_not_pair<T>
   , uses_allocator<T, ArgAlloc>
   , container_detail::not_<is_constructible_with_allocator_prefix<T, ArgAlloc, Args...> >
   >::type dispatch_uses_allocator
   ( ConstructAlloc& construct_alloc, BOOST_FWD_REF(ArgAlloc) arg_alloc, T* p, BOOST_FWD_REF(Args)...args)
{
   allocator_traits<ConstructAlloc>::construct
      (construct_alloc, p, ::mars_boost::forward<Args>(args)..., ::mars_boost::forward<ArgAlloc>(arg_alloc));
}

#else    //#if !defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES)

#define BOOST_CONTAINER_SCOPED_ALLOCATOR_DISPATCH_USES_ALLOCATOR_CODE(N) \
   template <typename ConstructAlloc, typename ArgAlloc, typename T BOOST_MOVE_I##N BOOST_MOVE_CLASS##N >\
   inline typename container_detail::enable_if_and\
      < void\
      , container_detail::is_not_pair<T>\
      , container_detail::not_<uses_allocator<T, ArgAlloc> >\
      >::type\
      dispatch_uses_allocator\
      (ConstructAlloc &construct_alloc, BOOST_FWD_REF(ArgAlloc) arg_alloc, T* p BOOST_MOVE_I##N BOOST_MOVE_UREF##N)\
   {\
      (void)arg_alloc;\
      allocator_traits<ConstructAlloc>::construct(construct_alloc, p BOOST_MOVE_I##N BOOST_MOVE_FWD##N);\
   }\
//
BOOST_MOVE_ITERATE_0TO9(BOOST_CONTAINER_SCOPED_ALLOCATOR_DISPATCH_USES_ALLOCATOR_CODE)
#undef BOOST_CONTAINER_SCOPED_ALLOCATOR_DISPATCH_USES_ALLOCATOR_CODE

#define BOOST_CONTAINER_SCOPED_ALLOCATOR_DISPATCH_USES_ALLOCATOR_CODE(N) \
   template < typename ConstructAlloc, typename ArgAlloc, typename T BOOST_MOVE_I##N BOOST_MOVE_CLASS##N >\
   inline typename container_detail::enable_if_and\
      < void\
      , container_detail::is_not_pair<T>\
      , uses_allocator<T, ArgAlloc>\
      , is_constructible_with_allocator_prefix<T, ArgAlloc BOOST_MOVE_I##N BOOST_MOVE_TARG##N>\
      >::type\
      dispatch_uses_allocator\
      (ConstructAlloc& construct_alloc, BOOST_FWD_REF(ArgAlloc) arg_alloc, T* p BOOST_MOVE_I##N BOOST_MOVE_UREF##N)\
   {\
      allocator_traits<ConstructAlloc>::construct\
         (construct_alloc, p, allocator_arg, ::mars_boost::forward<ArgAlloc>(arg_alloc) BOOST_MOVE_I##N BOOST_MOVE_FWD##N);\
   }\
//
BOOST_MOVE_ITERATE_0TO9(BOOST_CONTAINER_SCOPED_ALLOCATOR_DISPATCH_USES_ALLOCATOR_CODE)
#undef BOOST_CONTAINER_SCOPED_ALLOCATOR_DISPATCH_USES_ALLOCATOR_CODE

#define BOOST_CONTAINER_SCOPED_ALLOCATOR_DISPATCH_USES_ALLOCATOR_CODE(N) \
   template < typename ConstructAlloc, typename ArgAlloc, typename T BOOST_MOVE_I##N BOOST_MOVE_CLASS##N >\
   inline typename container_detail::enable_if_and\
      < void\
      , container_detail::is_not_pair<T>\
      , uses_allocator<T, ArgAlloc>\
      , container_detail::not_<is_constructible_with_allocator_prefix<T, ArgAlloc BOOST_MOVE_I##N BOOST_MOVE_TARG##N> >\
      >::type\
      dispatch_uses_allocator\
      (ConstructAlloc& construct_alloc, BOOST_FWD_REF(ArgAlloc) arg_alloc, T* p BOOST_MOVE_I##N BOOST_MOVE_UREF##N)\
   {\
      allocator_traits<ConstructAlloc>::construct\
         (construct_alloc, p BOOST_MOVE_I##N BOOST_MOVE_FWD##N, ::mars_boost::forward<ArgAlloc>(arg_alloc));\
   }\
//
BOOST_MOVE_ITERATE_0TO9(BOOST_CONTAINER_SCOPED_ALLOCATOR_DISPATCH_USES_ALLOCATOR_CODE)
#undef BOOST_CONTAINER_SCOPED_ALLOCATOR_DISPATCH_USES_ALLOCATOR_CODE

#endif   //#if !defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES)

template < typename ConstructAlloc
         , typename ArgAlloc
         , typename Pair
         > inline 
BOOST_CONTAINER_DOC1ST(void, typename container_detail::enable_if<container_detail::is_pair<Pair> >::type)
   dispatch_uses_allocator
   ( ConstructAlloc & construct_alloc
   , ArgAlloc & arg_alloc
   , Pair* p)
{
   (dispatch_uses_allocator)(construct_alloc, arg_alloc, container_detail::addressof(p->first));
   BOOST_TRY{
      (dispatch_uses_allocator)(construct_alloc, arg_alloc, container_detail::addressof(p->second));
   }
   BOOST_CATCH(...) {
      allocator_traits<ConstructAlloc>::destroy(construct_alloc, container_detail::addressof(p->first));
      BOOST_RETHROW
   }
   BOOST_CATCH_END
}


template < typename ConstructAlloc
         , typename ArgAlloc
         , class Pair, class U, class V>
BOOST_CONTAINER_DOC1ST(void, typename container_detail::enable_if<container_detail::is_pair<Pair> >::type)
   dispatch_uses_allocator
   ( ConstructAlloc & construct_alloc
   , ArgAlloc & arg_alloc
   , Pair* p, BOOST_FWD_REF(U) x, BOOST_FWD_REF(V) y)
{
   (dispatch_uses_allocator)(construct_alloc, arg_alloc, container_detail::addressof(p->first), ::mars_boost::forward<U>(x));
   BOOST_TRY{
      (dispatch_uses_allocator)(construct_alloc, arg_alloc, container_detail::addressof(p->second), ::mars_boost::forward<V>(y));
   }
   BOOST_CATCH(...){
      allocator_traits<ConstructAlloc>::destroy(construct_alloc, container_detail::addressof(p->first));
      BOOST_RETHROW
   }
   BOOST_CATCH_END
}

template < typename ConstructAlloc
         , typename ArgAlloc
         , class Pair, class Pair2>
BOOST_CONTAINER_DOC1ST(void, typename container_detail::enable_if< container_detail::is_pair<Pair> >::type)
   dispatch_uses_allocator
   (ConstructAlloc & construct_alloc
   , ArgAlloc & arg_alloc
   , Pair* p, Pair2& x)
{  (dispatch_uses_allocator)(construct_alloc, arg_alloc, p, x.first, x.second);  }

template < typename ConstructAlloc
         , typename ArgAlloc
         , class Pair, class Pair2>
typename container_detail::enable_if_and
   < void
   , container_detail::is_pair<Pair>
   , container_detail::not_<mars_boost::move_detail::is_reference<Pair2> > >::type //This is needed for MSVC10 and ambiguous overloads
   dispatch_uses_allocator
   (ConstructAlloc & construct_alloc
      , ArgAlloc & arg_alloc
      , Pair* p, BOOST_RV_REF_BEG Pair2 BOOST_RV_REF_END x)
{  (dispatch_uses_allocator)(construct_alloc, arg_alloc, p, ::mars_boost::move(x.first), ::mars_boost::move(x.second));  }

//template <typename ConstructAlloc, typename ArgAlloc, class Pair, class Pair2>
//void dispatch_uses_allocator( ConstructAlloc & construct_alloc, ArgAlloc & arg_alloc
//                            , pair<T1, T2>* p, piecewise_construct_t, tuple<Args1...> x, tuple<Args2...> y);

}  //namespace container_detail

}} // namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace container {

#include <boost/container/detail/config_end.hpp>

#endif //  BOOST_CONTAINER_DISPATCH_USES_ALLOCATOR_HPP
