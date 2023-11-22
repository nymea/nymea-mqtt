QMAKE_CXXFLAGS *= -Werror -g

greaterThan(QT_MAJOR_VERSION, 5) {
    message("Building with Qt6 support")
    CONFIG *= c++17
    QMAKE_LFLAGS *= -std=c++17
    QMAKE_CXXFLAGS *= -std=c++17
} else {
    message("Building with Qt5 support")
    CONFIG *= c++11
    QMAKE_LFLAGS *= -std=c++11
    QMAKE_CXXFLAGS *= -std=c++11
    DEFINES += QT_DISABLE_DEPRECATED_UP_TO=0x050F00
}

top_srcdir=$$PWD
top_builddir=$$shadowed($$PWD)

