//#####################################################################
// Class Class
//#####################################################################
#include <other/core/python/Class.h>
#include <iostream>
namespace other{

int trivial_init(PyObject* self,PyObject* args,PyObject* kwds) {
  return 0; // all initialization is handled in __new__
}

PyObject* simple_alloc(PyTypeObject* type,Py_ssize_t nitems) {
  PyObject* object = (PyObject*)calloc(1,type->tp_basicsize);
  return object?PyObject_INIT(object,type):0;
}

void add_descriptor(PyTypeObject* type,const char* name,PyObject* descr) {
  if (PyDict_GetItemString(type->tp_dict,name))
    PyErr_Format(PyExc_TypeError,"Descriptor '%s.%s' already specified for class",type->tp_name,name);
  else if (!PyDict_SetItemString(type->tp_dict,name,descr))
    return;
  Py_DECREF(descr);
  throw PythonError();
}

ClassBase::ClassBase(const char* name,bool visible,PyTypeObject* type,int offset)
  :type(type) {

  // Verify that OTHER_DECLARE_TYPE has been used, and initialize name
  if (!type->tp_name || strncmp(type->tp_name,"other_default_name",18)) {
    PyErr_Format(PyExc_AssertionError,"%s->type->tp_name is %s: did you forget OTHER_DECLARE_TYPE?",name,type->tp_name?type->tp_name:"0");
    throw_python_error();
  }
  type->tp_name = name;

  // Make sure base starts at the beginning
  if (offset) {
    PyErr_Format(PyExc_AssertionError,"%s is offset %d bytes before its base",name,offset);
    throw_python_error();
  }

  // Verify that base class has been initialized
  if (type->tp_base && !strncmp(type->tp_base->tp_name,"other_default_name",18)) {
    PyErr_Format(PyExc_AssertionError,"%s's base class hasn't been initialized for python: make sure wrap_<base> is called before wrap_<self>",name);
    throw_python_error();
  }

  // Fill in some additional fields automatically
  if (PyType_Ready(type)<0)
    throw_python_error();

  // Prevent tp_new from inheriting, since the base tp_new constructs an instance of the base class
  type->tp_new = 0;

  // Add the type to current module
  if (visible) {
    Py_INCREF(type);
    python::add_object(type->tp_name,(PyObject*)type);
  }
}

}