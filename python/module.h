//#####################################################################
// Header Module
//#####################################################################
//
// Macro to declare a python module initialization function.  This does two things:
//
// 1. Catches and translates any C++ exceptions into python.
// 2. Sets the current_module variable for use in Class and other wrapper helpers.
//
// If the module is named blah, the shared library should be libBlah.so, not Blah.so.
//
//#####################################################################
#pragma once

#include <other/core/python/config.h>
#include <other/core/python/exceptions.h>
#include <other/core/python/wrap_function.h>
#include <other/core/utility/config.h>
namespace other {
namespace python {

#define OTHER_PYTHON_MODULE(name) \
  static void Init_Helper_##name(); \
  PyMODINIT_FUNC OTHER_EXPORT initlib##name(); \
  PyMODINIT_FUNC OTHER_EXPORT \
  initlib##name() { \
    PyObject* module = Py_InitModule3("lib"#name,0,0); \
    if (module) { \
      try { \
        ::other::python::Scope scope(module); \
        ::other::python::import_core(); \
        Init_Helper_##name(); \
      } catch(std::exception& error) { \
        ::other::set_python_exception(error); \
      } \
    } \
  } \
  static void Init_Helper_##name()

void import_core() OTHER_EXPORT;

// Steal reference to object and add it to the current module
void add_object(const char* name,PyObject* object) OTHER_EXPORT;

template<class T> static inline void add_object(const char* name,const T& object) {
  add_object(name,to_python(object));
}

template<class Function> static inline void function(const char* name,Function function) {
  add_object(name,wrap_function(name,function)); 
}

#define OTHER_OBJECT(name) ::other::python::add_object(#name,name);
#define OTHER_OBJECT_2(name,object) ::other::python::add_object(#name,object);

#define OTHER_FUNCTION(name) ::other::python::function(#name,name);
#define OTHER_FUNCTION_2(name,f) ::other::python::function(#name,f);

#define OTHER_OVERLOADED_FUNCTION_2(type,name,function_) ::other::python::function(name,(type)function_);
  
#define OTHER_OVERLOADED_FUNCTION(type,function_) OTHER_OVERLOADED_FUNCTION_2(type,#function_,function_);
  
struct Scope {
  OTHER_EXPORT Scope(PyObject* module); 
  OTHER_EXPORT ~Scope();
};

#define OTHER_WRAP(name) extern void wrap_##name();wrap_##name();

}
}