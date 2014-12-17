#!/usr/bin/env python
# -*- coding: utf-8 -*-

def license():
    return \
"""
/*******************************************************************************
  WEOS - Wrapper for embedded operating systems

  Copyright (c) 2013-2014, Manuel Freiberger
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  - Redistributions of source code must retain the above copyright notice, this
    list of conditions and the following disclaimer.
  - Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

"""


def argumentTuple(numArgs, maxArgs):
    s = ""
    s += "template <"
    if numArgs == maxArgs:
        s += ",\n          ".join(["typename A%d = argument_tuple_null_type" % i for i in xrange(numArgs)])
    else:
        s += ",\n          ".join(["typename A%d" % i for i in xrange(numArgs)])
    s += ">\n"
    s += "struct argument_tuple"
    if numArgs != maxArgs:
        s += "<"
        s += ",\n                      ".join(["A%d" % i for i in xrange(numArgs)] +
                       ["argument_tuple_null_type" for i in xrange(maxArgs - numArgs)])
        s += ">"
    s += "\n{\n"

    s += "    static const std::size_t size = %d;\n\n" % numArgs

    if numArgs:
        s += "    explicit argument_tuple("
        s += ",\n                            ".join(["const A%d& a%d" % (i,i) for i in xrange(numArgs)])
        s += ")\n"
        s += "        : "
        s += ",\n          ".join(["m_a%d(a%d)" % (i,i) for i in xrange(numArgs)])
        s += "\n    {\n    }\n\n"

    if numArgs:
        s += "    // Constructor with perfect forwarding\n"
        s += "    template <"
        s += ",\n              ".join(["typename T%d" % i for i in xrange(numArgs)])
        s += ">\n"
        s += "    explicit argument_tuple("
        s += ",\n                            ".join(["T%d&& t%d" % (i,i) for i in xrange(numArgs)])
        s += ")\n"
        s += "        : "
        s += ",\n          ".join(["m_a%d(WEOS_NAMESPACE::forward<T%d>(t%d))" % (i,i,i) for i in xrange(numArgs)])
        s += "\n    {\n    }\n\n"

    if numArgs:
        s += "    // Copy constructor\n"
        s += "    argument_tuple(const argument_tuple& other)\n"
        s += "        : "
        s += ",\n          ".join(["m_a%d(other.m_a%d)" % (i,i) for i in xrange(numArgs)])
        s += "\n    {\n    }\n\n"

    if numArgs:
        s += "    // Move constructor\n"
        s += "    argument_tuple(argument_tuple&& other)\n"
        s += "        : "
        s += ",\n          ".join(["m_a%d(WEOS_NAMESPACE::forward<A%d>(other.m_a%d))" % (i,i,i) for i in xrange(numArgs)])
        s += "\n    {\n    }\n\n"

    if numArgs:
        s += "    // Accessors\n"
    for i in xrange(numArgs):
        s += "    A%d& get(WEOS_NAMESPACE::integral_constant<std::size_t, %d>) { return m_a%d; }\n" % (i,i,i)
        s += "    const A%d& get(WEOS_NAMESPACE::integral_constant<std::size_t, %d>) const { return m_a%d; }\n\n" % (i,i,i)


    s += "private:\n"
    if numArgs:
        s += "    "
        s += "\n    ".join(["A%d m_a%d;" % (i,i) for i in xrange(numArgs)])
        s += "\n\n"

    #s += "    const argument_tuple& operator= (const argument_tuple&);\n\n"

    s += "};\n\n"
    return s

def argumentTupleNonMembers(maxArgs):
    s = ""
    s += "// --------------------------------------------------------------------\n"
    s += "//     Get the type at index \p TIndex.\n"
    s += "// --------------------------------------------------------------------\n"
    s += "template <std::size_t TIndex, typename T>\n"
    s += "struct argument_tuple_element;\n\n"

    s += "// Recursive case\n"
    s += "template <std::size_t TIndex,\n          "
    s += ",\n          ".join(["typename A%d" % i for i in xrange(maxArgs)])
    s += ">\n"
    s += "struct argument_tuple_element<TIndex, argument_tuple<"
    s += ", ".join(["A%d" % i for i in xrange(maxArgs)])
    s += "> >\n"
    s += "        : argument_tuple_element<TIndex - 1, argument_tuple<"
    s += ", ".join(["A%d" % i for i in xrange(1, maxArgs)])
    s += "> >\n"
    s += "{\n};\n\n"

    s += "// End of recursion\n"
    s += "template <"
    s += ",\n          ".join(["typename A%d" % i for i in xrange(maxArgs)])
    s += ">\n"
    s += "struct argument_tuple_element<0, argument_tuple<"
    s += ", ".join(["A%d" % i for i in xrange(maxArgs)])
    s += "> >\n"
    s += "{\n"
    s += "    typedef A0 type;\n"
    s += "};\n\n"

    s += "template <std::size_t TIndex,\n          "
    s += ",\n          ".join(["typename A%d" % i for i in xrange(maxArgs)])
    s += ">\n"
    s += "struct argument_tuple_element<TIndex, const argument_tuple<"
    s += ", ".join(["A%d" % i for i in xrange(maxArgs)])
    s += "> >\n"
    s += "{\n"
    s += "    typedef argument_tuple<"
    s += ", ".join(["A%d" % i for i in xrange(maxArgs)])
    s += "> tuple_type;\n"
    s += "    typedef typename WEOS_NAMESPACE::add_const<\n"
    s += "        typename argument_tuple_element<TIndex, tuple_type>::type>::type type;\n"
    s += "};\n\n"

    s += "// --------------------------------------------------------------------\n"
    s += "//     Get the element at \p TIndex.\n"
    s += "// --------------------------------------------------------------------\n"
    s += "template <std::size_t TIndex,\n          "
    s += ",\n          ".join(["typename A%d" % i for i in xrange(maxArgs)])
    s += ">\n"
    s += "inline\n"
    s += "typename argument_tuple_element<TIndex, argument_tuple<"
    s += ", ".join(["A%d" % i for i in xrange(maxArgs)])
    s += "> >::type&\n"
    s += "    get(argument_tuple<"
    s += ", ".join(["A%d" % i for i in xrange(maxArgs)])
    s += ">& t)\n"
    s += "{\n"
    s += "    return t.get(WEOS_NAMESPACE::integral_constant<std::size_t, TIndex>());\n"
    s += "}\n\n"
    s += "template <std::size_t TIndex,\n          "
    s += ",\n          ".join(["typename A%d" % i for i in xrange(maxArgs)])
    s += ">\n"
    s += "inline\n"
    s += "const typename argument_tuple_element<TIndex, argument_tuple<"
    s += ", ".join(["A%d" % i for i in xrange(maxArgs)])
    s += "> >::type&\n"
    s += "    get(const argument_tuple<"
    s += ", ".join(["A%d" % i for i in xrange(maxArgs)])
    s += ">& t)\n"
    s += "{\n"
    s += "    return t.get(WEOS_NAMESPACE::integral_constant<std::size_t, TIndex>());\n"
    s += "}\n\n"

    s += "// --------------------------------------------------------------------\n"
    s += "//     Get the size.\n"
    s += "// --------------------------------------------------------------------\n"
    s += "template <typename T>\n"
    s += "struct argument_tuple_size;\n\n"

    s += "template <"
    s += ",\n          ".join(["typename A%d" % i for i in xrange(maxArgs)])
    s += ">\n"
    s += "struct argument_tuple_size<argument_tuple<"
    s += ", ".join(["A%d" % i for i in xrange(maxArgs)])
    s += "> >\n"
    s += "        : WEOS_NAMESPACE::integral_constant<\n"
    s += "              std::size_t,\n"
    s += "              argument_tuple<"
    s += ", ".join(["A%d" % i for i in xrange(maxArgs)])
    s += ">::size>\n"
    s += "{\n};\n\n"

    s += "template <"
    s += ",\n          ".join(["typename A%d" % i for i in xrange(maxArgs)])
    s += ">\n"
    s += "struct argument_tuple_size<const argument_tuple<"
    s += ", ".join(["A%d" % i for i in xrange(maxArgs)])
    s += "> >\n"
    s += "        : WEOS_NAMESPACE::integral_constant<\n"
    s += "              std::size_t,\n"
    s += "              argument_tuple<"
    s += ", ".join(["A%d" % i for i in xrange(maxArgs)])
    s += ">::size>\n"
    s += "{\n};\n\n"

    return s

def forwardAsArgumentTuple(numArgs):
    s = ""
    if numArgs == 0:
        s += "inline\n"
        s += "argument_tuple<> forward_as_argument_tuple()\n"
        s += "{\n"
        s += "    return argument_tuple<>();\n"
        s += "}\n\n"
        return s

    s += "template <"
    s += ",\n          ".join(["typename A%d" % i for i in xrange(numArgs)])
    s += ">\n"
    s += "inline\n"
    s += "argument_tuple<"
    s += ",\n               ".join(["A%d&&" % i for i in xrange(numArgs)])
    s += ">\n"
    s += "forward_as_argument_tuple("
    s += ",\n                          ".join(["A%d&& a%d" % (i,i) for i in xrange(numArgs)])
    s += ")\n{\n"
    s += "    return argument_tuple<"
    s += ",\n                          ".join(["A%d&&" % i for i in xrange(numArgs)])
    s += ">(\n            "
    s += ",\n            ".join(["WEOS_NAMESPACE::forward<A%d>(a%d)" % (i,i) for i in xrange(numArgs)])
    s += ");\n"
    s += "}\n\n"

    return s

def placeholders(maxArgs):
    s = ""
    s += "namespace placeholders\n{\n\n"
    s += "template <int TIndex>\n"
    s += "struct placeholder\n{\n};\n\n"
    for i in xrange(maxArgs):
        s += "extern const placeholder<%d> _%d;\n" % (i+1,i+1)
    s += "} // namespace placeholders\n\n"
    s += "template <typename T>\n"
    s += "struct is_placeholder : WEOS_NAMESPACE::integral_constant<int, 0>\n{\n};\n\n"
    s += "template <int TIndex>\n"
    s += "struct is_placeholder<placeholders::placeholder<TIndex> >\n"
    s += "        : WEOS_NAMESPACE::integral_constant<int, TIndex>\n{\n};\n\n"
    return s

def unpackArgument():
    s = ""

    s += "struct placeholder_out_of_bounds;\n\n"

    s += "template <int TIndex, typename TArguments,\n"
    s += "          bool TValid = (TIndex < argument_tuple_size<TArguments>::value)>\n"
    s += "struct placeholder_bounds_checker\n"
    s += "{\n"
    s += "    typedef typename argument_tuple_element<TIndex, TArguments>::type type;\n"
    s += "};\n\n"

    s += "template <int TIndex, typename TArguments>\n"
    s += "struct placeholder_bounds_checker<TIndex, TArguments, false>\n"
    s += "{\n"
    s += "    typedef placeholder_out_of_bounds type;\n"
    s += "};\n\n"

    s += "template <typename TBound, typename TUnbound>\n"
    s += "struct unpacked_argument_type\n"
    s += "{\n"
    s += "    typedef TBound type;\n"
    s += "};\n\n"

    s += "template <int TIndex, typename TUnbound>\n"
    s += "struct unpacked_argument_type<placeholders::placeholder<TIndex>, TUnbound>\n"
    s += "{\n"
    s += "    typedef typename placeholder_bounds_checker<TIndex - 1, TUnbound>::type temp_type;\n"
    s += "    typedef typename WEOS_NAMESPACE::add_rvalue_reference<temp_type>::type type;\n"
    s += "};\n\n"

    s += "template <typename TBound>\n"
    s += "struct unpack_argument\n"
    s += "{\n"
    s += "    template <typename TType, typename TUnbound>\n"
    s += "    TType&& operator() (TType&& bound,\n"
    s += "                        TUnbound& unbound) const\n"
    s += "    {\n"
    s += "        return WEOS_NAMESPACE::forward<TType>(bound);\n"
    s += "    }\n"
    s += "};\n\n"

    s += "template <int TIndex>\n"
    s += "struct unpack_argument<placeholders::placeholder<TIndex> >\n"
    s += "{\n"
    s += "    typedef placeholders::placeholder<TIndex> bound_type;\n\n"
    s += "    template <typename TUnbound>\n"
    s += "    typename unpacked_argument_type<bound_type, TUnbound>::type operator() (\n"
    s += "        const bound_type& /*bound*/, TUnbound& unbound) const\n"
    s += "    {\n"
    s += "        return WEOS_NAMESPACE::forward<typename unpacked_argument_type<\n"
    s += "                                  bound_type, TUnbound>::type>(\n"
    s += "                    get<TIndex - 1>(unbound));\n"
    s += "    }\n"
    s += "};\n\n"

    return s


def memFnResult(numArgs, cv):
    s = ""

    s += "// Result of mem_fn(TResult (TClass::*) ("
    s += ", ".join(["A%d" % i for i in xrange(numArgs)])
    s += ") %s)\n" % cv

    s += "template <typename TResult,\n"
    s += "          typename TClass"
    if numArgs:
        s += ",\n          "
        s += ",\n          ".join(["typename A%d" % i for i in xrange(numArgs)])
    s += ">\n"
    s += "class MemFnResult<TResult (TClass::*) ("
    s += ", ".join(["A%d" % i for i in xrange(numArgs)])
    s += ") %s>\n" % cv
    s += "{\n"

    s += "public:\n"
    s += "    typedef TResult result_type;\n\n"
    s += "private:\n"

    s += "    typedef TResult (TClass::* mem_fn_t) ("
    s += ", ".join(["A%d" % i for i in xrange(numArgs)])
    s += ") %s;\n" % cv

    s += "    mem_fn_t m_pm;\n\n"

    ## Invoke via reference to derived
    s += "    // Helpers to differentiate between smart pointers and references/pointers to derived classes\n"
    s += "    template <typename TPointer"
    if numArgs:
        s += ",\n              "
        s += ",\n              ".join(["typename T%d" % i for i in xrange(numArgs)])
    s += ">\n"
    s += "    result_type call(TPointer&& object,\n"
    s += "                     const volatile TClass*"
    if numArgs:
        s += ",\n                     "
        s += ",\n                     ".join(["T%d&& t%d" % (i,i) for i in xrange(numArgs)])
    s += ") const\n"
    s += "    {\n"
    s += "        return (WEOS_NAMESPACE::forward<TPointer>(object).*m_pm)("
    if numArgs:
        s += "\n                "
        s += ",\n                ".join(["WEOS_NAMESPACE::forward<T%d>(t%d)" % (i,i) for i in xrange(numArgs)])
    s += ");\n"
    s += "    }\n\n"

    ## Invoke via smart pointer or pointer to derived
    s += "    template <typename TPointer"
    if numArgs:
        s += ",\n              "
        s += ",\n              ".join(["typename T%d" % i for i in xrange(numArgs)])
    s += ">\n"
    s += "    result_type call(TPointer&& ptr,\n"
    s += "                     const volatile void*"
    if numArgs:
        s += ",\n                     "
        s += ",\n                     ".join(["T%d&& t%d" % (i,i) for i in xrange(numArgs)])
    s += ") const\n"
    s += "    {\n"
    s += "        return ((*ptr).*m_pm)("
    s += ",\n                              ".join(["WEOS_NAMESPACE::forward<T%d>(t%d)" % (i,i) for i in xrange(numArgs)])
    s += ");\n"
    s += "    }\n\n"

    s += "public:\n"

    s += "    explicit constexpr MemFnResult(mem_fn_t pm)\n"
    s += "        : m_pm(pm)\n"
    s += "    {\n    }\n\n"

    ## template <typename T>
    ## result_type operator() (TClass& object, T&&... t);
    s += "    // Reference to object\n"
    if numArgs:
        s += "    template <"
        s += ",\n              ".join(["typename T%d" % i for i in xrange(numArgs)])
        s += ">\n"
    s += "    result_type operator() (%s TClass& object" % cv
    if numArgs:
        s += ",\n                            "
        s += ",\n                            ".join(["T%d&& t%d" % (i,i) for i in xrange(numArgs)])
    s += ") const\n"
    s += "    {\n"
    s += "        return (object.*m_pm)("
    s += ",\n                              ".join(["WEOS_NAMESPACE::forward<T%d>(t%d)" % (i,i) for i in xrange(numArgs)])
    s += ");\n"
    s += "    }\n\n"

    ## template <typename T>
    ## result_type operator() (TClass&& object, T&&... t);
    s += "    // Reference to movable object\n"
    if numArgs:
        s += "    template <"
        s += ",\n              ".join(["typename T%d" % i for i in xrange(numArgs)])
        s += ">\n"
    s += "    result_type operator() (%s TClass&& object" % cv
    if numArgs:
        s += ",\n                            "
        s += ",\n                            ".join(["T%d&& t%d" % (i,i) for i in xrange(numArgs)])
    s += ") const\n"
    s += "    {\n"
    s += "        return (WEOS_NAMESPACE::move(object).*m_pm)("
    s += ",\n                                          ".join(["WEOS_NAMESPACE::forward<T%d>(t%d)" % (i,i) for i in xrange(numArgs)])
    s += ");\n"
    s += "    }\n\n"

    ## template <typename T>
    ## result_type operator() (TClass* object, T&&... t);
    s += "    // Pointer to object\n"
    if numArgs:
        s += "    template <"
        s += ",\n              ".join(["typename T%d" % i for i in xrange(numArgs)])
        s += ">\n"
    s += "    result_type operator() (%s TClass* object" % cv
    if numArgs:
        s += ",\n                            "
        s += ",\n                            ".join(["T%d&& t%d" % (i,i) for i in xrange(numArgs)])
    s += ") const\n"
    s += "    {\n"
    s += "        return (object->*m_pm)("
    s += ",\n                               ".join(["WEOS_NAMESPACE::forward<T%d>(t%d)" % (i,i) for i in xrange(numArgs)])
    s += ");\n"
    s += "    }\n\n"

    ## template <typename TPointer, typename T>
    ## result_type operator() (TPointer&& object, T&&... t);
    s += "    // Smart pointer, reference/pointer to derived class\n"
    s += "    template <typename TPointer"
    if numArgs:
        s += ",\n              "
        s += ",\n              ".join(["typename T%d" % i for i in xrange(numArgs)])
    s += ">\n"
    s += "    result_type operator() (TPointer&& object"
    if numArgs:
        s += ",\n                            "
        s += ",\n                            ".join(["T%d&& t%d" % (i,i) for i in xrange(numArgs)])
    s += ") const\n"
    s += "    {\n"
    s += "        return call(WEOS_NAMESPACE::forward<TPointer>(object),\n"
    s += "                    &object"
    if numArgs:
        s += ",\n                    "
        s += ",\n                    ".join(["WEOS_NAMESPACE::forward<T%d>(t%d)" % (i,i) for i in xrange(numArgs)])
    s += ");\n"
    s += "    }\n\n"

    s += "};\n\n"
    return s

def bindResult(numArgs, maxArgs):
    s = ""
    s += "template <typename TResult, typename F"
    if numArgs:
        s += ",\n          "
        s += ",\n          ".join(["typename A%d" % i for i in xrange(numArgs)])
    s += ">\n"
    s += "struct BindResult<TResult, F("
    s += ", ".join(["A%d" % i for i in xrange(numArgs)])
    s += ")>\n{\n"

    s += "    typedef TResult result_type;\n\n"

    s += "    // The bound functor. To be used internally only.\n"
    s += "    typedef F _functor_type_;\n\n"

    s += "    // Constructor with perfect forwarding\n"
    if numArgs:
        s += "    template <"
        s += ",\n              ".join(["typename T%d" % i for i in xrange(numArgs)])
        s += ">\n"
    s += "    explicit BindResult(const F& f"
    if numArgs:
        s += ",\n                        "
        s += ",\n                        ".join(["T%d&& t%d" % (i,i) for i in xrange(numArgs)])
    s += ")\n"
    s += "        : m_functor(f)"
    if numArgs:
        s += ",\n          m_arguments("
        s += ",\n                      ".join(["WEOS_NAMESPACE::forward<T%d>(t%d)" % (i,i) for i in xrange(numArgs)])
        s += ")"
    s += "\n    {\n    }\n\n"

    s += "    // Copy construction\n"
    s += "    BindResult(const BindResult& other)\n"
    s += "        : m_functor(other.m_functor),\n"
    s += "          m_arguments(other.m_arguments)\n"
    s += "    {\n    }\n\n"

    s += "    // Move construction\n"
    s += "    BindResult(BindResult&& other)\n"
    s += "        : m_functor(WEOS_NAMESPACE::move(other.m_functor)),\n"
    s += "          m_arguments(WEOS_NAMESPACE::move(other.m_arguments))\n"
    s += "    {\n    }\n\n"

    def operator(nargs, qualifier):
        s = ""
        if nargs:
            s += "    template <"
            s += ",\n              ".join(["typename T%d" % i for i in xrange(nargs)])
            s += ">\n"
        s += "    result_type operator() ("
        s += ",\n                            ".join(["T%d&& t%d" % (i,i) for i in xrange(nargs)])
        s += ")"
        if qualifier:
            s += " " + qualifier
        s += "\n"
        s += "    {\n"
        s += "        return this->invoke<result_type>(\n"
        s += "                forward_as_argument_tuple("
        if nargs:
            s += "\n                    "
            s += ",\n                    ".join(["WEOS_NAMESPACE::forward<T%d>(t%d)" % (i,i) for i in xrange(nargs)])
        s += "));\n"
        s += "    }\n\n"
        return s

    for i in xrange(maxArgs + 1):
        s += operator(i, "")
        s += operator(i, "const")

    s += "private:\n"
    s += "    typedef argument_tuple<"
    s += ", ".join(["A%d" % i for i in xrange(numArgs)])
    s += "> arguments_type;\n\n"
    s += "    F m_functor;\n"
    s += "    arguments_type m_arguments;\n\n"

    s += "    struct dispatch_tag;\n\n"

    ## TODO: Remove the member function case from here.
    s += "    //! \\todo We can never have a member function pointer.\n"
    s += "    static_assert(!WEOS_NAMESPACE::is_member_function_pointer<F>::value,\n"
    s += "                  \"The callable has not been wrapped.\");\n\n"

    def invoke(void, memberFunction, qualifier):
        if numArgs == 0 and memberFunction:
            return ""

        s = ""

        s += "    // Invoke "
        if memberFunction:
            s += "member function pointer "
        else:
            s += "function pointer "

        if void:
            s += "and return void "
        else:
            s += "and return non-void "

        if qualifier:
            s += "(" + qualifier + " qualified).\n"
        else:
            s += "(unqualified).\n"

        s += "    template <typename TReturn, typename TUnbound>\n"
        s += "    TReturn invoke(\n"
        s += "            TUnbound&& unbound_args,\n"
        s += "            typename WEOS_NAMESPACE::enable_if<\n"
        if void:
            s += "                WEOS_NAMESPACE::is_same<TReturn, void>::value"
        else:
            s += "                !WEOS_NAMESPACE::is_same<TReturn, void>::value"
        if memberFunction:
            s += "\n                && WEOS_NAMESPACE::is_member_function_pointer<F>::value"
        elif numArgs:
            s += "\n                && !WEOS_NAMESPACE::is_member_function_pointer<F>::value"
        s += ",\n"
        s += "                dispatch_tag>::type* = 0)"
        if qualifier:
            s += " " + qualifier
        s += "\n"
        s += "    {\n"
        if void:
            s += "        // The bind expression returns void. Thus, ignore the return value\n"
            s += "        // of the functor.\n"
            s += "        "
        else:
            s += "        return "
        if memberFunction:
            s += "(*unpack_argument<\n"
            s += "                typename argument_tuple_element<0, arguments_type>::type>()(\n"
            s += "                    get<0>(m_arguments), unbound_args).*m_functor)("
        else:
            s += "m_functor("

        if memberFunction:
            startArg = 1
        else:
            startArg = 0
        if numArgs > startArg:
            s += "\n"
            for i in xrange(startArg, numArgs):
                if i > startArg:
                    s += ",\n"
                s += "            unpack_argument<\n"
                s += "                typename argument_tuple_element<%d, arguments_type>::type>()(\n" % i
                s += "                    get<%d>(m_arguments), unbound_args)" % i
        s += ");\n"
        s += "    }\n\n"
        return s

    for returnsVoid in [True, False]:
        for memFun in [False, True]:
            s += invoke(returnsVoid, memFun, "")
            s += invoke(returnsVoid, memFun, "const")

    s += "};\n\n"
    return s

def deduceResultType(numArgs):
    s = ""

    s += "// Function\n"
    def fun(ref, cv, klass=False):
        s = ""
        s += "template <typename R"
        if klass:
            s += ",\n          typename C"
            ref = "(C::*)"
        if numArgs:
            s += ",\n          "
            s += ",\n          ".join(["typename A%d" % i for i in xrange(numArgs)])
        s += ">\n"
        s += "struct deduce_result_type<detail::unspecified_type,\n"
        s += "                          R %s (" % ref
        s += ", ".join(["A%d" % i for i in xrange(numArgs)])
        s += ") %s>\n" % cv
        s += "{\n"
        s += "    typedef R type;\n"
        s += "};\n\n"
        return s

    s += fun("", "")

    s += "// Function reference\n"
    s += fun("(&)", "")

    s += "// Function pointer\n"
    s += fun("(*)", "")

    s += "// Member function pointer\n"
    s += fun("", "", True)
    s += fun("", "const", True)
    s += fun("", "volatile", True)
    s += fun("", "const volatile", True)
    return s


def bindHelper(numArgs, maxArgs):
    s = ""

    s += "template <typename TResult,\n"
    s += "          typename TCallable"
    if numArgs == maxArgs:
        s += ",\n          "
        s += ",\n          ".join(["typename A%d = bind_helper_null_type" % i for i in xrange(numArgs)])
    elif numArgs:
        s += ",\n          "
        s += ",\n          ".join(["typename A%d" % i for i in xrange(numArgs)])
    s += ">\n"
    s += "struct bind_helper"
    if numArgs != maxArgs:
        s += "<TResult, TCallable,\n                   "
        s += ",\n                   ".join(
                ["A%d" % i for i in xrange(numArgs)] +
                ["bind_helper_null_type" for i in xrange(maxArgs - numArgs)])
        s += ">"
    s += "\n{\n"
    s += "    // Deduce the result type.\n"
    s += "    typedef typename deduce_result_type<TResult, TCallable>::type result_type;\n"
    s += "    // A plain member pointer will be wrapped using mem_fn<>. This way we have a uniform calling syntax.\n"
    s += "    typedef MemberPointerWrapper<typename WEOS_NAMESPACE::decay<TCallable>::type> wrapper_type;\n"
    s += "    typedef typename wrapper_type::type functor_type;\n"
    s += "    typedef BindResult<result_type,\n"
    s += "                       functor_type("
    s += ",\n                                    ".join(["typename WEOS_NAMESPACE::decay<A%d>::type" % i for i in xrange(numArgs)])
    s += ")> type;\n"
    s += "};\n\n"
    return s

def bind(numArgs, withResult):
    s = ""

    if withResult:
        s += "template <typename TResult,\n"
        s += "          typename TCallable"
        resultType = "TResult"
    else:
        s += "template <typename TCallable"
        resultType = "detail::unspecified_type"
    if numArgs:
        s += ",\n          "
        s += ",\n          ".join(["typename A%d" % i for i in xrange(numArgs)])
    s += ">\n"
    s += "inline\n"
    s += "typename detail::bind_helper<%s,\n" % resultType
    s += "                             TCallable"
    if numArgs:
        s += ",\n                             "
        s += ",\n                             ".join(["A%d" % i for i in xrange(numArgs)])
    s += ">::type\n"
    s += "bind(TCallable&& f"
    if numArgs:
        s += ",\n     "
        s += ",\n     ".join(["A%d&& a%d" % (i,i) for i in xrange(numArgs)])
    s += ")\n{\n"
    s += "    typedef detail::bind_helper<%s,\n" % resultType
    s += "                                TCallable"
    if numArgs:
        s += ",\n                                "
        s += ",\n                                ".join(["A%d" % i for i in xrange(numArgs)])
    s += "> helper_type;\n"
    s += "    typedef typename helper_type::wrapper_type wrapper_type;\n"
    s += "    typedef typename helper_type::type bind_result_type;\n\n"

    s += "    return bind_result_type(wrapper_type::wrap(WEOS_NAMESPACE::forward<TCallable>(f))"
    if numArgs:
        s += ",\n                      "
        s += ",\n                      ".join(["WEOS_NAMESPACE::forward<A%d>(a%d)" % (i,i) for i in xrange(numArgs)])
    s += ");\n"
    s += "}\n\n"
    return s

## function<>

def typeErasedBindInvoker(maxArgs):
    s = ""

    s += "// Adapters for the implementation of the polymorphic function<>.\n"
    s += "// Required members:\n"
    s += "// static bool isEmpty(const F& f)\n"
    s += "//     ... checks if f is empty\n"
    s += "// static void init(SmallFunctorStorage& self, F&& f)\n"
    s += "//     ... inits self from f\n"
    s += "// static void manage(AdapterTask task, SmallFunctorStorage& self,\n"
    s += "//                    const SmallFunctorStorage* other)\n"
    s += "//     ... clones *other into self   if task == AdapterTask_Clone\n"
    s += "//     ... destroys self             if task == AdapterTask_Destroy\n"
    s += "// template <typename... TArgs>\n"
    s += "// static /*undefined*/ invoke(const SmallFunctorStorage& self, TArgs... args)\n"
    s += "//     ... invokes self with args\n"
    s += "\n\n"

    s += "enum AdapterTask\n"
    s += "{\n"
    s += "    AdapterTask_Clone,\n"
    s += "    AdapterTask_Destroy\n"
    s += "};\n\n"

    s += "typedef void (*manager_function)(AdapterTask task, SmallFunctorStorage& self, const SmallFunctorStorage* other);\n\n"

    s += "// An adapter which allows to use a function pointer in function<>.\n"
    s += "template <typename TSignature>\n"
    s += "struct FunctionPointerAdapter\n"
    s += "{\n"
    s += "};\n\n"

    s += "// An adapter which allows to use a bind expression in function<>.\n"
    s += "template <typename TBindResult>\n"
    s += "class BindAdapter\n"
    s += "{\n"

    s += "    static const std::size_t smallSize = sizeof(SmallFunctorStorage);\n"
    s += "    static const std::size_t smallAlign = alignment_of<SmallFunctorStorage>::value;\n\n"

    s += "    typedef typename TBindResult::_functor_type_ functor_type;\n"
    s += "    static const bool can_store_inplace =\n"
    s += "           sizeof(TBindResult) <= smallSize\n"
    s += "        && alignment_of<TBindResult>::value <= smallAlign\n"
    s += "        && (smallAlign % alignment_of<TBindResult>::value == 0);\n"
    s += "    typedef integral_constant<bool, can_store_inplace> store_inplace;\n\n"

    s += "    static void doInit(SmallFunctorStorage& self, const TBindResult& f, true_type)\n"
    s += "    {\n"
    s += "        new (self.get()) TBindResult(f);\n"
    s += "    }\n\n"

    s += "    static void doInit(SmallFunctorStorage& self, const TBindResult& f, false_type)\n"
    s += "    {\n"
    s += "        self.get<TBindResult*>() = new TBindResult(f);\n"
    s += "    }\n\n"

    s += "    // Clone a bind result which fits into the small functor storage.\n"
    s += "    static void doClone(SmallFunctorStorage& self, const SmallFunctorStorage& other, true_type)\n"
    s += "    {\n"
    s += "        new (self.get()) TBindResult(other.get<TBindResult>());\n"
    s += "    }\n\n"

    s += "    // Clone a bind result which does not fit into the small functor storage.\n"
    s += "    static void doClone(SmallFunctorStorage& self, const SmallFunctorStorage& other, false_type)\n"
    s += "    {\n"
    s += "        self.get<TBindResult*>() = new TBindResult(*other.get<TBindResult*>());\n"
    s += "    }\n\n"

    s += "    // Destroy a bind result which fits into the SFS.\n"
    s += "    static void doDestroy(SmallFunctorStorage& self, true_type)\n"
    s += "    {\n"
    s += "        self.get<TBindResult>().~TBindResult();\n"
    s += "    }\n\n"

    s += "    // Destroy a bind result which doesn't fit into the SFS.\n"
    s += "    static void doDestroy(SmallFunctorStorage& self, false_type)\n"
    s += "    {\n"
    s += "        delete self.get<TBindResult*>();\n"
    s += "    }\n\n"

    s += "public:\n"

    s += "    static bool isEmpty(const TBindResult&)\n"
    s += "    {\n"
    s += "        return false;\n"
    s += "    }\n\n"

    s += "    static void init(SmallFunctorStorage& self, const TBindResult& f)\n"
    s += "    {\n"
    s += "        doInit(self, f, store_inplace());\n"
    s += "    }\n\n"

    s += "    static void manage(AdapterTask task, SmallFunctorStorage& self, const SmallFunctorStorage* other)\n"
    s += "    {\n"
    s += "        switch (task)\n"
    s += "        {\n"
    s += "        case AdapterTask_Clone:\n"
    s += "            doClone(self, *other, store_inplace());\n"
    s += "        case AdapterTask_Destroy:\n"
    s += "            doDestroy(self, store_inplace());\n"
    s += "        }\n"
    s += "    }\n\n"

    def invoke(nargs):
        s = ""
        if nargs:
            s += "    template <"
            s += ",\n              ".join(["typename T%d" % i for i in xrange(nargs)])
            s += ">\n"
        s += "    static typename TBindResult::result_type invoke(\n"
        s += "            const SmallFunctorStorage& self"
        if nargs:
            s += ",\n            "
            s += ",\n            ".join(["T%d t%d" % (i,i) for i in xrange(nargs)])
        s += ")\n"
        s += "    {\n"
        s += "        const TBindResult* functor = can_store_inplace ? &self.get<TBindResult>() : self.get<TBindResult*>();\n"
        s += "        return (*const_cast<TBindResult*>(functor))("
        if nargs:
            s += "\n                    "
            s += ",\n                    ".join(["WEOS_NAMESPACE::forward<T%d>(t%d)" % (i,i) for i in xrange(nargs)])
        s += ");\n"
        s += "    }\n\n"
        return s

    for i in xrange(maxArgs + 1):
        s += invoke(i)

    s += "};\n\n"

    return s


"""
def functionBase():
    namespace detail\n
    {\n\n
    struct function_pointer_tag {};\n
    struct function_object_tag {};\n\n
    template <typename TCallable>\n
    struct callable_traits\n
    {\n
        typedef typename conditional<is_pointer<TCallable>::value,\n
                                     function_pointer_tag,\n
                                     function_object_tag>::type type;\n
    };\n\n
    } // namespace detail\n\n
"""

def function(numArgs):
    name = "function"
    s = ""
    s += "template <typename TResult"
    if numArgs:
        s += ",\n          "
        s += ",\n          ".join(["typename A%d" % i for i in xrange(numArgs)])
    s += ">\n"
    s += "class %s<TResult(" % name
    s += ", ".join(["A%d" % i for i in xrange(numArgs)])
    s += ")>\n{\n"

    s += "public:\n"
    s += "    typedef TResult result_type;\n\n"

    s += "    %s() noexcept\n" % name
    s += "        : m_invoker(0)\n"
    s += "    {\n"
    s += "    }\n\n"

    s += "    /*\n"
    s += "    template <typename TCallable>\n"
    s += "    %s(TCallable f)\n" % name
    s += "    {\n"
    s += "    }\n\n"
    s += "    */\n\n"

    s += "    %s(nullptr_t) noexcept\n" % name
    s += "        : m_invoker(0)\n"
    s += "    {\n"
    s += "    }\n\n"

    s += "    %s(const %s& other)\n" % (name, name)
    s += "        : m_invoker(0)\n"
    s += "    {\n"
    s += "        *this = other;\n"
    s += "    }\n\n"

    s += "    template <typename TSignature>\n"
    s += "    %s(const detail::BindResult<result_type, TSignature>& expr)\n" % name
    s += "        : m_invoker(0)\n"
    s += "    {\n"
    s += "        *this = expr;\n"
    s += "    }\n\n"

    s += "    ~%s()\n" % name
    s += "    {\n"
    s += "        release();\n"
    s += "    }\n\n"

    s += "    %s& operator= (const %s& other)\n" % (name, name)
    s += "    {\n"
    s += "        if (this != &other)\n"
    s += "        {\n"
    s += "            release();\n"
    s += "            if (other.m_invoker)\n"
    s += "            {\n"
    s += "                m_manager = other.m_manager;\n"
    s += "                m_manager(detail::AdapterTask_Clone, m_storage, &other.m_storage);\n"
    s += "                m_invoker = other.m_invoker;\n"
    s += "            }\n"
    s += "        }\n"
    s += "        return *this;\n"
    s += "    }\n\n"

    s += "    template <typename TSignature>\n"
    s += "    %s& operator= (const detail::BindResult<result_type, TSignature>& expr)\n" % name
    s += "    {\n"
    s += "        typedef detail::BindAdapter<detail::BindResult<result_type, TSignature> > adapter;\n\n"
    s += "        release();\n"
    s += "        adapter::init(m_storage, expr);\n"
    s += "        m_manager = &adapter::manage;\n"
    s += "        m_invoker = &adapter::"
    if numArgs == 0:
        s += "invoke;\n"
    else:
        s += "template invoke<"
        s += ",\n                                              ".join(["A%d" % i for i in xrange(numArgs)])
        s += ">;\n"
    s += "        return *this;\n"
    s += "    }\n\n"

    s += "    result_type operator() ("
    s += ",\n                            ".join(["A%d a%d" % (i,i) for i in xrange(numArgs)])
    s += ") const\n"
    s += "    {\n"
    s += "        WEOS_ASSERT(m_invoker);\n"
    s += "        return (*m_invoker)(m_storage"
    if numArgs:
        s += ",\n                            "
        s += ",\n                            ".join(["WEOS_NAMESPACE::forward<A%d>(a%d)" % (i,i) for i in xrange(numArgs)])
    s += ");\n"
    s += "    }\n\n"

    s += "    %s& operator= (nullptr_t)\n" % name
    s += "    {\n"
    s += "        release();\n"
    s += "        return *this;\n"
    s += "    }\n\n"

    s += "    /*explicit*/ operator bool() const noexcept\n"
    s += "    {\n"
    s += "        return m_invoker != 0;\n"
    s += "    }\n\n"

    s += "private:\n"
    s += "    typedef result_type (*invoker_type)(const detail::SmallFunctorStorage&"
    if numArgs:
        s += ",\n                                        "
        s += ",\n                                        ".join(["A%d" % i for i in xrange(numArgs)])
    s += ");\n\n"

    s += "    detail::SmallFunctorStorage m_storage;\n"
    s += "    detail::manager_function m_manager;\n"
    s += "    invoker_type m_invoker;\n\n"

    s += "    void release()\n"
    s += "    {\n"
    s += "        if (m_invoker)\n"
    s += "        {\n"
    s += "            m_manager(detail::AdapterTask_Destroy, m_storage, 0);\n"
    s += "            m_invoker = 0;\n"
    s += "        }\n"
    s += "    }\n"

    s += "};\n\n"
    return s


def staticFunction(numArgs):
    s = ""
    s += "template <typename TResult"
    if numArgs:
        s += ",\n          "
        s += ",\n          ".join(["typename A%d" % i for i in xrange(numArgs)])
    s += ",\n          std::size_t TStorageSize>\n"
    s += "class static_function<TResult("
    s += ", ".join(["A%d" % i for i in xrange(numArgs)])
    s += "), TStorageSize>\n{\n"

    s += "public:\n"
    s += "    typedef TResult result_type;\n\n"

    s += "    static_function() noexcept\n"
    s += "        : m_invoker(0)\n"
    s += "    {\n"
    s += "    }\n\n"

    s += "    static_function(nullptr_t) noexcept\n"
    s += "        : m_invoker(0)\n"
    s += "    {\n"
    s += "    }\n\n"

    s += "    template <typename TSignature>\n"
    s += "    static_function(const detail::BindResult<result_type, TSignature>& expr)\n"
    s += "        : m_invoker(0)\n"
    s += "    {\n"
    s += "        *this = expr;\n"
    s += "    }\n\n"

    s += "    ~static_function()\n"
    s += "    {\n"
    s += "        release();\n"
    s += "    }\n\n"

    s += "    template <typename TSignature>\n"
    s += "    static_function& operator= (const detail::BindResult<result_type, TSignature>& expr)\n"
    s += "    {\n"
    s += "        static_assert(sizeof(expr) <= TStorageSize,\n"
    s += "                      \"The bind expression is too large for this function.\");\n\n"
    s += "        typedef detail::BindAdapter<detail::BindResult<result_type, TSignature> > adapter;\n\n"
    s += "        release();\n"
    s += "        m_manager = &adapter::manage;\n"
    s += "        m_manager(detail::AdapterTask_Clone, &m_storage, &expr);\n"
    s += "        m_invoker = &adapter::"
    if numArgs == 0:
        s += "invoke;\n"
    else:
        s += "template invoke<"
        s += ",\n                                              ".join(["A%d" % i for i in xrange(numArgs)])
        s += ">;\n"
    s += "        return *this;\n"
    s += "    }\n\n"

    s += "    result_type operator() ("
    s += ",\n                            ".join(["A%d a%d" % (i,i) for i in xrange(numArgs)])
    s += ") const\n"
    s += "    {\n"
    s += "        WEOS_ASSERT(m_invoker);\n"
    s += "        return (*m_invoker)(&m_storage"
    if numArgs:
        s += ",\n                            "
        s += ",\n                            ".join(["WEOS_NAMESPACE::forward<A%d>(a%d)" % (i,i) for i in xrange(numArgs)])
    s += ");\n"
    s += "    }\n\n"

    s += "    static_function& operator= (nullptr_t)\n"
    s += "    {\n"
    s += "        release();\n"
    s += "        return *this;\n"
    s += "    }\n\n"

    s += "    /*explicit*/ operator bool() const noexcept\n"
    s += "    {\n"
    s += "        return m_invoker != 0;\n"
    s += "    }\n\n"

    s += "private:\n"
    s += "    typedef void* (*manager_type)(detail::AdapterTask, void*, const void*);\n"
    s += "    typedef result_type (*invoker_type)(void*"
    if numArgs:
        s += ",\n                                        "
        s += ",\n                                        ".join(["A%d" % i for i in xrange(numArgs)])
    s += ");\n\n"

    s += "    typename WEOS_NAMESPACE::aligned_storage<TStorageSize>::type m_storage;\n"
    s += "    manager_type m_manager;\n"
    s += "    invoker_type m_invoker;\n\n"

    s += "    void release()\n"
    s += "    {\n"
    s += "        if (m_invoker)\n"
    s += "        {\n"
    s += "            m_manager(detail::AdapterTask_Destroy, &m_storage, 0);\n"
    s += "            m_invoker = 0;\n"
    s += "        }\n"
    s += "    }\n"

    s += "static_function(const static_function&);\n"
    s += "static_function& operator= (const static_function&);\n"

    s += "};\n\n"
    return s



def generateHeader(maxArgs):
    s = ""

    s += license()

    s += "#ifndef WEOS_COMMON_FUNCTIONAL_HPP\n"
    s += "#define WEOS_COMMON_FUNCTIONAL_HPP\n\n"

    s += "#include \"../config.hpp\"\n\n"

    s += "#include \"../type_traits.hpp\"\n"
    s += "#include \"../utility.hpp\"\n\n"

    s += "WEOS_BEGIN_NAMESPACE\n\n"

    s += placeholders(maxArgs)

    ## namespace detail {
    s += "namespace detail\n{\n\n"

    s += "struct unspecified_type {};\n\n"
    s += "template <typename TResult, typename TF>\n"
    s += "struct result_traits\n"
    s += "{\n"
    s += "    typedef TResult type;\n"
    s += "};\n\n"

    s += "template <typename TF>\n"
    s += "struct result_traits<unspecified_type, TF>\n"
    s += "{\n"
    s += "    typedef typename TF::result_type type;\n"
    s += "};\n\n"

    s += "// ====================================================================\n"
    s += "// argument_tuple<>\n"
    s += "// ====================================================================\n\n"

    s += "struct argument_tuple_null_type;\n\n"
    for i in xrange(maxArgs + 1):
        s += argumentTuple(maxArgs - i, maxArgs)
    s += "\n"
    s += argumentTupleNonMembers(maxArgs)
    for i in xrange(maxArgs + 1):
        s += forwardAsArgumentTuple(i)
    s += unpackArgument()

    s += "// ====================================================================\n"
    s += "// MemFnResult\n"
    s += "// ====================================================================\n\n"

    # Forward declaration of MemFnResult
    s += "template <typename TMemberPointer>\n"
    s += "class MemFnResult;\n\n"

    # MemFnResult for pointer to (qualified) member functions
    for i in xrange(maxArgs + 1):
        s += memFnResult(i, "")
        s += memFnResult(i, "const")
        s += memFnResult(i, "volatile")
        s += memFnResult(i, "const volatile")

    # MemFnResult for pointer to member objects
    s += "// Result of mem_fn(TResult TClass::*)\n"
    s += "template <typename TResult, typename TClass>\n"
    s += "class MemFnResult<TResult TClass::*>\n"
    s += "{\n"

    s += "    typedef TResult TClass::* mem_fn_t;\n"
    s += "    mem_fn_t m_pm;\n\n"

    s += "public:\n"

    s += "    explicit constexpr MemFnResult(mem_fn_t pm) noexcept\n"
    s += "        : m_pm(pm)\n"
    s += "    {\n    }\n\n"

    s += "    TResult& operator() (TClass& object) const noexcept\n"
    s += "    {\n"
    s += "        return object.*m_pm;\n"
    s += "    }\n\n"

    s += "    constexpr\n"
    s += "    const TResult& operator() (const TClass& object) const noexcept\n"
    s += "    {\n"
    s += "        return object.*m_pm;\n"
    s += "    }\n\n"

    s += "    TResult&& operator()(TClass&& object) const noexcept\n"
    s += "    {\n"
    s += "        return WEOS_NAMESPACE::forward<TClass>(object).*m_pm;\n"
    s += "    }\n\n"

    s += "    const TResult&& operator()(const TClass&& object) const noexcept\n"
    s += "    {\n"
    s += "        return WEOS_NAMESPACE::forward<const TClass>(object).*m_pm;\n"
    s += "    }\n\n"

    s += "    TResult& operator() (TClass* object) const noexcept\n"
    s += "    {\n"
    s += "        return object->*m_pm;\n"
    s += "    }\n\n"

    s += "    constexpr\n"
    s += "    const TResult& operator() (const TClass* object) const noexcept\n"
    s += "    {\n"
    s += "        return object->*m_pm;\n"
    s += "    }\n\n"

    ## TODO: Smart pointers and pointers to derived classes
    s += "    // Smart pointer, reference/pointer to derived class\n"
    s += "    //! \\todo Missing\n"

    s += "};\n\n"

    s += "// ====================================================================\n"
    s += "// deduce_result_type\n"
    s += "// ====================================================================\n\n"

    s += "// Default case with explicit result type.\n"
    s += "template <typename TResult, typename TCallable>\n"
    s += "struct deduce_result_type\n"
    s += "{\n"
    s += "    typedef TResult type;\n"
    s += "};\n\n"

    for i in xrange(maxArgs):
        s += deduceResultType(i)

    s += "// ====================================================================\n"
    s += "// BindResult\n"
    s += "// ====================================================================\n\n"

    s += "// MemberPointerWrapper will wrap member pointers using mem_fn<>.\n"
    s += "// The default case does nothing.\n"
    s += "template <typename TType>\n"
    s += "struct MemberPointerWrapper\n"
    s += "{\n"
    s += "    typedef TType type;\n\n"
    s += "    static const TType& wrap(const TType& t)\n"
    s += "    {\n"
    s += "        return t;\n"
    s += "    }\n\n"
    s += "    static TType&& wrap(TType&& t)\n"
    s += "    {\n"
    s += "        return static_cast<TType&&>(t);\n"
    s += "    }\n"
    s += "};\n\n"

    s += "// In the special case of a member pointer, mem_fn<> is applied.\n"
    s += "template <typename TType, typename TClass>\n"
    s += "struct MemberPointerWrapper<TType TClass::*>\n"
    s += "{\n"
    s += "    typedef MemFnResult<TType TClass::*> type;\n\n"
    s += "    static type wrap(TType TClass::* pm)\n"
    s += "    {\n"
    s += "        return type(pm);\n"
    s += "    }\n"
    s += "};\n\n"

    s += "// When the best overload for bind<>() is determined, the compiler\n"
    s += "// instantiates MemberPointerWrapper<void>, which forms a reference\n"
    s += "// to void in turn. A solution is to provide a template\n"
    s += "// specialization for this case. It is never used, because there\n"
    s += "// are better matches for bind<>.\n"
    s += "template <>\n"
    s += "struct MemberPointerWrapper<void>\n"
    s += "{\n"
    s += "    typedef void type;\n"
    s += "};\n\n"

    s += "// The result of a bind<>() call.\n"
    s += "// The TSignature will be something of the form\n"
    s += "// TFunctor(TBoundArg0, TBoundArg1, ...),\n"
    s += "// where TFunctor can be a function pointer or a MemFnResult\n"
    s += "template <typename TResult, typename TSignature>\n"
    s += "struct BindResult;\n\n"
    for i in xrange(maxArgs + 1):
        s += bindResult(i, maxArgs)

    s += "\nstruct bind_helper_null_type;\n\n"
    for i in xrange(maxArgs + 1):
        s += bindHelper(maxArgs - i, maxArgs)

    s += "} // namespace detail\n\n"
    ## } namespace detail

    s += "// ====================================================================\n"
    s += "// mem_fn<>\n"
    s += "// ====================================================================\n\n"

    s += "template <typename TResult, typename TClass>\n"
    s += "inline\n"
    s += "detail::MemFnResult<TResult TClass::*> mem_fn(TResult TClass::* pm) noexcept\n"
    s += "{\n"
    s += "    return detail::MemFnResult<TResult TClass::*>(pm);\n"
    s += "}\n\n"

    s += "// ====================================================================\n"
    s += "// bind<>\n"
    s += "// ====================================================================\n\n"

    s += "// template <typename F, typename... TArgs>\n"
    s += "// /*unspecified*/ bind(F&& f, TArgs&&... args);\n"
    for i in xrange(maxArgs + 1):
        s += bind(i, False)

    s += "// template <typename R, typename F, typename... TArgs>\n"
    s += "// /*unspecified*/ bind(F&& f, TArgs&&... args);\n"
    for i in xrange(maxArgs + 1):
        s += bind(i, True)

    s += "// ====================================================================\n"
    s += "// function<>\n"
    s += "// ====================================================================\n\n"

    s += "namespace detail\n{\n\n"

    s += "class AnonymousClass;\n\n"

    s += "// A small functor.\n"
    s += "// This type is just large enough to hold a member function pointer\n"
    s += "// plus a pointer to an instance or a function pointer plus an\n"
    s += "// argument of pointer size.\n"
    s += "struct SmallFunctor\n"
    s += "{\n"
    s += "    union Callable\n"
    s += "    {\n"
    s += "        void* objectPointer;\n"
    s += "        const void* constObjectPointer;\n"
    s += "        void (*functionPointer)();\n"
    s += "        void* AnonymousClass::*memberDataPointer;\n"
    s += "        void (AnonymousClass::*memberFunctionPointer)();\n"
    s += "    };\n"
    s += "    union Argument\n"
    s += "    {\n"
    s += "        void* objectPointer;\n"
    s += "        const void* constObjectPointer;\n"
    s += "    };\n\n"
    s += "    Callable callable;\n"
    s += "    Argument argument;\n"
    s += "};\n"

    s += "struct SmallFunctorStorage\n"
    s += "{\n"
    s += "    void* get() { return &data; }\n"
    s += "    const void* get() const { return &data; }\n\n"

    s += "    template <typename T>\n"
    s += "    T& get() { return *static_cast<T*>(get()); }\n\n"

    s += "    template <typename T>\n"
    s += "    const T& get() const { return *static_cast<const T*>(get()); }\n\n"

    s += "    SmallFunctor data;\n"
    s += "};\n\n"

    s += typeErasedBindInvoker(maxArgs)
    s += "} // namespace detail\n\n"

    s += "template <typename TSignature>\n"
    s += "class function;\n\n"

    for i in xrange(maxArgs + 1):
        s += function(i)

    s += "// ====================================================================\n"
    s += "// static_function<>\n"
    s += "// ====================================================================\n\n"

    s += "template <typename TSignature,\n"
    s += "          std::size_t TStorageSize = WEOS_DEFAULT_STATIC_FUNCTION_SIZE>\n"
    s += "class static_function;\n\n"

    #for i in xrange(maxArgs + 1):
    #    s += staticFunction(i)

    s += "WEOS_END_NAMESPACE\n\n"

    s += "#endif // WEOS_COMMON_FUNCTIONAL_HPP\n\n"

    return s



def generateSource(maxArgs):
    s = ""

    s += license()

    s += "#include \"functional.hpp\"\n\n"

    s += "WEOS_BEGIN_NAMESPACE\n\n"
    s += "namespace placeholders\n{\n\n"

    for i in xrange(1, maxArgs + 1):
        s += "const placeholder<%d> _%d;\n" % (i,i)

    s += "} // namespace placeholders\n\n"
    s += "WEOS_END_NAMESPACE\n\n"

    return s



maxArgs = 4

print(generateHeader(maxArgs))

with open('functional.hpp', 'w') as fout:
    fout.write(generateHeader(maxArgs))

with open('functional.cpp', 'w') as fout:
    fout.write(generateSource(maxArgs))
