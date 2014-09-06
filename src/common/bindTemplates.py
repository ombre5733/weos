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
        s += ",\n                            ".join(["BOOST_FWD_REF(T%d) t%d" % (i,i) for i in xrange(numArgs)])
        s += ")\n"
        s += "        : "
        s += ",\n          ".join(["m_a%d(boost::forward<T%d>(t%d))" % (i,i,i) for i in xrange(numArgs)])
        s += "\n    {\n    }\n\n"

    if numArgs:
        s += "    // Copy constructor\n"
        s += "    argument_tuple(const argument_tuple& other)\n"
        s += "        : "
        s += ",\n          ".join(["m_a%d(other.m_a%d)" % (i,i) for i in xrange(numArgs)])
        s += "\n    {\n    }\n\n"

    if numArgs:
        s += "    // Move constructor\n"
        s += "    argument_tuple(BOOST_RV_REF(argument_tuple) other)\n"
        s += "        : "
        s += ",\n          ".join(["m_a%d(boost::forward<A%d>(other.m_a%d))" % (i,i,i) for i in xrange(numArgs)])
        s += "\n    {\n    }\n\n"

    if numArgs:
        s += "    // Accessors\n"
    for i in xrange(numArgs):
        s += "    A%d& get(boost::integral_constant<std::size_t, %d>) { return m_a%d; }\n" % (i,i,i)
        s += "    const A%d& get(boost::integral_constant<std::size_t, %d>) const { return m_a%d; }\n\n" % (i,i,i)


    s += "private:\n"
    if numArgs:
        s += "    "
        s += "\n    ".join(["A%d m_a%d;" % (i,i) for i in xrange(numArgs)])
        s += "\n\n"

    #s += "    const argument_tuple& operator= (const argument_tuple&);\n\n"

    if numArgs:
        s += "    BOOST_COPYABLE_AND_MOVABLE(argument_tuple)\n"

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
    s += "    typedef typename boost::add_const<\n"
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
    s += "    return t.get(boost::integral_constant<std::size_t, TIndex>());\n"
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
    s += "    return t.get(boost::integral_constant<std::size_t, TIndex>());\n"
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
    s += "        : boost::integral_constant<\n"
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
    s += "        : boost::integral_constant<\n"
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
    s += ",\n               ".join(["BOOST_FWD_REF(A%d)" % i for i in xrange(numArgs)])
    s += ">\n"
    s += "forward_as_argument_tuple("
    s += ",\n                          ".join(["BOOST_FWD_REF(A%d) a%d" % (i,i) for i in xrange(numArgs)])
    s += ")\n{\n"
    s += "    return argument_tuple<"
    s += ",\n                          ".join(["BOOST_FWD_REF(A%d)" % i for i in xrange(numArgs)])
    s += ">(\n            "
    s += ",\n            ".join(["boost::forward<A%d>(a%d)" % (i,i) for i in xrange(numArgs)])
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
    s += "struct is_placeholder : boost::integral_constant<int, 0>\n{\n};\n\n"
    s += "template <int TIndex>\n"
    s += "struct is_placeholder<placeholders::placeholder<TIndex> >\n"
    s += "        : boost::integral_constant<int, TIndex>\n{\n};\n\n"
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
    s += "    typedef typename boost::add_rvalue_reference<temp_type>::type type;\n"
    s += "};\n\n"

    s += "template <typename TBound>\n"
    s += "struct unpack_argument\n"
    s += "{\n"
    s += "    template <typename TType, typename TUnbound>\n"
    s += "    BOOST_FWD_REF(TType) operator() (BOOST_FWD_REF(TType) bound,\n"
    s += "                                     TUnbound& unbound) const\n"
    s += "    {\n"
    s += "        return boost::forward<TType>(bound);\n"
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
    s += "        return boost::forward<typename unpacked_argument_type<\n"
    s += "                                  bound_type, TUnbound>::type>(\n"
    s += "                    get<TIndex - 1>(unbound));\n"
    s += "    }\n"
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

    s += "    // Constructor with perfect forwarding\n"
    if numArgs:
        s += "    template <"
        s += ",\n              ".join(["typename T%d" % i for i in xrange(numArgs)])
        s += ">\n"
    s += "    explicit BindResult(const F& f"
    if numArgs:
        s += ",\n                        "
        s += ",\n                        ".join(["BOOST_FWD_REF(T%d) t%d" % (i,i) for i in xrange(numArgs)])
    s += ")\n"
    s += "        : m_functor(f)"
    if numArgs:
        s += ",\n          m_arguments("
        s += ",\n                      ".join(["boost::forward<T%d>(t%d)" % (i,i) for i in xrange(numArgs)])
        s += ")"
    s += "\n    {\n    }\n\n"

    s += "    // Copy construction\n"
    s += "    BindResult(const BindResult& other)\n"
    s += "        : m_functor(other.m_functor),\n"
    s += "          m_arguments(other.m_arguments)\n"
    s += "    {\n    }\n\n"

    s += "    // Move construction\n"
    s += "    BindResult(BOOST_RV_REF(BindResult) other)\n"
    s += "        : m_functor(boost::move(other.m_functor)),\n"
    s += "          m_arguments(boost::move(other.m_arguments))\n"
    s += "    {\n    }\n\n"

    def operator(nargs, qualifier):
        s = ""
        if nargs:
            s += "    template <"
            s += ",\n              ".join(["typename T%d" % i for i in xrange(nargs)])
            s += ">\n"
        s += "    TResult operator() ("
        s += ",\n                        ".join(["BOOST_FWD_REF(T%d) t%d" % (i,i) for i in xrange(nargs)])
        s += ")"
        if qualifier:
            s += " " + qualifier
        s += "\n"
        s += "    {\n"
        s += "        return this->invoke<TResult>(\n"
        s += "                forward_as_argument_tuple("
        if nargs:
            s += "\n                    "
            s += ",\n                    ".join(["boost::forward<T%d>(t%d)" % (i,i) for i in xrange(nargs)])
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
        s += "            BOOST_FWD_REF(TUnbound) unbound_args,\n"
        s += "            typename boost::enable_if_c<\n"
        if void:
            s += "                boost::is_same<TReturn, void>::value"
        else:
            s += "                !boost::is_same<TReturn, void>::value"
        if memberFunction:
            s += "\n                && boost::is_member_function_pointer<F>::value"
        elif numArgs:
            s += "\n                && !boost::is_member_function_pointer<F>::value"
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

    s += "    BOOST_COPYABLE_AND_MOVABLE(BindResult)\n"

    s += "};\n\n"
    return s

def bindHelper(numArgs, maxArgs):
    s = ""

    s += "template <typename TResult,\n"
    s += "          typename TFunctor"
    if numArgs == maxArgs:
        s += ",\n          "
        s += ",\n          ".join(["typename A%d = bind_helper_null_type" % i for i in xrange(numArgs)])
    elif numArgs:
        s += ",\n          "
        s += ",\n          ".join(["typename A%d" % i for i in xrange(numArgs)])
    s += ">\n"
    s += "struct bind_helper"
    if numArgs != maxArgs:
        s += "<TResult, TFunctor,\n                   "
        s += ",\n                   ".join(["A%d" % i for i in xrange(numArgs)] +
                       ["bind_helper_null_type" for i in xrange(maxArgs - numArgs)])
        s += ">"
    s += "\n{\n"
    s += "    typedef typename boost::decay<TFunctor>::type functor_type;\n"
    s += "    typedef BindResult<TResult,\n"
    s += "                       functor_type("
    s += ",\n                                    ".join(["typename boost::decay<A%d>::type" % i for i in xrange(numArgs)])
    s += ")> type;\n"
    s += "};\n\n"
    return s

def bind(numArgs):
    s = ""

    s += "template <typename TResult,\n"
    s += "          typename TFunctor"
    if numArgs:
        s += ",\n          "
        s += ",\n          ".join(["typename A%d" % i for i in xrange(numArgs)])
    s += ">\n"
    s += "inline\n"
    s += "typename detail::bind_helper<TResult,\n"
    s += "                             TFunctor"
    if numArgs:
        s += ",\n                             "
        s += ",\n                             ".join(["A%d" % i for i in xrange(numArgs)])
    s += ">::type\n"
    s += "bind(BOOST_FWD_REF(TFunctor) f"
    if numArgs:
        s += ",\n     "
        s += ",\n     ".join(["BOOST_FWD_REF(A%d) a%d" % (i,i) for i in xrange(numArgs)])
    s += ")\n{\n"
    s += "    typedef typename detail::bind_helper<TResult,\n"
    s += "                                         TFunctor"
    if numArgs:
        s += ",\n                                         "
        s += ",\n                                         ".join(["A%d" % i for i in xrange(numArgs)])
    s += ">::type bound_type;\n"
    s += "    return bound_type(boost::forward<TFunctor>(f)"
    if numArgs:
        s += ",\n                      "
        s += ",\n                      ".join(["boost::forward<A%d>(a%d)" % (i,i) for i in xrange(numArgs)])
    s += ");\n"
    s += "}\n\n"
    return s

def typeErasedBindInvoker(maxArgs):
    s = ""

    s += "enum AdapterTask\n"
    s += "{\n"
    s += "    AdapterTaskClone,\n"
    s += "    AdapterTaskDestroy\n"
    s += "};\n\n"

    s += "// An adapter which allows to erase the type of a bind expression.\n"
    s += "template <typename TBindResult>\n"
    s += "struct BindAdapter\n"
    s += "{\n"

    s += "    static void manage(AdapterTask task, void* self, const void* other)\n"
    s += "    {\n"
    s += "        switch (task)\n"
    s += "        {\n"
    s += "        case AdapterTaskClone:\n"
    s += "            new (self) TBindResult(*static_cast<const TBindResult*>(other));\n"
    s += "            break;\n"
    s += "        case AdapterTaskDestroy:\n"
    s += "            static_cast<TBindResult*>(self)->~TBindResult();\n"
    s += "            break;\n"
    s += "        }\n"
    s += "    }\n\n"

    def invoke(nargs):
        s = ""
        if nargs:
            s += "    template <"
            s += ",\n              ".join(["typename T%d" % i for i in xrange(nargs)])
            s += ">\n"
        s += "    static typename TBindResult::result_type invoke(\n"
        s += "            void* bindExpression"
        if nargs:
            s += ",\n            "
            s += ",\n            ".join(["T%d t%d" % (i,i) for i in xrange(nargs)])
        s += ")\n"
        s += "    {\n"
        s += "        return (*static_cast<TBindResult*>(bindExpression))("
        if nargs:
            s += "\n                    "
            s += ",\n                    ".join(["boost::forward<T%d>(t%d)" % (i,i) for i in xrange(nargs)])
        s += ");\n"
        s += "    }\n\n"
        return s

    for i in xrange(maxArgs + 1):
        s += invoke(i)

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

    s += "    static_function()\n"
    s += "        : m_invoker(0)\n"
    s += "    {\n"
    s += "    }\n\n"

    s += "    ~static_function()\n"
    s += "    {\n"
    s += "        release();\n"
    s += "    }\n\n"

    s += "    template <typename TSignature>\n"
    s += "    static_function& operator= (const detail::BindResult<TResult, TSignature>& expr)\n"
    s += "    {\n"
    s += "        BOOST_STATIC_ASSERT_MSG(sizeof(expr) <= TStorageSize,\n"
    s += "                                \"The bind expression is too large for this function.\");\n\n"
    s += "        typedef detail::BindAdapter<detail::BindResult<TResult, TSignature> > adapter;\n\n"
    s += "        release();\n"
    s += "        m_manager = &adapter::manage;\n"
    s += "        m_manager(detail::AdapterTaskClone, &m_storage, &expr);\n"
    s += "        m_invoker = &adapter::"
    if numArgs == 0:
        s += "invoke;\n"
    else:
        s += "template invoke<"
        s += ",\n                                              ".join(["A%d" % i for i in xrange(numArgs)])
        s += ">;\n"
    s += "        return *this;\n"
    s += "    }\n\n"

    s += "    TResult operator() ("
    s += ",\n                        ".join(["A%d a%d" % (i,i) for i in xrange(numArgs)])
    s += ")\n"
    s += "    {\n"
    s += "        return (*m_invoker)(&m_storage"
    if numArgs:
        s += ",\n                            "
        s += ",\n                            ".join(["boost::forward<A%d>(a%d)" % (i,i) for i in xrange(numArgs)])
    s += ");\n"
    s += "    }\n\n"

    s += "    static_function& operator= (weos::nullptr_t)\n"
    s += "    {\n"
    s += "        release();\n"
    s += "        return *this;\n"
    s += "    }\n\n"

    s += "    operator bool() const\n"
    s += "    {\n"
    s += "        return m_invoker != 0;\n"
    s += "    }\n\n"

    s += "private:\n"
    s += "    typedef void (*manager_type)(detail::AdapterTask, void*, const void*);\n"
    s += "    typedef TResult (*invoker_type)(void*"
    if numArgs:
        s += ",\n                                    "
        s += ",\n                                    ".join(["A%d" % i for i in xrange(numArgs)])
    s += ");\n\n"

    s += "    typename boost::aligned_storage<TStorageSize>::type m_storage;\n"
    s += "    manager_type m_manager;\n"
    s += "    invoker_type m_invoker;\n\n"

    s += "    void release()\n"
    s += "    {\n"
    s += "        if (m_invoker)\n"
    s += "        {\n"
    s += "            m_manager(detail::AdapterTaskDestroy, &m_storage, 0);\n"
    s += "            m_invoker = 0;\n"
    s += "        }\n"
    s += "    }\n"

    s += "};\n\n"
    return s



def generateHeader(maxArgs):
    s = ""

    s += license()

    s += "#ifndef WEOS_COMMON_FUNCTIONAL_HPP\n"
    s += "#define WEOS_COMMON_FUNCTIONAL_HPP\n\n"

    s += "#include \"../config.hpp\"\n\n"
    s += "#include \"../utility.hpp\"\n\n"

    s += "#include <boost/utility/enable_if.hpp>\n"
    s += "#include <boost/move/move.hpp>\n"
    s += "#include <boost/type_traits.hpp>\n\n"

    s += "namespace weos\n{\n\n"

    s += placeholders(maxArgs)

    s += "namespace detail\n{\n\n"

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

    s += "template <typename TResult, typename TSignature>\n"
    s += "struct BindResult;\n\n"
    for i in xrange(maxArgs + 1):
        s += bindResult(i, maxArgs)

    s += "\nstruct bind_helper_null_type;\n\n"
    for i in xrange(maxArgs + 1):
        s += bindHelper(maxArgs - i, maxArgs)
    s += "} // namespace detail\n\n"

    s += "// ====================================================================\n"
    s += "// bind<>\n"
    s += "// ====================================================================\n\n"

    for i in xrange(maxArgs + 1):
        s += bind(i)

    s += "// ====================================================================\n"
    s += "// static_function<>\n"
    s += "// ====================================================================\n\n"

    s += "namespace detail\n{\n\n"
    s += typeErasedBindInvoker(maxArgs)
    s += "} // namespace detail\n\n"

    s += "template <typename TSignature,\n"
    s += "          std::size_t TStorageSize = WEOS_DEFAULT_STATIC_FUNCTION_SIZE>\n"
    s += "class static_function;\n\n"

    for i in xrange(maxArgs + 1):
        s += staticFunction(i)

    s += "} // namespace weos\n\n"

    s += "#endif // WEOS_COMMON_FUNCTIONAL_HPP\n\n"

    return s



def generateSource(maxArgs):
    s = ""

    s += license()

    s += "#include \"functional.hpp\"\n\n"

    s += "namespace weos\n{\n\n"
    s += "namespace placeholders\n{\n\n"

    for i in xrange(1, maxArgs + 1):
        s += "const placeholder<%d> _%d;\n" % (i,i)

    s += "} // namespace placeholders\n\n"
    s += "} // namespace weos\n\n"

    return s



maxArgs = 4

print(generateHeader(maxArgs))

with open('functional.hpp', 'w') as fout:
    fout.write(generateHeader(maxArgs))

with open('functional.cpp', 'w') as fout:
    fout.write(generateSource(maxArgs))
