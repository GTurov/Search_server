TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    document.cpp \
    read_input_functions.cpp \
    request_queue.cpp \
    search_server.cpp \
    string_processing.cpp \
    test_example_functions.cpp

HEADERS += \
    document.h \
    paginator.h \
    read_input_functions.h \
    request_queue.h \
    search_server.h \
    string_processing.h \
    test_example_functions.h
