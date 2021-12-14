//! @file Delegator.h

// This file is part of Cantera. See License.txt in the top-level directory or
// at https://cantera.org/license.txt for license and copyright information.

#ifndef CT_DELEGATOR_H
#define CT_DELEGATOR_H

#include "cantera/base/global.h"
#include "cantera/base/ctexceptions.h"
#include <array>

namespace Cantera
{

//! Delegate member functions of a C++ class to externally-specified functions
/*
 * This base class provides functions for setting delegates for the member
 * functions of a C++ class at runtime. The purpose of this capability is to
 * allow the class to be extended using functions defined in any programming
 * language that provides a C API for calling functions in that language.
 *
 * Delegates are specified as std::function objects that are responsible for
 * encapsulating the data specific to the target language and calling the
 * appropriate function in the target language. The std::function has a
 * modified function signature compared to the method that it is replacing or
 * augmenting:
 * - Methods with no return value and scalar arguments are treated the same
 * - Methods with a return value have that value as the first reference argument
 *   of their delegate function, and return an int. The delegate should return
 *   zero if it does not set the arguments value, and a non-zero value if it
 *   does.
 * - Methods with pointers to arrays as arguments have an additional argument
 *   introduced to indicate the length of each array argument. This argument
 *   occurs either first, or after the return value reference, and is a
 *   std::array<size_t, N> where N is the number of array arguments.
 *
 * Delegated methods can be specified to either "replace" the original class's
 * method, or to run "before" or "after" the original method, using the `when`
 * parameter of the `setDelegate` method.
 */
class Delegator
{
public:
    //! Set delegates for member functions with the signature `void()`.
    void setDelegate(const std::string& name, const std::function<void()>& func,
                     const std::string& when)
    {
        if (!m_funcs_v.count(name)) {
            throw NotImplementedError("Delegator::setDelegate",
                "for function named '{}' with signature 'void()'.", name);
        }
        *m_funcs_v[name] = makeDelegate(func, when, *m_funcs_v[name]);
    }

    //! set delegates for member functions with the signature `void(bool)`
    void setDelegate(const std::string& name, const std::function<void(bool)>& func,
                     const std::string& when)
    {
        if (!m_funcs_v_b.count(name)) {
            throw NotImplementedError("Delegator::setDelegate",
                "for function named '{}' with signature 'void(bool)'.", name);
        }
        *m_funcs_v_b[name] = makeDelegate(func, when, *m_funcs_v_b[name]);
    }

    //! set delegates for member functions with the signature `void(double)`
    void setDelegate(const std::string& name, const std::function<void(double)>& func,
                     const std::string& when)
    {
        if (!m_funcs_v_d.count(name)) {
            throw NotImplementedError("Delegator::setDelegate",
                "for function named '{}' with signature 'void(double)'.", name);
        }
        *m_funcs_v_d[name] = makeDelegate(func, when, *m_funcs_v_d[name]);
    }

    //! Set delegates for member functions with the signature `void(double*)`
    void setDelegate(const std::string& name,
                     const std::function<void(std::array<size_t, 1>, double*)>& func,
                     const std::string& when)
    {
        if (!m_funcs_v_dp.count(name)) {
            throw NotImplementedError("Delegator::setDelegate",
                "for function named '{}' with signature 'void(double*)'.", name);
        }
        *m_funcs_v_dp[name] = makeDelegate(func, when, *m_funcs_v_dp[name]);
    }

    //! Set delegates for member functions with the signature `void(double, double*)`
    void setDelegate(
        const std::string& name,
        const std::function<void(std::array<size_t, 1>, double, double*)>& func,
        const std::string& when)
    {
        if (!m_funcs_v_d_dp.count(name)) {
            throw NotImplementedError("Delegator::setDelegate",
                "for function named '{}' with signature 'void(double, double*)'.",
                name);
        }
        *m_funcs_v_d_dp[name] = makeDelegate(func, when, *m_funcs_v_d_dp[name]);
    }

    //! Set delegates for member functions with the signature
    //! `void(double, double*, double*)`
    void setDelegate(
        const std::string& name,
        const std::function<void(std::array <size_t, 2>, double, double*, double*)>& func,
        const std::string& when)
    {
        if (!m_funcs_v_d_dp_dp.count(name)) {
            throw NotImplementedError("Delegator::setDelegate",
                "for function named '{}' with signature "
                "'void(double, double*, double*)'.", name);
        }
        *m_funcs_v_d_dp_dp[name] = makeDelegate(func, when, *m_funcs_v_d_dp_dp[name]);
    }

    //! Set delegates for member functions with the signature
    //! `void(double*, double*, double*)`
    void setDelegate(
        const std::string& name,
        const std::function<void(std::array<size_t, 3>, double*, double*, double*)>& func,
        const std::string& when)
    {
        if (!m_funcs_v_dp_dp_dp.count(name)) {
            throw NotImplementedError("Delegator::setDelegate",
                "for function named '{}' with signature "
                "'void(double*, double*, double*)'.", name);
        }
        *m_funcs_v_dp_dp_dp[name] = makeDelegate(func, when, *m_funcs_v_dp_dp_dp[name]);
    }

    //! Set delegates for member functions with the signature `string(size_t)`
    void setDelegate(const std::string& name,
                     const std::function<int(std::string&, size_t)>& func,
                     const std::string& when)
    {
        if (!m_funcs_s_sz.count(name)) {
            throw NotImplementedError("Delegator::setDelegate",
                "for function named '{}' with signature "
                "'string(size_t)'.", name);
        }
        *m_funcs_s_sz[name] = makeDelegate(func, when, m_base_s_sz[name]);
    }

    //! Set delegates for member functions with the signature `size_t(string)`
    void setDelegate(const std::string& name,
                     const std::function<int(size_t&, const std::string&)>& func,
                     const std::string& when)
    {
        if (!m_funcs_sz_csr.count(name)) {
            throw NotImplementedError("Delegator::setDelegate",
                "for function named '{}' with signature "
                "'size_t(const string&)'.", name);
        }
        *m_funcs_sz_csr[name] = makeDelegate(func, when, m_base_sz_csr[name]);
    }

protected:
    //! Install a function with the signature `void()` as being delegatable
    void install(const std::string& name, std::function<void()>& target,
                 const std::function<void()>& func)
    {
        target = func;
        m_funcs_v[name] = &target;
    }

    //! Install a function with the signature `void(bool)` as being delegatable
    void install(const std::string& name, std::function<void(bool)>& target,
                 const std::function<void(bool)>& func)
    {
        target = func;
        m_funcs_v_b[name] = &target;
    }

    //! Install a function with the signature `void(double)` as being delegatable
    void install(const std::string& name, std::function<void(double)>& target,
                 const std::function<void(double)>& func)
    {
        target = func;
        m_funcs_v_d[name] = &target;
    }

    //! Install a function with the signature `void(double*)` as being delegatable
    void install(const std::string& name,
                 std::function<void(std::array<size_t, 1>, double*)>& target,
                 const std::function<void(std::array<size_t, 1>, double*)>& func)
    {
        target = func;
        m_funcs_v_dp[name] = &target;
    }

    //! Install a function with the signature `void(double, double*)` as being delegatable
    void install(const std::string& name,
                 std::function<void(std::array<size_t, 1>, double, double*)>& target,
                 const std::function<void(std::array<size_t, 1>, double, double*)>& func)
    {
        target = func;
        m_funcs_v_d_dp[name] = &target;
    }

    //! Install a function with the signature `void(double, double*, double*)` as being
    //! delegatable
    void install(const std::string& name,
                 std::function<void(std::array<size_t, 2>, double, double*, double*)>& target,
                 const std::function<void(std::array<size_t, 2>, double, double*, double*)>& func)
    {
        target = func;
        m_funcs_v_d_dp_dp[name] = &target;
    }

    //! Install a function with the signature
    //! `void(double*, double*, double*)` as being delegatable
    void install(const std::string& name,
                 std::function<void(std::array<size_t, 3>, double*, double*, double*)>& target,
                 const std::function<void(std::array<size_t, 3>, double*, double*, double*)>& base)
    {
        target = base;
        m_funcs_v_dp_dp_dp[name] = &target;
    }

    //! Install a function with the signature `string(size_t)` as being delegatable
    void install(const std::string& name,
                 std::function<std::string(size_t)>& target,
                 const std::function<std::string(size_t)>& base)
    {
        target = base;
        m_funcs_s_sz[name] = &target;
        m_base_s_sz[name] = base;
    }

    //! Install a function with the signature `size_t(string)` as being delegatable
    void install(const std::string& name,
                 std::function<size_t(const std::string&)>& target,
                 const std::function<size_t(const std::string&)>& base)
    {
        target = base;
        m_funcs_sz_csr[name] = &target;
        m_base_sz_csr[name] = base;
    }

    //! Create a delegate for a function with no return value
    template <typename BaseFunc, class ... Args>
    std::function<void(Args ...)> makeDelegate(
        const std::function<void(Args ...)>& func,
        const std::string& when,
        BaseFunc base)
    {
        if (when == "before") {
            return [base, func](Args ... args) {
                func(args ...);
                base(args ...);
            };
        } else if (when == "after") {
            return [base, func](Args ... args) {
                base(args ...);
                func(args ...);
            };
        } else if (when == "replace") {
            return [func](Args ... args) {
                func(args ...);
            };
        } else {
            throw CanteraError("Delegator::makeDelegate",
                "'when' must be one of 'before', 'after', or 'replace';"
                " not '{}", when);
        }
    }

    //! Create a delegate for a function with a return value
    template <typename ReturnType, class ... Args>
    std::function<ReturnType(Args ...)> makeDelegate(
        const std::function<int(ReturnType&, Args ...)>& func,
        const std::string& when,
        const std::function<ReturnType(Args ...)>& base)
    {
        if (when == "before") {
            return [base, func](Args ... args) {
                // Call the provided delegate first. If it sets the return
                // value, return that, otherwise return the value from the
                // original method
                ReturnType ret;
                int done = func(ret, args ...);
                if (done) {
                    return ret;
                } else {
                    return base(args ...);
                }
            };
        } else if (when == "after") {
            return [base, func](Args ... args) {
                // Add the value returned by the original method and the
                // provided delegate
                ReturnType ret1 = base(args ...);
                ReturnType ret2;
                int done = func(ret2, args ...);
                if (done) {
                    return ret1 + ret2;
                } else {
                    return ret1;
                }
            };
        } else if (when == "replace") {
            return [func](Args ... args) {
                ReturnType ret;
                func(ret, args ...);
                return ret;
            };
        } else {
            throw CanteraError("Delegator::makeDelegate",
                "'when' must be one of 'before', 'after', or 'replace';"
                " not '{}", when);
        }
    }

    // Delegates with no return value
    std::map<std::string, std::function<void()>*> m_funcs_v;
    std::map<std::string, std::function<void(bool)>*> m_funcs_v_b;
    std::map<std::string, std::function<void(double)>*> m_funcs_v_d;
    std::map<std::string,
        std::function<void(std::array<size_t, 1>, double*)>*> m_funcs_v_dp;
    std::map<std::string,
        std::function<void(std::array<size_t, 1>, double, double*)>*> m_funcs_v_d_dp;
    std::map<std::string,
        std::function<void(std::array<size_t, 2>, double, double*, double*)>*> m_funcs_v_d_dp_dp;
    std::map<std::string,
        std::function<void(std::array<size_t, 3>, double*, double*, double*)>*> m_funcs_v_dp_dp_dp;

    // Delegates with a return value
    std::map<std::string,
        std::function<std::string(size_t)>> m_base_s_sz;
    std::map<std::string,
        std::function<std::string(size_t)>*> m_funcs_s_sz;

    std::map<std::string,
        std::function<size_t(const std::string&)>> m_base_sz_csr;
    std::map<std::string,
        std::function<size_t(const std::string&)>*> m_funcs_sz_csr;
};

}

#endif
