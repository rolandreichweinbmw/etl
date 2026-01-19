///\file

/******************************************************************************
The MIT License(MIT)

Embedded Template Library.
https://github.com/ETLCPP/etl
https://www.etlcpp.com

Copyright(c) 2026 BMW AG

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
******************************************************************************/

#ifndef ETL_RANGES_INCLUDED
#define ETL_RANGES_INCLUDED

#include "platform.h"

#include "array.h"
#include "delegate.h"
#include "iterator.h"
#include "limits.h"
#include "optional.h"
#include "span.h"
#include "tuple.h"
#include "type_traits.h"
#include "variant.h"

#if ETL_USING_CPP17

namespace etl
{
  namespace ranges
  {
    //*************************************************************************
    /// Range adaptors.
    //*************************************************************************

    namespace private_ranges
    {
      template<typename T, typename Enable = void>
      struct iterator_trait;

      template<typename T>
      struct iterator_trait<T, etl::enable_if_t<etl::is_class_v<T>>>
      {
        typedef typename T::iterator iterator;
        typedef typename T::const_iterator const_iterator;

        typedef typename etl::iterator_traits<const_iterator>::value_type value_type;
        typedef typename etl::iterator_traits<const_iterator>::difference_type difference_type;
        typedef typename etl::iterator_traits<const_iterator>::pointer pointer;
        typedef typename etl::iterator_traits<const_iterator>::reference reference;
      };

      template<typename T>
      struct iterator_trait<T, etl::enable_if_t<etl::is_reference_v<T> && !etl::is_array_v<etl::remove_reference_t<T>>>>
      {
        typedef typename etl::remove_reference<T>::type::iterator iterator;
        typedef typename etl::remove_reference<T>::type::const_iterator const_iterator;

        typedef typename etl::iterator_traits<iterator>::value_type value_type;
        typedef typename etl::iterator_traits<iterator>::difference_type difference_type;
        typedef typename etl::iterator_traits<iterator>::pointer pointer;
        typedef typename etl::iterator_traits<iterator>::reference reference;
      };

      template<typename T>
      struct iterator_trait<T, etl::enable_if_t<etl::is_array_v<etl::remove_reference_t<T>>>>
      {
        using value_type = typename etl::remove_all_extents<etl::remove_reference_t<T>>::type;
        using iterator = value_type*;
        using const_iterator = const value_type*;
        using difference_type = ptrdiff_t;
        using pointer = const value_type*;
        using reference = const value_type&;
      };
    }

    template<class D>
    class view_interface
    {
      public:
        view_interface() = default;

        constexpr bool empty() const
        {
          return cbegin() == cend();
        }

        auto cbegin() const
        {
          return static_cast<const D*>(this)->begin();
        }

        auto cend() const
        {
          return static_cast<const D*>(this)->end();
        }

        operator bool() const
        {
          return !empty();
        }

        size_t size() const
        {
          return etl::distance(cbegin(), cend());
        }

        constexpr decltype(auto) front()
        {
          return *(static_cast<D*>(this)->begin());
        }

        constexpr decltype(auto) front() const
        {
          return *cbegin();
        }

        constexpr decltype(auto) back()
        {
          return *(static_cast<D*>(this)->end() - 1);
        }

        constexpr decltype(auto) back() const
        {
          return *(cend() - 1);
        }

        constexpr decltype(auto) operator[](size_t i)
        {
          auto it{static_cast<D*>(this)->begin()};
          etl::advance(it, i);
          return *it;
        }

        constexpr decltype(auto) operator[](size_t i) const
        {
          auto it{cbegin()};
          etl::advance(it, i);
          return *it;
        }
    };

    template<class T, class U>
    bool operator==(const view_interface<T>& lhs, const view_interface<U>& rhs)
    {
      return etl::equal(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend());
    }

    template<class T, class U>
    bool operator!=(const view_interface<T>& lhs, const view_interface<U>& rhs)
    {
      return !(lhs == rhs);
    }

    template <class I>
    class range_iterator
    {
    public:
      auto get() const
      {
        return **(static_cast<const I*>(this));
      }
    };

    template<class I, class S = I>
    class subrange: public etl::ranges::view_interface<subrange<I, S>>
    {
    public:
      subrange(I i, S s): _begin{i}, _end{s}
      {
      }

      constexpr I begin() const
      {
        return _begin;
      }

      constexpr S end() const
      {
        return _end;
      }

      constexpr subrange& advance(etl::iter_difference_t<I> n)
      {
        etl::advance(_begin, n);
        return *this;
      }

      constexpr subrange prev(etl::iter_difference_t<I> n = 1)
      {
        auto result = subrange{_begin, _end};
        result.advance(-n);
        return result;
      }

      constexpr subrange next(etl::iter_difference_t<I> n = 1)
      {
        auto result = subrange{_begin, _end};
        result.advance(n);
        return result;
      }

    private:
      I _begin;
      S _end;
    };

    template<class I, class S>
    subrange(I, S) -> subrange<I, S>;

    template<class T>
    class empty_view: public etl::ranges::view_interface<empty_view<T>>
    {
    public:
      using iterator = T*;

      constexpr empty_view() = default;

      static constexpr iterator begin() noexcept
      {
        return nullptr;
      }

      static constexpr iterator end() noexcept
      {
        return nullptr;
      }

      static constexpr T* data() noexcept
      {
        return nullptr;
      }

      static constexpr size_t size() noexcept
      {
        return 0;
      }

      static constexpr bool empty() noexcept
      {
        return true;
      }
    };

    namespace views
    {
      template <class T>
      constexpr empty_view<T> empty{};
    }

    template<class T>
    class single_view: public etl::ranges::view_interface<single_view<T>>
    {
    public:
      using value_type = T;
      using iterator = value_type*;
      using const_iterator = const value_type*;

      constexpr single_view(const T& t) noexcept: _value(t)
      {
      }

      constexpr single_view(T&& t) noexcept: _value(etl::move(t))
      {
      }

      constexpr single_view(const single_view<T>& other): _value(other._value) {}

      constexpr single_view(single_view<T>&& other): _value(etl::move(other._value)) {}

      constexpr single_view& operator=(const single_view<T>& other)
      {
        _value = other._value;
        return *this;
      }

      constexpr single_view& operator=(single_view<T>&& other)
      {
        _value = etl::move(other._value);
        return *this;
      }

      constexpr iterator begin() noexcept
      {
        return data();
      }

      constexpr const_iterator begin() const noexcept
      {
        return data();
      }

      constexpr iterator end() noexcept
      {
        return data() + 1;
      }

      constexpr const_iterator end() const noexcept
      {
        return data() + 1;
      }

      constexpr const T* data() const noexcept
      {
        return &_value;
      }

      constexpr T* data() noexcept
      {
        return &_value;
      }

      constexpr size_t size() const noexcept
      {
        return 1;
      }

      constexpr bool empty() const noexcept
      {
        return false;
      }

    private:
      value_type _value;
    };

    template<class T>
    single_view(T) -> single_view<T>;

    namespace views
    {
      namespace private_views
      {
        struct single
        {
          template<typename T>
          constexpr auto operator()(T&& t) const
          {
            return etl::ranges::single_view(t);
          }
        };
      }

      inline constexpr private_views::single single;
    }

    template<typename T>
    struct iota_iterator: public range_iterator<iota_iterator<T>>
    {
    public:
      using value_type = T;
      using difference_type = ptrdiff_t;
      using pointer = T*;
      using reference = T&;

      using iterator_category =  ETL_OR_STD::random_access_iterator_tag;

      constexpr explicit iota_iterator(T i): _i{i}
      {
      }

      constexpr iota_iterator(const iota_iterator& other): _i{other._i}
      {
      }

      iota_iterator& operator++()
      {
        ++_i;
        return *this;
      }

      iota_iterator operator++(int)
      {
        iota_iterator tmp = *this;
        _i++;
        return tmp;
      }

      iota_iterator& operator--()
      {
        --_i;
        return *this;
      }

      iota_iterator operator--(int)
      {
        iota_iterator tmp = *this;
        _i--;
        return tmp;
      }

      iota_iterator& operator+=(difference_type n)
      {
        _i += n;
        return *this;
      }

      iota_iterator operator+(difference_type n) const
      {
        return iota_iterator{static_cast<value_type>(_i + n)};
      }

      iota_iterator operator-(difference_type n) const
      {
        return iota_iterator{static_cast<value_type>(_i - n)};
      }

      size_t operator-(iota_iterator other) const
      {
        return _i - other._i;
      }

      iota_iterator& operator=(const iota_iterator& other)
      {
        _i = other._i;
        return *this;
      }

      constexpr bool operator==(const iota_iterator& other) const
      {
        return _i == other._i;
      }

      constexpr bool operator!=(const iota_iterator& other) const
      {
        return _i != other._i;
      }

      constexpr value_type operator*() const
      {
        return _i;
      }

      constexpr value_type operator*()
      {
        return _i;
      }

    private:
      value_type _i;
    };

    template<class T>
    class iota_view: public etl::ranges::view_interface<iota_view<T>>
    {
    public:
      using iterator = iota_iterator<T>;
      using const_iterator = iota_iterator<T>;

      iota_view() = default;

      constexpr explicit iota_view(T value, T bound = etl::numeric_limits<T>::max()): _value(value), _bound(bound)
      {
      }

      constexpr iterator begin() const noexcept
      {
        return iterator(_value);
      }

      constexpr iterator end() const noexcept
      {
        return iterator(_bound);
      }

      constexpr size_t size() const noexcept
      {
        if (_bound == etl::numeric_limits<T>::max())
        {
          return etl::numeric_limits<T>::max();
        }
        return _bound - _value;
      }

      constexpr bool empty() const noexcept
      {
        return _value == _bound;
      }

    private:
      T _value;
      T _bound;
    };

    template<class T>
    iota_view(T) -> iota_view<T>;

    namespace views
    {
      namespace private_views
      {
        struct iota
        {
          template<typename T, typename B>
          constexpr auto operator()(T&& t, B&& b) const
          {
            return etl::ranges::iota_view(t, b);
          }
        };
      }

      inline constexpr private_views::iota iota;
    }

    template<typename T, typename B = T>
    struct repeat_iterator: public range_iterator<repeat_iterator<T, B>>
    {
    public:
      using value_type = T;
      using difference_type = ptrdiff_t;
      using pointer = T*;
      using reference = T&;

      using iterator_category =  ETL_OR_STD::random_access_iterator_tag;

      constexpr explicit repeat_iterator(T value, B i = etl::numeric_limits<B>::max()): _value{value}, _i{i}
      {
      }

      constexpr repeat_iterator(const repeat_iterator& other) = default;

      repeat_iterator& operator++()
      {
        --_i;
        return *this;
      }

      repeat_iterator operator++(int)
      {
        repeat_iterator tmp(*this);
        _i--;
        return tmp;
      }

      repeat_iterator& operator--()
      {
        ++_i;
        return *this;
      }

      repeat_iterator operator--(int)
      {
        repeat_iterator tmp(*this);
        _i++;
        return tmp;
      }

      repeat_iterator& operator+=(size_t n)
      {
        _i -= n;
        return *this;
      }

      repeat_iterator operator+(size_t n) const
      {
        return repeat_iterator{_value, static_cast<B>(_i - n)};
      }

      repeat_iterator operator-(size_t n) const
      {
        return repeat_iterator{_value, static_cast<B>(_i + n)};
      }

      difference_type operator-(repeat_iterator other) const
      {
        return other._i - _i;
      }

      repeat_iterator& operator=(const repeat_iterator& other)
      {
        _i = other._i;
        _value = other._value;
        return *this;
      }

      constexpr bool operator==(const repeat_iterator& other) const
      {
        return _i == other._i;
      }

      constexpr bool operator!=(const repeat_iterator& other) const
      {
        return _i != other._i;
      }

      constexpr value_type operator*() const
      {
        return _value;
      }

      constexpr value_type operator*()
      {
        return _value;
      }

    private:
      value_type _value;
      B _i;
    };

    template<class T, class B = T>
    class repeat_view: public etl::ranges::view_interface<repeat_view<T>>
    {
    public:
      using iterator = repeat_iterator<T, B>;
      using const_iterator = repeat_iterator<T, B>;

      repeat_view() = default;

      constexpr explicit repeat_view(T value, B bound = etl::numeric_limits<B>::max()): _value(value), _bound(bound)
      {
      }

      constexpr iterator begin() const noexcept
      {
        return iterator(_value, _bound);
      }

      constexpr iterator end() const noexcept
      {
        return iterator(_value, 0);
      }

      constexpr size_t size() const noexcept
      {
        return _bound;
      }

      constexpr bool empty() const noexcept
      {
        return _bound == 0;
      }

    private:
      T _value;
      B _bound;
    };

    template<class T, class B = T>
    repeat_view(T, B = B()) -> repeat_view<T, B>;

    namespace views
    {
      namespace private_views
      {
        struct repeat
        {
          template<typename T, typename B>
          constexpr auto operator()(T&& t, B&& b) const
          {
            return etl::ranges::repeat_view(t, b);
          }
        };
      }

      inline constexpr private_views::repeat repeat;
    }

    template <class Range>
    class range_adapter_closure
    {
    };

    template<class Range>
    class ref_view: public etl::ranges::view_interface<ref_view<Range>>
    {
      public:
        using iterator = typename etl::ranges::private_ranges::iterator_trait<Range>::iterator;
        using const_iterator = typename etl::ranges::private_ranges::iterator_trait<Range>::const_iterator;
        using pointer = typename etl::ranges::private_ranges::iterator_trait<Range>::pointer;

        ref_view(Range& r): _r{&r}
        {
        }

        constexpr Range& base() const
        {
          return *_r;
        }

        constexpr iterator begin() const
        {
          return iterator(ETL_OR_STD::begin(*_r));
        }

        constexpr iterator end() const
        {
          return iterator(ETL_OR_STD::end(*_r));
        }

        constexpr bool empty() const
        {
          return begin() == end();
        }

        constexpr size_t size() const
        {
          return etl::distance(begin(), end());
        }

        constexpr pointer data() const
        {
          return &(*begin());
        }

      private:
        Range* _r;
    };

    template<class Range>
    ref_view(Range&) -> ref_view<Range>;

    struct ref_range_adapter_closure: public range_adapter_closure<ref_range_adapter_closure>
    {
      template<typename Range>
      using target_view_type = ref_view<Range>;

      ref_range_adapter_closure() = default;

      template<typename Range>
      ref_view<Range> operator()(Range& r)
      {
        return ref_view<Range>(r);
      }
    };

    namespace views
    {
      namespace private_views
      {
        struct ref
        {
          template<class Range>
          constexpr auto operator()(Range& r) const
          {
            return ranges::ref_view(r);
          }

          constexpr auto operator()() const
          {
            return ranges::ref_range_adapter_closure();
          }
        };
      }

      inline constexpr private_views::ref ref;
    }

    template<class Range>
    class owning_view: public etl::ranges::view_interface<owning_view<Range>>
    {
      public:
        using iterator = typename etl::ranges::private_ranges::iterator_trait<Range>::iterator;
        using const_iterator = typename etl::ranges::private_ranges::iterator_trait<Range>::const_iterator;
        using pointer = typename etl::ranges::private_ranges::iterator_trait<Range>::pointer;

        constexpr owning_view(): _r{}
        {
        }

        constexpr owning_view(Range&& r) : _r(etl::move(r))
        {
        }

        owning_view& operator=(const owning_view&) = delete;

        owning_view& operator=(owning_view&& other)
        {
          _r = etl::move(other._r);
          return *this;
        }

        constexpr Range& base() noexcept
        {
          return _r;
        }

        constexpr iterator begin() const
        {
          return iterator(ETL_OR_STD::begin(_r));
        }

        constexpr iterator end() const
        {
          return iterator(ETL_OR_STD::end(_r));
        }

        constexpr bool empty() const
        {
          return begin() == end();
        }

        constexpr size_t size() const
        {
          return etl::distance(begin(), end());
        }

        constexpr pointer data()
        {
          return &(*begin());
        }

      private:
        Range _r;
    };

    template<class Range>
    owning_view(Range&&) -> owning_view<Range>;

    struct owning_range_adapter_closure: public range_adapter_closure<owning_range_adapter_closure>
    {
      template<typename Range, typename CleanRange = etl::remove_reference_t<Range>>
      using target_view_type = owning_view<CleanRange>;

      owning_range_adapter_closure() = default;

      template<typename Range, typename CleanRange = etl::remove_reference_t<Range>>
      owning_view<CleanRange> operator()(Range&& r)
      {
        return owning_view<CleanRange>(etl::move(r));
      }
    };

    namespace views
    {
      namespace private_views
      {
        struct owning
        {
          template<class Range>
          constexpr auto operator()(Range& r) const
          {
            return ranges::owning_view(r);
          }

          constexpr auto operator()() const
          {
            return ranges::owning_range_adapter_closure();
          }
        };
      }

      inline constexpr private_views::owning owning;
    }

    namespace views
    {
      namespace private_views
      {
        struct all
        {
          template<class Range, etl::enable_if_t<etl::is_base_of_v<etl::ranges::view_interface<etl::decay_t<Range>>, etl::decay_t<Range>>, int> = 0>
          constexpr auto operator()(Range&& r) const
          {
            return etl::decay_t<Range>(r);
          }

          template<class Range, etl::enable_if_t<!etl::is_base_of_v<etl::ranges::view_interface<etl::decay_t<Range>>, etl::decay_t<Range>>, int> = 0>
          constexpr auto operator()(Range&& r) const
          {
            if constexpr(etl::is_lvalue_reference_v<Range>)
            {
              return etl::ranges::ref_view(r);
            }
            else
            {
              return etl::ranges::owning_view(r);
            }
          }
        };
      }

      inline constexpr private_views::all all;

      template<typename R>
      using all_t = decltype(views::all(std::declval<R>()));
    }

    template<class Range, class Pred>
    class filter_iterator
    {
      public:
        using trait = typename etl::ranges::private_ranges::iterator_trait<Range>;

        using iterator = typename trait::iterator;
        using const_iterator = typename trait::const_iterator;
        using value_type = typename trait::value_type;
        using difference_type = typename trait::difference_type;
        using pointer = typename trait::pointer;
        using reference = typename trait::reference;

        using iterator_category =  ETL_OR_STD::bidirectional_iterator_tag;

        filter_iterator(const_iterator it, const_iterator it_end, const Pred& p): _it{it}, _it_begin{it}, _it_end{it_end}, _p{p}
        {
        }

        filter_iterator(const filter_iterator& other): _it{other._it}, _it_begin{other._it_begin}, _it_end{other._it_end}, _p{other._p}
        {
        }

        filter_iterator& operator++()
        {
          ++_it;
          while (_it != _it_end && !_p(*_it))
          {
            ++_it;
          }
          return *this;
        }

        filter_iterator operator++(int)
        {
          filter_iterator tmp = *this;

          _it++;
          if (_it != _it_end && !_p(*_it))
          {
            _it++;
          }

          return tmp;
        }

        filter_iterator& operator--()
        {
          --_it;
          while (_it != _it_begin && !_p(*_it))
          {
            --_it;
          }
          return *this;
        }

        filter_iterator operator--(int)
        {
          filter_iterator tmp = *this;

          _it--;
          if (_it != _it_begin && !_p(*_it))
          {
            _it--;
          }

          return tmp;
        }

        filter_iterator& operator+=(size_t n)
        {
          for (size_t i = 0; i < n; i++)
          {
            if (_it != _it_end)
            {
              ++(*this);
            }
          }

          return *this;
        }

        filter_iterator& operator-=(size_t n)
        {
          for (size_t i = 0; i < n; i++)
          {
            if (_it != _it_begin)
            {
              --(*this);
            }
          }

          return *this;
        }

        filter_iterator& operator=(const filter_iterator& other)
        {
          _it = other._it;
          _it_begin = other._it_begin;
          _it_end = other._it_end;
          ETL_ASSERT(&_p == &other._p, ETL_ERROR_GENERIC("Predicates need to be the same"));
          return *this;
        }

        value_type operator*()
        {
          if (_it != _it_end && !_p(*_it))
          {
            ++_it;
          }
          return *_it;
        }

        bool operator==(const filter_iterator& other) const
        {
          return other._it == _it;
        }

        bool operator!=(const filter_iterator& other) const
        {
          return !(*this == other);
        }

      private:
        const_iterator _it;
        const_iterator _it_begin;
        const_iterator _it_end;
        const Pred& _p;
    };

    template<class Range, class Pred>
    constexpr typename filter_iterator<Range, Pred>::difference_type operator-(const filter_iterator<Range, Pred>& lhs, const filter_iterator<Range, Pred>& rhs)
    {
      typename filter_iterator<Range, Pred>::difference_type result{0};
      filter_iterator<Range, Pred> it_up{rhs};
      while (it_up != lhs)
      {
        ++it_up;
        ++result;
      }
      return result;
    }

    template<class Range, typename Pred>
    class filter_view: public etl::ranges::view_interface<filter_view<Range, Pred>>
    {
      public:
        using iterator = filter_iterator<Range, Pred>;
        using const_iterator = filter_iterator<Range, Pred>;

        filter_view(Range&& r, const Pred& pred): _pred{pred}, _r{etl::move(r)}
        {
        }

        constexpr Range& base() const&
        {
          return _r;
        }

        constexpr const Pred& pred() const
        {
          return _pred;
        }

        constexpr const_iterator begin() const
        {
          return const_iterator(ETL_OR_STD::cbegin(_r), ETL_OR_STD::cend(_r), _pred);
        }

        constexpr const_iterator end() const
        {
          return const_iterator(ETL_OR_STD::cend(_r), ETL_OR_STD::cend(_r), _pred);
        }

      private:
        const Pred _pred;
        Range _r;
    };

    template<class Range, typename Pred>
    filter_view(Range&&, Pred) -> filter_view<views::all_t<Range>, Pred>;

    template<typename Pred>
    struct filter_range_adapter_closure: public range_adapter_closure<filter_range_adapter_closure<Pred>>
    {
      template<typename Range>
      using target_view_type = filter_view<Range, Pred>;

      filter_range_adapter_closure(const Pred& p): _p{p}
      {
      }

      template<typename Range>
      constexpr auto operator()(Range&& r)
      {
        return filter_view(views::all(etl::forward<Range>(r)), _p);
      }

      const Pred _p;
    };

    namespace views
    {
      namespace private_views
      {
        struct filter
        {
          template<class Range, typename Pred>
          constexpr auto operator()(Range&& r, const Pred& p) const
          {
            return filter_view(views::all(etl::forward<Range>(r)), p);
          }

          template<typename Pred>
          constexpr auto operator()(const Pred& p) const
          {
            return ranges::filter_range_adapter_closure<Pred>(p);
          }
        };
      }

      inline constexpr private_views::filter filter;
    }

    template<class Range, class Fun>
    class transform_iterator
    {
      public:
        using trait = typename etl::ranges::private_ranges::iterator_trait<Range>;

        using iterator = typename trait::iterator;
        using const_iterator = typename trait::const_iterator;
        using value_type = typename trait::value_type;
        using difference_type = typename trait::difference_type;
        using pointer = typename trait::pointer;
        using reference = typename trait::reference;

        using iterator_category =  ETL_OR_STD::random_access_iterator_tag;

        transform_iterator(const_iterator it, Fun f): _it(it), _f(f)
        {
        }

        transform_iterator(const transform_iterator& other): _it{other._it}, _f{other._f}
        {
        }

        transform_iterator& operator++()
        {
          ++_it;
          return *this;
        }

        transform_iterator operator++(int)
        {
          transform_iterator tmp = *this;
          _it++;
          return tmp;
        }

        transform_iterator& operator=(const transform_iterator& other)
        {
          _it = other._it;
          _f = other._f;
        }

        value_type operator*()
        {
          return _f(*_it);
        }

        bool operator==(const transform_iterator& other) const
        {
          return other._it == _it;
        }

        bool operator!=(const transform_iterator& other) const
        {
          return !(*this == other);
        }

      private:
        const_iterator _it;
        Fun _f;
    };

    template<class Range, typename Fun>
    class transform_view: public etl::ranges::view_interface<transform_view<Range, Fun>>
    {
      public:
        using iterator = transform_iterator<Range, Fun>;
        using const_iterator = transform_iterator<Range, Fun>;

        transform_view(Range&& r, Fun fun): _fun{fun}, _r{etl::move(r)}
        {
        }

        constexpr Range& base() const&
        {
          return _r;
        }

        constexpr const_iterator begin() const
        {
          return const_iterator(ETL_OR_STD::cbegin(_r), _fun);
        }

        constexpr const_iterator end() const
        {
          return const_iterator(ETL_OR_STD::cend(_r), _fun);
        }

        constexpr size_t size() const
        {
          return etl::distance(ETL_OR_STD::cbegin(_r), ETL_OR_STD::cend(_r));
        }
      private:
        Fun _fun;
        Range _r;
    };

    template<class Range, typename Fun>
    transform_view(Range&&, Fun) -> transform_view<views::all_t<Range>, Fun>;

    template<typename Fun>
    struct transform_range_adapter_closure: public range_adapter_closure<transform_range_adapter_closure<Fun>>
    {
      template<typename Range>
      using target_view_type = transform_view<Range, Fun>;

      transform_range_adapter_closure(Fun& f): _f{f}
      {
      }

      template<typename Range>
      constexpr auto operator()(Range&& r)
      {
        return transform_view(views::all(etl::forward<Range>(r)), _f);
      }

      Fun _f;
    };

    namespace views
    {
      namespace private_views
      {
        struct transform
        {
          template<class Range, typename Fun>
          constexpr auto operator()(Range&& r, Fun&& f) const
          {
            return transform_view(views::all(etl::forward<Range>(r)), etl::forward<Fun>(f));
          }

          template<typename Fun>
          constexpr auto operator()(Fun& f) const
          {
            return ranges::transform_range_adapter_closure<Fun>(f);
          }
        };
      }

      inline constexpr private_views::transform transform;
    }

    template<class Range>
    class as_rvalue_view: public etl::ranges::view_interface<as_rvalue_view<Range>>
    {
      public:

        using iterator = typename etl::move_iterator<typename etl::ranges::private_ranges::iterator_trait<Range>::iterator>;

        as_rvalue_view(const as_rvalue_view& other) = default;

        as_rvalue_view(Range&& r): _r{etl::move(r)}
        {
        }

        constexpr Range& base() const
        {
          return _r;
        }

        constexpr iterator begin() const
        {
          return iterator(ETL_OR_STD::begin(_r));
        }

        constexpr iterator end() const
        {
          return iterator(ETL_OR_STD::end(_r));
        }

        constexpr size_t size() const
        {
          return etl::distance(ETL_OR_STD::cbegin(_r), ETL_OR_STD::cend(_r));
        }

      private:
        Range _r;
    };

    template<class Range>
    as_rvalue_view(Range&&) -> as_rvalue_view<views::all_t<Range>>;

    struct as_rvalue_range_adapter_closure: public range_adapter_closure<as_rvalue_range_adapter_closure>
    {
      template<typename Range>
      using target_view_type = as_rvalue_view<Range>;

      as_rvalue_range_adapter_closure() = default;

      template<typename Range>
      constexpr auto operator()(Range&& r)
      {
        return as_rvalue_view(views::all(etl::forward<Range>(r)));
      }
    };

    namespace views
    {
      namespace private_views
      {
        struct as_rvalue
        {
          template<class Range>
          constexpr auto operator()(Range&& r) const
          {
            return as_rvalue_view(views::all(etl::forward<Range>(r)));
          }

          constexpr auto operator()() const
          {
            return ranges::as_rvalue_range_adapter_closure();
          }
        };
      }

      inline constexpr private_views::as_rvalue as_rvalue;
    }

    template<class Range>
    class reverse_view: public etl::ranges::view_interface<reverse_view<Range>>
    {
      public:
        using iterator = ETL_OR_STD::reverse_iterator<typename etl::ranges::private_ranges::iterator_trait<Range>::iterator>;
        using const_iterator = ETL_OR_STD::reverse_iterator<typename etl::ranges::private_ranges::iterator_trait<Range>::const_iterator>;
        using difference_type = typename etl::ranges::private_ranges::iterator_trait<Range>::difference_type;

        constexpr reverse_view(Range&& r): _r{etl::move(r)}
        {
        }

        reverse_view(const reverse_view& other) = default;

        constexpr Range base() const&
        {
          return _r;
        }

        constexpr iterator begin() const
        {
          return iterator(ETL_OR_STD::end(_r));
        }

        constexpr iterator end() const
        {
          return iterator(ETL_OR_STD::begin(_r));
        }

        constexpr size_t size() const
        {
          return etl::distance(ETL_OR_STD::begin(_r), ETL_OR_STD::end(_r));
        }

      private:
        Range _r;
    };

    template<class Range>
    reverse_view(Range&&) -> reverse_view<views::all_t<Range>>;

    template<typename T>
    struct is_reverse_view : etl::false_type
    {
    };

    template<typename Range>
    struct is_reverse_view<reverse_view<Range>> : etl::true_type
    {
    };

    namespace views
    {
      namespace private_views
      {
        struct reverse
        {
          template<class Range>
          constexpr auto operator()(Range&& r) const
          {
            if constexpr (is_reverse_view<etl::remove_cv_t<etl::remove_reference_t<Range>>>::value)
            {
              return r.base();
            }
            else
            {
              return reverse_view(views::all(etl::forward<Range>(r)));
            }
          }
        };
      }

      inline constexpr private_views::reverse reverse;
    }

    template<class Range>
    class drop_view: public etl::ranges::view_interface<drop_view<Range>>
    {
      public:
        using iterator = typename etl::ranges::private_ranges::iterator_trait<Range>::iterator;
        using const_iterator = typename etl::ranges::private_ranges::iterator_trait<Range>::const_iterator;
        using difference_type = typename etl::ranges::private_ranges::iterator_trait<Range>::difference_type;

        constexpr drop_view(Range&& r, size_t drop_n)
        : _r{etl::move(r)}, _drop_n{drop_n}, _begin_cache{}
        {
        }

        drop_view(const drop_view& other) = default;

        constexpr Range base() const&
        {
          return _r;
        }

        constexpr iterator begin() const
        {
          if (!_begin_cache)
          {
            _begin_cache = drop_begin();
          }
          return *_begin_cache;
        }

        constexpr iterator end() const
        {
          return iterator(ETL_OR_STD::end(_r));
        }

        constexpr size_t size() const
        {
          if (!_begin_cache)
          {
            _begin_cache = drop_begin();
          }
          return etl::distance(*_begin_cache, ETL_OR_STD::end(_r));
        }

      private:
        constexpr iterator drop_begin() const
        {
          iterator result {ETL_OR_STD::end(_r)};
          if (static_cast<difference_type>(_drop_n) < etl::distance(ETL_OR_STD::begin(_r), ETL_OR_STD::end(_r)))
          {
            result = ETL_OR_STD::begin(_r);
            etl::advance(result, _drop_n);
          }
          return result;
        }

        Range _r;
        size_t _drop_n;
        mutable etl::optional<iterator> _begin_cache;
    };

    template<class Range>
    drop_view(Range&&) -> drop_view<views::all_t<Range>>;

    struct drop_range_adapter_closure: public range_adapter_closure<drop_range_adapter_closure>
    {
      template<typename Range>
      using target_view_type = drop_view<Range>;

      constexpr drop_range_adapter_closure(size_t drop_n): _drop_n{drop_n}
      {
      }

      template<typename Range>
      constexpr auto operator()(Range&& r) const
      {
        return drop_view(views::all(etl::forward<Range>(r)), _drop_n);
      }

      const size_t _drop_n;
    };

    namespace views
    {
      namespace private_views
      {
        struct drop
        {
          template<class Range>
          constexpr auto operator()(Range&& r, size_t drop_n) const
          {
            return drop_view(views::all(etl::forward<Range>(r)), drop_n);
          }

          constexpr auto operator()(size_t drop_n) const
          {
            return ranges::drop_range_adapter_closure(drop_n);
          }
        };
      }

      inline constexpr private_views::drop drop;
    }

    template<class Range, class Pred>
    class drop_while_view: public etl::ranges::view_interface<drop_while_view<Range, Pred>>
    {
      public:
        using const_iterator = typename etl::ranges::private_ranges::iterator_trait<Range>::const_iterator;
        using difference_type = typename etl::ranges::private_ranges::iterator_trait<Range>::difference_type;

        constexpr drop_while_view(Range&& r, Pred pred)
        : _r{etl::move(r)}, _pred{pred}, _begin_cache{}
        {
        }

        constexpr const Range base() const&
        {
          return _r;
        }

        constexpr Pred& pred() const
        {
          return _pred;
        }

        constexpr const_iterator begin() const
        {
          if (!_begin_cache)
          {
            const_iterator result{ETL_OR_STD::cbegin(_r)};
            while (result != ETL_OR_STD::cend(_r) && _pred(*result))
            {
              ++result;
            }
            _begin_cache = result;
          }
          return *_begin_cache;
        }

        constexpr const_iterator end() const
        {
          return const_iterator(ETL_OR_STD::cend(_r));
        }

      private:
        Range _r;
        Pred _pred;
        mutable etl::optional<const_iterator> _begin_cache;
    };

    template<class Range, class Pred>
    drop_while_view(Range&&, Pred) -> drop_while_view<views::all_t<Range>, Pred>;

    template<typename Pred>
    struct drop_while_range_adapter_closure: public range_adapter_closure<drop_while_range_adapter_closure<Pred>>
    {
      template<typename Range>
      using target_view_type = drop_while_view<Range, Pred>;

      constexpr drop_while_range_adapter_closure(Pred& pred): _pred{pred}
      {
      }

      template<typename Range>
      constexpr auto operator()(Range&& r) const
      {
        return drop_while_view(views::all(etl::forward<Range>(r)), _pred);
      }

      Pred _pred;
    };

    namespace views
    {
      namespace private_views
      {
        struct drop_while
        {
          template<class Range, class Pred>
          constexpr auto operator()(Range&& r, Pred pred) const
          {
            return drop_while_view(views::all(etl::forward<Range>(r)), pred);
          }

          template<class Pred>
          constexpr auto operator()(Pred pred) const
          {
            return ranges::drop_while_range_adapter_closure(pred);
          }
        };
      }

      inline constexpr private_views::drop_while drop_while;
    }

    template<class Range>
    class take_view: public etl::ranges::view_interface<take_view<Range>>
    {
      public:
        using iterator = typename etl::ranges::private_ranges::iterator_trait<Range>::iterator;
        using const_iterator = typename etl::ranges::private_ranges::iterator_trait<Range>::const_iterator;
        using difference_type = typename etl::ranges::private_ranges::iterator_trait<Range>::difference_type;

        constexpr take_view(Range&& r, ranges::range_difference_t<Range> take_n)
        : _r{etl::move(r)}
        , _take_n{etl::min<ranges::range_difference_t<Range>>(take_n, etl::distance(ETL_OR_STD::cbegin(r), ETL_OR_STD::cend(r)))}
        {
        }

        take_view(const take_view& other) = default;

        constexpr Range base() const&
        {
          return _r;
        }

        constexpr iterator begin() const
        {
          return iterator(ETL_OR_STD::begin(_r));
        }

        constexpr iterator end() const
        {
          iterator result {begin()};
          etl::advance(result, _take_n);
          return result;
        }

        constexpr ranges::range_difference_t<Range> size() const
        {
          return _take_n;
        }

      private:
        Range _r;
        ranges::range_difference_t<Range> _take_n;
    };

    template<class Range>
    take_view(Range&&, ranges::range_difference_t<Range>) -> take_view<views::all_t<Range>>;

    struct take_range_adapter_closure: public range_adapter_closure<take_range_adapter_closure>
    {
      template<typename Range>
      using target_view_type = take_view<Range>;

      template<class DifferenceType>
      constexpr take_range_adapter_closure(DifferenceType take_n): _take_n{static_cast<size_t>(take_n)}
      {
      }

      template<typename Range>
      constexpr auto operator()(Range&& r) const
      {
        return take_view(views::all(etl::forward<Range>(r)), _take_n);
      }

      const size_t _take_n;
    };

    namespace views
    {
      namespace private_views
      {
        struct take
        {
          template<class Range>
          constexpr auto operator()(Range&& r, ranges::range_difference_t<Range> take_n) const
          {
            return take_view(views::all(etl::forward<Range>(r)), take_n);
          }

          template<class DifferenceType>
          constexpr auto operator()(DifferenceType take_n) const
          {
            return ranges::take_range_adapter_closure(take_n);
          }
        };
      }

      inline constexpr private_views::take take;
    }

    template<class Range, class Pred>
    class take_while_view: public etl::ranges::view_interface<take_while_view<Range, Pred>>
    {
      public:
        using const_iterator = typename etl::ranges::private_ranges::iterator_trait<Range>::const_iterator;
        using difference_type = typename etl::ranges::private_ranges::iterator_trait<Range>::difference_type;

        constexpr take_while_view(Range&& r, Pred pred)
        : _r{etl::move(r)}, _pred{etl::move(pred)}, _end_cache{}
        {
        }

        constexpr const Range base() const&
        {
          return _r;
        }

        constexpr Pred& pred() const
        {
          return _pred;
        }

        constexpr const_iterator begin() const
        {
          return const_iterator(ETL_OR_STD::cbegin(_r));
        }

        constexpr const_iterator end() const
        {
          if (!_end_cache)
          {
            const_iterator result{ETL_OR_STD::cbegin(_r)};
            while (result != ETL_OR_STD::cend(_r) && _pred(*result))
            {
              ++result;
            }
            _end_cache = result;
          }
          return *_end_cache;
        }

      private:
        Range _r;
        Pred _pred;
        mutable etl::optional<const_iterator> _end_cache;
    };

    template<class Range, class Pred>
    take_while_view(Range&&, Pred) -> take_while_view<views::all_t<Range>, Pred>;

    template<typename Pred>
    struct take_while_range_adapter_closure: public range_adapter_closure<take_while_range_adapter_closure<Pred>>
    {
      template<typename Range>
      using target_view_type = take_while_view<Range, Pred>;

      constexpr take_while_range_adapter_closure(Pred pred): _pred{etl::move(pred)}
      {
      }

      template<typename Range>
      constexpr auto operator()(Range&& r) const
      {
        return take_while_view(views::all(etl::forward<Range>(r)), _pred);
      }

      Pred _pred;
    };

    namespace views
    {
      namespace private_views
      {
        struct take_while
        {
          template<class Range, class Pred>
          constexpr auto operator()(Range&& r, Pred&& pred) const
          {
            return take_while_view(views::all(etl::forward<Range>(r)), etl::forward<Pred>(pred));
          }

          template<class Pred>
          constexpr auto operator()(Pred&& pred) const
          {
            return ranges::take_while_range_adapter_closure(etl::forward<Pred>(pred));
          }
        };
      }

      inline constexpr private_views::take_while take_while;
    }

    template<class Range>
    class join_iterator
    {
      public:
        using trait = typename etl::ranges::private_ranges::iterator_trait<Range>;

        using iterator = typename trait::iterator;
        using const_iterator = typename trait::const_iterator;
        using difference_type = typename trait::difference_type;

        using iterator_category =  ETL_OR_STD::forward_iterator_tag;

        using InnerRange = decltype(*(ETL_OR_STD::begin(etl::declval<Range>())));
        using inner_trait = typename etl::ranges::private_ranges::iterator_trait<InnerRange>;
        using inner_iterator = typename inner_trait::iterator;

        using value_type = typename inner_trait::value_type;
        using pointer = typename inner_trait::pointer;
        using reference = typename inner_trait::reference;

        join_iterator(iterator it, iterator it_end)
         : _it(it)
         , _it_end(it_end)
         , _inner_it(it != it_end ? ETL_OR_STD::begin(*it) : inner_iterator{})
         , _inner_it_end(it != it_end ? ETL_OR_STD::end(*it) : inner_iterator{})
        {
          adjust_iterator();
        }

        join_iterator(const join_iterator& other) = default;

        join_iterator& operator++()
        {
          if (_inner_it != _inner_it_end)
          {
            ++_inner_it;
          }

          adjust_iterator();

          return *this;
        }

        join_iterator operator++(int)
        {
          join_iterator tmp{*this};

          if (_inner_it != _inner_it_end)
          {
            _inner_it++;
          }

          adjust_iterator();

          return tmp;
        }

        join_iterator& operator=(const join_iterator& other)
        {
          _it = other._it;
          _it_end = other._it_end;
          _inner_it = other._inner_it;
          _inner_it_end = other._inner_it_end;

          adjust_iterator();

          return *this;
        }

        reference operator*() const
        {
          return *_inner_it;
        }

        constexpr bool operator==(const join_iterator& other) const
        {
          return (_it == other._it && _inner_it == other._inner_it) || (_it == _it_end && other._it == other._it_end);
        }

        constexpr bool operator!=(const join_iterator& other) const
        {
          return !(*this == other);
        }

      private:
        void adjust_iterator()
        {
          while (_it != _it_end && _inner_it == _inner_it_end)
          {
            ++_it;
            if (_it != _it_end)
            {
              _inner_it = ETL_OR_STD::begin((*_it));
              _inner_it_end = ETL_OR_STD::end((*_it));
            }
          }
        }

        iterator _it;
        iterator _it_end;
        inner_iterator _inner_it;
        inner_iterator _inner_it_end;
    };

    template<class Range>
    class join_view: public etl::ranges::view_interface<join_view<Range>>
    {
      public:
        using iterator = join_iterator<Range>;
        using const_iterator = join_iterator<Range>;

        join_view(Range&& r): _r{etl::move(r)}
        {
        }

        constexpr Range base() const&
        {
          return _r;
        }

        constexpr iterator begin() const
        {
          return iterator(ETL_OR_STD::begin(_r), ETL_OR_STD::end(_r));
        }

        constexpr iterator end() const
        {
          return iterator(ETL_OR_STD::end(_r), ETL_OR_STD::end(_r));
        }

      private:
        Range _r;
    };

    struct join_range_adapter_closure: public range_adapter_closure<join_range_adapter_closure>
    {
      template<typename Range>
      using target_view_type = join_view<Range>;

      join_range_adapter_closure() = default;

      template<typename Range>
      constexpr auto operator()(Range&& r)
      {
        return join_view(views::all(etl::forward<Range>(r)));
      }
    };

    template<class Range>
    explicit join_view(Range&&) -> join_view<views::all_t<Range>>;

    namespace views
    {
      namespace private_views
      {
        struct join
        {
          template<class Range>
          constexpr auto operator()(Range&& r) const
          {
            return join_view(views::all(etl::forward<Range>(r)));
          }

          constexpr auto operator()() const
          {
            return ranges::join_range_adapter_closure();
          }
        };
      }

      inline constexpr private_views::join join;
    }

    template<class Range, class Pattern>
    class join_with_iterator
    {
      public:
        using trait = typename etl::ranges::private_ranges::iterator_trait<Range>;

        using iterator = typename trait::iterator;
        using const_iterator = typename trait::const_iterator;
        using difference_type = typename trait::difference_type;

        using iterator_category =  ETL_OR_STD::forward_iterator_tag;

        using InnerRange = decltype(*(ETL_OR_STD::begin(etl::declval<Range>())));
        using inner_trait = typename etl::ranges::private_ranges::iterator_trait<InnerRange>;
        using inner_iterator = typename inner_trait::iterator;

        using value_type = typename inner_trait::value_type;
        using pointer = typename inner_trait::pointer;
        using reference = typename inner_trait::reference;

        using pattern_trait = typename etl::ranges::private_ranges::iterator_trait<Pattern>;
        using pattern_iterator = typename pattern_trait::iterator;
        using pattern_const_iterator = typename pattern_trait::const_iterator;

        join_with_iterator(iterator it, iterator it_end, const Pattern& pattern)
        : _it(it)
        , _it_end(it_end)
        , _inner_it(it != it_end ? ETL_OR_STD::begin(*it) : inner_iterator{})
        , _inner_it_end(it != it_end ? ETL_OR_STD::end(*it) : inner_iterator{})
        , _pattern(pattern)
        , _pattern_it(pattern.cend())
        , _pattern_it_end(pattern.cend())
        {
          adjust_iterator();
        }

        join_with_iterator(const join_with_iterator& other) = default;

        join_with_iterator& operator++()
        {
          if (_pattern_it != _pattern_it_end)
          {
            ++_pattern_it;
          }
          else if (_inner_it != _inner_it_end)
          {
            ++_inner_it;
          }

          adjust_iterator();

          return *this;
        }

        join_with_iterator operator++(int)
        {
          join_with_iterator tmp{*this};

          if (_pattern_it != _pattern_it_end)
          {
            _pattern_it++;
          }
          else if (_inner_it != _inner_it_end)
          {
            _inner_it++;
          }

          adjust_iterator();

          return tmp;
        }

        join_with_iterator& operator=(const join_with_iterator& other)
        {
          _it = other._it;
          _it_end = other._it_end;
          _inner_it = other._inner_it;
          _inner_it_end = other._inner_it_end;
          _pattern_it = other._pattern_it;
          _pattern_it_end = other._pattern_it_end;

          adjust_iterator();

          return *this;
        }

        value_type operator*() const
        {
          if (_pattern_it != _pattern_it_end)
          {
            return *_pattern_it;
          }
          return *_inner_it;
        }

        constexpr bool operator==(const join_with_iterator& other) const
        {
          return (_it == other._it && _inner_it == other._inner_it && _pattern_it == other._pattern_it) || (_it == _it_end);
        }

        constexpr bool operator!=(const join_with_iterator& other) const
        {
          return !(*this == other);
        }

      private:
        void adjust_iterator()
        {
          if (_it != _it_end && _inner_it == _inner_it_end && _pattern_it == _pattern_it_end)
          {
            ++_it;
            if (_it != _it_end)
            {
              _pattern_it = ETL_OR_STD::cbegin(_pattern);
              _pattern_it_end = ETL_OR_STD::cend(_pattern);
              _inner_it = ETL_OR_STD::begin(*_it);
              _inner_it_end = ETL_OR_STD::end(*_it);
            }
          }
        }

        iterator _it;
        iterator _it_end;
        inner_iterator _inner_it;
        inner_iterator _inner_it_end;
        const Pattern& _pattern;
        pattern_const_iterator _pattern_it;
        pattern_const_iterator _pattern_it_end;
    };

    template<class Range, class Pattern>
    class join_with_view: public etl::ranges::view_interface<join_with_view<Range, Pattern>>
    {
      public:
        using iterator = join_with_iterator<Range, Pattern>;
        using const_iterator = join_with_iterator<Range, Pattern>;

        join_with_view(Range&& r, Pattern&& pattern): _r{etl::move(r)}, _pattern{etl::move(pattern)}
        {
        }

        constexpr Range base() const&
        {
          return _r;
        }

        constexpr iterator begin() const
        {
          return iterator(ETL_OR_STD::begin(_r), ETL_OR_STD::end(_r), _pattern);
        }

        constexpr iterator end() const
        {
          return iterator(ETL_OR_STD::end(_r), ETL_OR_STD::end(_r), _pattern);
        }

      private:
        Range _r;
        Pattern _pattern;
    };

    // For range as separator
    template<class Range, class Pattern>
    join_with_view(Range&&, Pattern&&) -> join_with_view<views::all_t<Range>, views::all_t<Pattern>>;

    // For single value as separator
    template<class Range>
    join_with_view(Range&&, etl::ranges::range_value_t<etl::ranges::range_reference_t<Range>>)
     -> join_with_view<views::all_t<Range>, etl::ranges::single_view<etl::ranges::range_value_t<etl::ranges::range_reference_t<Range>>>>;

    namespace private_ranges
    {
      template<class Pattern>
      constexpr auto make_pattern(Pattern&& pattern)
      {
        if constexpr(etl::is_base_of_v<etl::ranges::view_interface<Pattern>, Pattern>)
        {
          return etl::forward<Pattern>(pattern);
        }
        else
        {
          return etl::ranges::single_view<Pattern>(etl::forward<Pattern>(pattern));
        }
      }

      template<class Pattern>
      constexpr auto make_pattern(const Pattern& pattern)
      {
        if constexpr(etl::is_array_v<etl::remove_reference_t<Pattern>> || etl::is_range_v<etl::remove_reference_t<Pattern>>)
        {
          return views::all(pattern);
        }
        else
        {
          return etl::ranges::single_view<etl::remove_reference_t<Pattern>>(pattern);
        }
      }
    }

    template<class Pattern>
    struct join_with_range_adapter_closure: public range_adapter_closure<join_with_range_adapter_closure<Pattern>>
    {
      template<typename Range>
      using target_view_type = join_with_view<Range, Pattern>;

      join_with_range_adapter_closure(const Pattern& pattern): _pattern(pattern)
      {
      }

      template<typename Range>
      constexpr auto operator()(Range&& r)
      {
        return join_with_view(views::all(etl::forward<Range>(r)), private_ranges::make_pattern<Pattern>(_pattern));
      }

      const Pattern& _pattern;
    };

    namespace views
    {
      namespace private_views
      {
        struct join_with
        {
          template<class Range, class Pattern>
          constexpr auto operator()(Range&& r, Pattern&& pattern) const
          {
            return join_with_view(views::all(etl::forward<Range>(r)), views::all(etl::ranges::private_ranges::make_pattern<Pattern>(etl::forward<Pattern>(pattern))));
          }

          template<class Pattern>
          constexpr auto operator()(const Pattern& pattern) const
          {
            return ranges::join_with_range_adapter_closure(pattern);
          }
        };
      }

      inline constexpr private_views::join_with join_with;
    }

    namespace views
    {
      namespace private_views
      {
        struct counted
        {
          template<class Iterator, class DifferenceType>
          constexpr auto operator()(Iterator&& it, DifferenceType&& count) const
          {
            using T = etl::decay_t<decltype(it)>;
            using D = etl::iter_difference_t<T>;

            // contiguous_iterator_tag not yet available
            //if constexpr(etl::is_same_v<typename etl::iterator_traits<Iterator>::iterator_category,ETL_OR_STD::contiguous_iterator_tag>)
            //{
            //  return etl::span(etl::to_address(it), static_cast<size_t>(static_cast<etl::iter_difference_t<T>>(count)));
            //}
            //else
            if constexpr(etl::is_same_v<typename etl::iterator_traits<Iterator>::iterator_category,ETL_OR_STD::random_access_iterator_tag>)
            {
              return etl::ranges::subrange(it, it + static_cast<D>(count));
            }
            else
            {
              return etl::ranges::subrange(etl::counted_iterator(it, count), etl::default_sentinel);
            }
          }
        };
      }

      inline constexpr private_views::counted counted;
    }

    template<class... Ranges>
    class concat_view;

    template<class... Ranges>
    class concat_iterator
    {
      static_assert(sizeof...(Ranges) > 0, "Type list must be non-empty");

      public:
        using types = typename etl::type_list<Ranges...>;
        using first_range = typename etl::type_list_type_at_index_t<types, 0>;
        using value_type = typename etl::ranges::private_ranges::iterator_trait<first_range>::value_type;
        using reference = typename etl::ranges::private_ranges::iterator_trait<first_range>::reference;
        using difference_type = ptrdiff_t;

        using iterator_variant_type = typename concat_view<Ranges...>::iterator_variant_type;

        concat_iterator(size_t index, concat_view<Ranges...>& view, iterator_variant_type current): _ranges_index{index}, _view(view), _current(current)
        {
        }

        concat_iterator(const concat_iterator& other) = default;

        constexpr reference operator*() const
        {
          return _view.get_value(_ranges_index, _current);
        }

        constexpr decltype(auto) operator[] (difference_type pos) const
        {
          auto tmp = *this;
          if (pos > 0)
          {
            for (difference_type i = 0; i < pos; ++i)
            {
              tmp._view.advance(tmp._ranges_index, tmp._current, 1);
            }
          }
          if (pos < 0)
          {
            for (difference_type i = 0; i < -pos; ++i)
            {
              tmp._view.advance(tmp._ranges_index, tmp._current, -1);
            }
          }
          return *tmp;
        }

        constexpr concat_iterator& operator++()
        {
          _view.advance(_ranges_index, _current, 1);
          return *this;
        }

        constexpr concat_iterator operator++(int)
        {
          auto result = *this;
          _view.advance(_ranges_index, _current, 1);
          return result;
        }

        constexpr concat_iterator& operator--()
        {
          _view.advance(_ranges_index, _current, -1);
          return *this;
        }

        constexpr concat_iterator operator--(int)
        {
          auto result = *this;
          _view.advance(_ranges_index, _current, -1);
          return result;
        }

        constexpr concat_iterator& operator+=(difference_type n)
        {
          for (difference_type i = 0; i < n; ++i)
          {
            _view.advance(_ranges_index, _current, 1);
          }
          return *this;
        }

        constexpr concat_iterator& operator-=(difference_type n)
        {
          for (difference_type i = 0; i < n; ++i)
          {
            _view.advance(_ranges_index, _current, -1);
          }
          return *this;
        }

        friend constexpr bool operator==(const concat_iterator<Ranges...>& x, etl::default_sentinel_t)
        {
          return x._ranges_index == x._view.number_of_ranges - 1 &&
            etl::get<x._view.number_of_ranges - 1>(x._current) == etl::get<x._view.number_of_ranges - 1>(x._view).end();
        }

        friend constexpr bool operator==(const concat_iterator<Ranges...>&x, const concat_iterator<Ranges...>&y)
        {
          return x._ranges_index == y._ranges_index && x._current.index() == y._current.index() && x._current == y._current;
        }

        friend constexpr bool operator!=(const concat_iterator<Ranges...>& x, etl::default_sentinel_t)
        {
          return !(x == etl::default_sentinel);
        }

        friend constexpr bool operator!=(const concat_iterator<Ranges...>&x, const concat_iterator<Ranges...>&y)
        {
          return !(x == y);
        }

      private:
        size_t _ranges_index;
        const concat_view<Ranges...>& _view;
        iterator_variant_type _current;
    };

    template<class... Ranges>
    class concat_view: public etl::ranges::view_interface<concat_view<Ranges...>>
    {
      static_assert(sizeof...(Ranges) > 0, "Type list must be non-empty");

      public:
        using types = typename etl::type_list<Ranges...>;
        using first_range = typename etl::type_list_type_at_index_t<types, 0>;
        using value_type = typename etl::ranges::private_ranges::iterator_trait<first_range>::value_type;
        using reference = typename etl::ranges::private_ranges::iterator_trait<first_range>::reference;
        using iterator = concat_iterator<Ranges...>;
        using const_iterator = concat_iterator<Ranges...>;
        using difference_type = typename etl::make_signed_t<size_t>;

        using iterator_variant_type = etl::variant<typename etl::ranges::private_ranges::iterator_trait<Ranges>::iterator ...>;

        static constexpr const size_t number_of_ranges = sizeof...(Ranges);

        constexpr concat_view(Ranges&&... r)
        : _r{etl::move(r)...}
        {
          set_delegates();
        }

        concat_view(const concat_view& other) = default;

        constexpr iterator begin()
        {
          iterator_variant_type current;
          current.template emplace<0>(etl::get<0>(_r).begin());
          return iterator{0, *this, current};
        }

        constexpr iterator end()
        {
          iterator_variant_type current;
          current.template emplace<number_of_ranges - 1>(etl::get<number_of_ranges - 1>(_r).end());
          return iterator{number_of_ranges - 1, *this, current};
        }

        constexpr size_t size() const
        {
          return get_size();
        }

      private:
        template <class... Rs>
        friend class concat_iterator;

        template<size_t n = 0>
        constexpr size_t get_size() const
        {
          if constexpr (n < etl::tuple_size_v<decltype(_r)>)
          {
            return etl::get<n>(_r).size() + get_size<n + 1>();
          }
          else
          {
            return 0;
          }
        }

        // helper to advance iterator index+iterator variant
        void advance(size_t& index, iterator_variant_type& current, difference_type n) const
        {
          advance_delegates[index](index, _r, current, n);
        }

        template<size_t i = 0>
        void set_delegates()
        {
          if constexpr(i < number_of_ranges)
          {
            advance_delegates[i] = [](size_t& index, const etl::tuple<Ranges...>& r, iterator_variant_type& current, difference_type n)
            {
              if (n > 0)
              {
                auto end = etl::get<i>(r).end();
                auto& it = etl::get<i>(current);
                if (it != end)
                {
                  ++it;
                }
                if (it == end)
                {
                  if constexpr (i + 1 < number_of_ranges)
                  {
                    current.template emplace<i + 1>(etl::get<i + 1>(r).begin());
                    index = i + 1;
                  }
                  else
                  {
                    // at end of last range
                    ETL_ASSERT(it == end && i + 1 == number_of_ranges, ETL_ERROR_GENERIC("Wrong iterator state at end"));
                  }
                }
              }
              if (n < 0)
              {
                auto begin = etl::get<i>(r).begin();
                auto& it = etl::get<i>(current);
                if (it == begin)
                {
                  if constexpr (i > 0)
                  {
                    current.template emplace<i - 1>(etl::get<i - 1>(r).end());
                    index = i - 1;

                    auto begin2 = etl::get<i - 1>(r).begin();
                    auto& it2 = etl::get<i - 1>(current);
                    if (it2 != begin2)
                    {
                      --it2;
                    }
                  }
                  else
                  {
                    // at beginning of first range
                    ETL_ASSERT(it == begin && i == 0, ETL_ERROR_GENERIC("Wrong iterator state at begin"));
                  }
                }
                else
                {
                  it--;
                }
              }
            };

            get_value_delegates[i] = [](const iterator_variant_type& current) -> reference {
              return *etl::get<i>(current);
            };

            set_delegates<i + 1>();
          }
        }

        reference get_value(size_t index, const iterator_variant_type& current) const
        {
          return get_value_delegates[index](current);
        }

        etl::tuple<Ranges...> _r;
        etl::array<etl::delegate<reference(const iterator_variant_type& /*current*/)>, number_of_ranges> get_value_delegates;
        etl::array<etl::delegate<void(size_t& /*index*/, const etl::tuple<Ranges...>& /*r*/, iterator_variant_type&/*current*/, difference_type/*n*/)>, number_of_ranges> advance_delegates;
    };

    template<class... Ranges>
    concat_view(Ranges&& ...) -> concat_view<views::all_t<Ranges>...>;

    struct concat_range_adapter_closure: public range_adapter_closure<concat_range_adapter_closure>
    {
      template<typename... Ranges>
      using target_view_type = concat_view<Ranges...>;

      constexpr concat_range_adapter_closure() = default;

      template<typename... Ranges>
      constexpr auto operator()(Ranges&&... r) const
      {
        return concat_view(views::all(etl::forward<Ranges>(r))...);
      }
    };

    namespace views
    {
      namespace private_views
      {
        struct concat
        {
          template<class... Ranges>
          constexpr auto operator()(Ranges&&... r) const
          {
            return concat_view(views::all(etl::forward<Ranges>(r))...);
          }
        };
      }

      inline constexpr private_views::concat concat;
    }

    namespace private_ranges
    {
    template <class C>
    struct to_range_adapter_closure: public range_adapter_closure<to_range_adapter_closure<C>>
    {
      template<class Range = void>
      using target_view_type = C;

      to_range_adapter_closure() = default;

      template<class Range>
      C operator()(const Range& r) const
      {
        using result_type = C;

        result_type result;

        for (auto i: r)
        {
          result.push_back(i);
        }

        return result;
      }

      template<class Range>
      C operator()(Range&& r)
      {
        using result_type = C;

        result_type result;

        for (auto&& i: r)
        {
          result.emplace_back(etl::move(i));
        }

        return result;
      }
    };
    }

    template<class C>
    private_ranges::to_range_adapter_closure<C> to()
    {
      return private_ranges::to_range_adapter_closure<C>();
    }

  }

  namespace views = ranges::views;
}

// For adapting real ranges like arrays, vectors
template<class Range,
         typename RangeAdaptorClosure,
         typename = etl::enable_if_t<(etl::is_array_v<Range> || etl::is_class_v<Range>)
                                     && !(etl::is_base_of_v<etl::ranges::view_interface<Range>,Range>)
                                     && !(etl::is_same_v<RangeAdaptorClosure,etl::ranges::owning_range_adapter_closure>)
                                     >>
auto
operator|(const Range& r, RangeAdaptorClosure rac)
{
  return rac(r);
}

// For adapting via view
template<class Range,
         typename RangeAdaptorClosure,
         typename = etl::enable_if_t<etl::is_base_of_v<etl::ranges::view_interface<Range>,Range>
                                     || etl::is_same_v<RangeAdaptorClosure,etl::ranges::owning_range_adapter_closure>>>
auto
operator|(Range&& r, RangeAdaptorClosure rac)
{
  return rac(etl::forward<Range>(r));
}

#endif

#endif
