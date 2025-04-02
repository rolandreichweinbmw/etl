#pragma once

#include "platform.h"

#include "etl/type_list.h"

namespace middleware
{

template <typename Ret, typename... Args>
struct Method
{
    using return_type = Ret;
    using arg_types   = etl::type_list<Args...>;
};

template <typename... Methods>
class Interface
{
};

template <typename Interface>
class Skeleton
{
};

template <typename Interface>
class Proxy
{
public:
    template<typename Arg>
    void serialize(Arg&& arg)
    {
        (void) arg;
    }
    template<typename Arg, typename ... Args>
    void serialize(Arg arg, Args&& ... args)
    {
        serialize(arg);
        serialize(etl::forward<Args>(args)...);
    }
    template<typename Method, typename ... Args>
    typename Method::return_type call(Args && ... args) {
        serialize(etl::forward<Args>(args)...);
        return typename Method::return_type{};
    }
};

}
