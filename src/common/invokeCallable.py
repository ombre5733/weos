# -*- coding: utf-8 -*-
"""
Simulates variadic templates for invokeCallable()
"""


def memFunPtr(fout, numParams, isValue, isConst):
    fout.write("template <typename Return, typename F,\n          ")
    fout.write(",\n          ".join(["typename A%d" % i 
                                     for i in xrange(numParams)]))
    fout.write(">\n")
    fout.write("inline\n")
    fout.write("typename boost::enable_if_c<\n")
    if isValue:
        fout.write("    boost::is_base_of<F, typename boost::remove_reference<A0>::type>::value>::type\n")
    else:
        fout.write("    !boost::is_base_of<F, typename boost::remove_reference<A0>::type>::value>::type\n")
    fout.write("invokeCallable(Return (F::*f)(")
    fout.write(", ".join(["A%d" % i for i in xrange(1, numParams)]))
    fout.write(")")
    if isConst:
        fout.write(" const")
    fout.write(",\n               ")
    fout.write(",\n               ".join(["BOOST_FWD_REF(A%d) a%d" % (i, i) 
                                          for i in xrange(numParams)]))
    fout.write(")\n{\n")
    if isValue:
        fout.write("    (boost::forward<A0>(a0).*f)")
    else:
        fout.write("    ((*boost::forward<A0>(a0)).*f)")
    fout.write("(\n        ")
    fout.write(",\n        ".join(["boost::forward<A%d>(a%d)" % (i, i)
                                   for i in xrange(1, numParams)]))
    fout.write(");\n}\n\n")

def funPtr(fout, numParams):
    fout.write("template <typename F")
    if numParams:
        fout.write(",\n          ")
    fout.write(",\n          ".join(["typename A%d" % i 
                                     for i in xrange(numParams)]))
    fout.write(">\n")
    fout.write("inline\n")
    fout.write("typename boost::enable_if_c<\n")
    fout.write("    !boost::is_member_function_pointer<typename boost::remove_reference<F>::type>::value>::type\n")
    fout.write("invokeCallable(BOOST_FWD_REF(F) f")
    if numParams:
        fout.write(",\n               ")
    fout.write(",\n               ".join(["BOOST_FWD_REF(A%d) a%d" % (i, i) 
                                          for i in xrange(numParams)]))
    fout.write(")\n{\n")
    fout.write("    boost::forward<F>(f)")
    fout.write("(\n        ")
    fout.write(",\n        ".join(["boost::forward<A%d>(a%d)" % (i, i)
                                   for i in xrange(numParams)]))
    fout.write(");\n}\n\n")



fout = open('thread_invoker.template.hpp', 'w')

fout.write("// --------------------------------------------------------------------\n");
fout.write("//     Case 1: Member function pointer together with object\n")
fout.write("// --------------------------------------------------------------------\n");
for numParams in xrange(1, 5):
    memFunPtr(fout, numParams, isValue=True, isConst=False)
    memFunPtr(fout, numParams, isValue=True, isConst=True)

fout.write("// --------------------------------------------------------------------\n");
fout.write("//     Case 2: Member function pointer together with pointer\n")
fout.write("// --------------------------------------------------------------------\n");
for numParams in xrange(1, 5):
    memFunPtr(fout, numParams, isValue=False, isConst=False)
    memFunPtr(fout, numParams, isValue=False, isConst=True)

fout.write("// --------------------------------------------------------------------\n");
fout.write("//     Case 3: Function pointer\n")
fout.write("// --------------------------------------------------------------------\n");
for numParams in xrange(0, 5):
    funPtr(fout, numParams)


fout.close()
