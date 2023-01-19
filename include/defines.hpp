#ifndef DEFINES_HPP
#define DEFINES_HPP

#include <tuple>
#include <string>

#include <fmt/core.h>

#define casts(type, value) static_cast<type>(value)
#define castr(type, value) reinterpret_cast<type>(value)
#define castc(type, value) const_cast<type>(value)

template <typename... T>
class MultiType : std::tuple<T...>
{
  private:
    class MultiTypePtr
    {
      private:
        MultiType<T...>* target_;

      public:
        MultiTypePtr(MultiType<T...>* target)
            : target_(target)
        {
        }

        template <typename Q>
        operator Q()
        {
            return &std::get<std::remove_cv_t<std::remove_pointer_t<Q>>>(*target_);
        }
    };

  public:
    template <typename Q>
    Q& data()
    {
        return std::get<Q>(*this);
    }

    template <typename Q>
    const Q* ptr()
    {
        return &std::get<Q>(*this);
    }

    MultiType<T...>::MultiTypePtr ptr()
    {
        return MultiTypePtr(this);
    }

    template <typename Q>
    Q& operator=(const Q& var)
    {
        std::get<Q>(*this) = var;
        return std::get<Q>(*this);
    }

    template <typename Q>
    operator Q&()
    {
        return std::get<std::remove_cv_t<Q>>(*this);
    }

    MultiType<T...>::MultiTypePtr operator&()
    {
        return MultiTypePtr(this);
    }
};

template <typename T>
class SingleType
{
  private:
    T data_;

  public:
    T data()
    {
        return data_;
    }

    T* ptr()
    {
        return &data_;
    }
    T& operator=(const T& var)
    {
        data_ = var;
        return data_;
    }

    operator T&()
    {
        return data_;
    }

    T* operator&()
    {
        return &data_;
    }
};

#endif // DEFINES_HPP
