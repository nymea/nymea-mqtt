QMAKE_CXXFLAGS *= -Werror -std=c++11 -g
QMAKE_LFLAGS *= -std=c++11

gcc {
    COMPILER_VERSION = $$system($$QMAKE_CXX " -dumpversion")
    COMPILER_MAJOR_VERSION = $$str_member($$COMPILER_VERSION)
    greaterThan(COMPILER_MAJOR_VERSION, 7): QMAKE_CXXFLAGS += -Wno-deprecated-copy
}

top_srcdir=$$PWD
top_builddir=$$shadowed($$PWD)

