// https://stackoverflow.com/questions/19808054/convert-c-function-pointer-to-c-function-pointer/19809787
// https://stackoverflow.com/users/6829808/evgeniy-alexeev
// https://stackoverflow.com/help/licensing
// CC BY-SA 3.0.

#ifndef _CALLBACK_H_
#define _CALLBACK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <type_traits>
#include <functional>

template<typename T>
struct ActualType
{
    typedef T type;
};
template<typename T>
struct ActualType<T*>
{
    typedef typename ActualType<T>::type type;
};

template<typename T, unsigned int n, typename CallerType>
struct Callback;

template<typename Ret, typename ... Params, unsigned int n, typename CallerType>
struct Callback<Ret(Params...), n, CallerType>
{
    typedef Ret (*ret_cb)(Params...);
    template<typename ... Args>
    static Ret callback(Args ... args)
    {
        func(args...);
    }

    static ret_cb getCallback(std::function<Ret(Params...)> fn)
    {
        func = fn;
        return static_cast<ret_cb>(Callback<Ret(Params...), n, CallerType>::callback);
    }

    static std::function<Ret(Params...)> func;

};

template<typename Ret, typename ... Params, unsigned int n, typename CallerType>
std::function<Ret(Params...)> Callback<Ret(Params...), n, CallerType>::func;

#define GETCB(ptrtype, callertype) Callback<ActualType<ptrtype>::type, __COUNTER__, callertype>::getCallback

#ifdef __cplusplus
}
#endif

#endif /* _CALLBACK_H_ */
