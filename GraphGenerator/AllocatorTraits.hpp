#ifndef ALLOCATOR_TRAITS_HPP
#define ALLOCATOR_TRAITS_HPP
#include <memory>
#include <type_traits>

namespace GraphGenerator
{
    template<typename T, typename = void>
    struct select_handle_or_pointer
    {
        using type = typename std::allocator_traits<T>::pointer;
        inline static auto constexpr has_handle = false;
    };

    template<typename T>
    struct select_handle_or_pointer<T, std::void_t<typename T::handle>>
    {
        using type = typename T::handle;
        inline static auto constexpr has_handle = true;
    };

    template<typename T, typename U, typename = void>
    struct detect_clone_method
    {
        inline static auto constexpr has_clone_method = false;
    };

    template<typename T, typename U>
    struct detect_clone_method<T, U, std::void_t<decltype(&(T::template clone<U>))>>
    {
        inline static auto constexpr has_clone_method = true;
    };

    template<typename Alloc>
    struct AllocatorTraits : std::allocator_traits<Alloc>
    {
        using typename std::allocator_traits<Alloc>::pointer;
        using handle = typename select_handle_or_pointer<Alloc>::type;

        template<typename U>
        static typename std::allocator_traits<Alloc>::template rebind_alloc<U> clone(Alloc& alloc)
        {
            if constexpr (detect_clone_method<Alloc, U>::has_clone_method)
            {
                return alloc.template clone<U>();
            }
            else
            {
                static_assert(std::is_empty_v<Alloc>, "non-stateless allocator needs to have thread safe clone() method");
                return {};
            }
        }

        static handle translate(Alloc& alloc, pointer p)
        {
            if constexpr (select_handle_or_pointer<Alloc>::has_handle)
            {
                return alloc.translate(p);
            }
            else
            {
                return p;
            }
        }

        static pointer resolve(Alloc& alloc, handle h)
        {
            if constexpr (select_handle_or_pointer<Alloc>::has_handle)
            {
                return alloc.resolve(h);
            }
            else
            {
                return h;
            }
        }
    };
}

#endif // ALLOCATOR_TRAITS_HPP