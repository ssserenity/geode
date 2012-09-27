//#####################################################################
// Module Python
//#####################################################################
#include <other/core/utility/config.h>
#define PY_ARRAY_UNIQUE_SYMBOL _try_python_array_api
extern "C" {
OTHER_EXPORT extern void** PY_ARRAY_UNIQUE_SYMBOL;
}
#define OTHER_IMPORT_NUMPY

#include <other/core/python/module.h>
#include <other/core/python/enum.h>
#include <other/core/python/numpy.h>
#include <other/core/python/stl.h>
using namespace other;

namespace other {
namespace python {

static std::vector<PyObject*> modules;

static PyObject* module() {
  if (modules.empty())
    throw RuntimeError("No current module");
  return modules.back();
}

Scope::Scope(PyObject* module) {
  modules.push_back(module);
}

Scope::~Scope() {
  modules.pop_back();
}

void import_core() {
  char* name = PyModule_GetName(module());
  if (!name) throw_python_error();
  if (strcmp(name,"libother_core")){
    PyObject* python_str=PyString_FromString("other.core");
    if (!python_str) throw_python_error();
    PyObject* python = PyImport_Import(python_str);
    Py_DECREF(python_str);
    if (!python) throw_python_error();}
}

void add_object(const char* name,PyObject* object) {
  if (!object) throw PythonError();
  PyModule_AddObject(module(),name,object);
}

template<class TC> static TC convert_test(const TC& c) {
  return c;
}

enum EnumTest { EnumTestA, EnumTestB };

}

OTHER_DEFINE_ENUM(python::EnumTest)

}
using namespace other;
using namespace other::python;

void wrap_python() {
  if(strncmp(PY_VERSION,Py_GetVersion(),3)) {
    PyErr_Format(PyExc_ImportError,"python version mismatch: compiled again %s, linked against %s",PY_VERSION,Py_GetVersion());
    throw_python_error();
  }

  OTHER_WRAP(object)
  OTHER_WRAP(python_function)
  OTHER_WRAP(exceptions)
  OTHER_WRAP(test_class)

  function("list_convert_test",convert_test<vector<int> >);
  function("set_convert_test",convert_test<unordered_set<int> >);
  function("dict_convert_test",convert_test<unordered_map<int,string> >);
  function("enum_convert_test",convert_test<EnumTest>);

  OTHER_ENUM(EnumTest)
  OTHER_ENUM_VALUE(EnumTestA)
  OTHER_ENUM_VALUE(EnumTestB)

  // import numpy
  if (_import_array()<0){
    PyErr_Print();
    PyErr_SetString(PyExc_ImportError,"numpy.core.multiarray failed to import");
    throw_python_error();
  }

  python::add_object("real",(PyObject*)PyArray_DescrFromType(NumpyScalar<real>::value));
}