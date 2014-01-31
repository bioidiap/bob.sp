/**
 * @author Andre Anjos <andre.anjos@idiap.ch>
 * @date Fri 31 Jan 14:26:40 2014
 *
 * @brief Support for quantization
 */

#include <xbob.blitz/cppapi.h>
#include <xbob.blitz/cleanup.h>
#include <bob/sp/Quantization.h>

PyDoc_STRVAR(s_quantization_str, XBOB_EXT_MODULE_PREFIX ".Quantization");

PyDoc_STRVAR(s_quantization_doc,
"Quantization(dtype, [rounding=False, [num_levels=-1, [min_level=None, [max_level=None]]]])\n\
Quantization(quantization_table)\n\
Quantization(other)\n\
\n\
Functor to quantize 1D or 2D signals into different number of\n\
levels. At the moment, only ``uint8`` and ``uint16`` data\n\
types are supported. The output array returned by this functor\n\
will always have a ``uint32`` data type.\n\
\n\
Parameters:\n\
\n\
dtype\n\
  (numpy.dtype) The data type of arrays that are going to be **input**\n\
  by this functor. Currently supported values are ``uint8`` and\n\
  ``uint16``.\n\
\n\
rounding\n\
  (bool) If set to ``True`` (defaults to ``False``), performs\n\
  Matlab-like uniform quantization with rounding (see\n\
  http://www.mathworks.com/matlabcentral/newsreader/view_thread/275291).\n\
\n\
num_levels\n\
  (int) the number of quantization levels. The default is the total\n\
  number of discrete values permitted by the data type. For example,\n\
  ``uint8`` allows for 256 levels.\n\
\n\
min_level\n\
  (scalar) Input values smaller than or equal to this value are\n\
  scaled to this value prior to quantization. As a result, they\n\
  will be scaled in the lowest quantization level. The data type\n\
  of this scalar should be coercible to the datatype of the input.\n\
\n\
max_level\n\
  (scalar) Input values higher than this value are scaled to this\n\
  value prior to quantization. As a result, they will be scaled in\n\
  the highest qunatization level. The data type of this scalar\n\
  should be coercible to the datatype of the input.\n\
\n\
quantization_table\n\
  (array) A 1-dimensional array matching the data type of ``input``\n\
  containing user-specified thresholds for the quantization. If\n\
  Each element corresponds to the lower boundary of the particular\n\
  quantization level. Eg. ``array([ 0,  5, 10])`` means quantization\n\
  in 3 levels. Input values in the range :math:`[0,4]` will be quantized\n\
  to level 0, input values in the range :math:`[5,9]` will be\n\
  quantized to level 1 and input values in the range\n\
  :math:`[10-\\text{max}]` will be quantized to level 2.\n\
\n\
other\n\
  (Quantization) You can also initialize a Quantization object\n\
  by passing another Quantization object as constructor parameter.\n\
  This will create a deep-copy of this Quantization object.\n\
\n\
Once this object has been created, it can be used through its ``()``\n\
operator, by passing ``input`` and ``output`` parameters:\n\
\n\
input\n\
  (array) a 1 or 2-dimensional ``uint8`` or ``uint16`` array of any\n\
  size.\n\
\n\
output\n\
  (array) The array where to store the output. This array should\n\
  have the same dimensions of the input array, but have data type\n\
  ``uint32``. If this array is not provided, a new one is allocated\n\
  internally and returned.\n\
\n\
"
);

/**
 * Represents either a bob::sp::Quantization<T> object
 */
typedef struct {
  PyObject_HEAD
  int type_num;
  std::shared_ptr<void> cxx;
} PyBobSpQuantizationObject;

extern PyTypeObject PyBobSpQuantization_Type; //forward declaration

int PyBobSpQuantization_Check(PyObject* o) {
  return PyObject_IsInstance(o, reinterpret_cast<PyObject*>(&PyBobSpQuantization_Type));
}

static void PyBobSpQuantization_Delete (PyBobSpQuantizationObject* self) {

  self->cxx.reset();
  Py_TYPE(self)->tp_free((PyObject*)self);

}

static int PyBobSpQuantization_InitCopy
(PyBobSpQuantizationObject* self, PyObject* args, PyObject* kwds) {

  /* Parses input arguments in a single shot */
  static const char* const_kwlist[] = {"other", 0};
  static char** kwlist = const_cast<char**>(const_kwlist);

  PyObject* other = 0;

  if (!PyArg_ParseTupleAndKeywords(args, kwds, "O!", kwlist,
        &PyBobSpQuantization_Type, &other)) return -1;

  auto copy = reinterpret_cast<PyBobSpQuantizationObject*>(other);

  try {
    self->type_num = copy->type_num;
    switch (self->type_num) {
      case NPY_UINT8:
        self->cxx.reset(new bob::sp::Quantization<uint8_t>(*reinterpret_cast<bob::sp::Quantization<uint8_t>*>(copy->cxx.get())));
      case NPY_UINT16:
        self->cxx.reset(new bob::sp::Quantization<uint16_t>(*reinterpret_cast<bob::sp::Quantization<uint16_t>*>(copy->cxx.get())));
      default:
        PyErr_Format(PyExc_TypeError, "`%s' only accepts `uint8' or `uint16' as data types (not `%s')", Py_TYPE(self)->tp_name, PyBlitzArray_TypenumAsString(copy->type_num));
        return -1;
    }

  }
  catch (std::exception& ex) {
    PyErr_SetString(PyExc_RuntimeError, ex.what());
    return -1;
  }
  catch (...) {
    PyErr_Format(PyExc_RuntimeError, "cannot create new object of type `%s' - unknown exception thrown", Py_TYPE(self)->tp_name);
    return -1;
  }

  return 0; ///< SUCCESS

}

template <typename T> int initialize(PyBobSpQuantizationObject* self,
    bob::sp::quantization::QuantizationType type, Py_ssize_t levels,
    PyObject* min, PyObject* max) {

  // calculates all missing elements:
  T c_min = std::numeric_limits<T>::min();
  if (min) {
    c_min = PyBlitzArrayCxx_AsCScalar<T>(min);
    if (PyErr_Occurred()) return -1;
  }

  T c_max = std::numeric_limits<T>::max();
  if (max) {
    c_max = PyBlitzArrayCxx_AsCScalar<T>(max);
    if (PyErr_Occurred()) return -1;
  }

  if (levels <= 0) levels = c_max - c_min + 1;

  try {
    self->cxx.reset(new bob::sp::Quantization<T>(type, levels, c_min, c_max));
  }
  catch (std::exception& ex) {
    PyErr_SetString(PyExc_RuntimeError, ex.what());
    return -1;
  }
  catch (...) {
    PyErr_Format(PyExc_RuntimeError, "cannot create new object of type `%s' - unknown exception thrown", Py_TYPE(self)->tp_name);
    return -1;
  }

  return 0; ///< SUCCESS

}

static int PyBobSpQuantization_InitDiscrete(PyBobSpQuantizationObject* self,
    PyObject *args, PyObject* kwds) {

  /* Parses input arguments in a single shot */
  static const char* const_kwlist[] = {
    "dtype", 
    "rounding",
    "num_levels",
    "min_level",
    "max_level",
    0};
  static char** kwlist = const_cast<char**>(const_kwlist);

  int type_num = NPY_NOTYPE;
  int* type_num_ptr = &type_num;
  PyObject* rounding = 0;
  Py_ssize_t levels = -1;
  PyObject* min = 0;
  PyObject* max = 0;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "O&|O!nOO", kwlist,
        &PyBlitzArray_TypenumConverter, &type_num_ptr,
        &PyBool_Type, &rounding,
        &levels,
        &min,
        &max
        )) return -1;

  if (type_num != NPY_UINT8 && type_num != NPY_UINT16) {
  }

  bob::sp::quantization::QuantizationType rounding_enum =
    bob::sp::quantization::UNIFORM;
  if (rounding) {
    rounding_enum = PyObject_IsTrue(rounding)?bob::sp::quantization::UNIFORM_ROUNDING:bob::sp::quantization::UNIFORM;
  }

  self->type_num = type_num;
  switch (type_num) {
    case NPY_UINT8:
      return initialize<uint8_t>(self, rounding_enum, levels, min, max);
    case NPY_UINT16:
      return initialize<uint16_t>(self, rounding_enum, levels, min, max);
    default:
      PyErr_Format(PyExc_TypeError, "`%s' only accepts `uint8' or `uint16' as data types (not `%s')", Py_TYPE(self)->tp_name, PyBlitzArray_TypenumAsString(type_num));
  }

  return -1; ///< FAIULRE 
}

static int PyBobSpQuantization_InitTable(PyBobSpQuantizationObject* self,
    PyObject *args, PyObject* kwds) {

  /* Parses input arguments in a single shot */
  static const char* const_kwlist[] = {"quantization_table", 0};
  static char** kwlist = const_cast<char**>(const_kwlist);

  PyBlitzArrayObject* table = 0;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "O&", kwlist, 
        &PyBlitzArray_Converter, &table)) return -1;

  auto table_ = make_safe(table);

  if (table->ndim != 1) {
    PyErr_Format(PyExc_TypeError, "`%s' only accepts 1-dimensional arrays as quantization table (not %" PY_FORMAT_SIZE_T "dD arrays)", Py_TYPE(self)->tp_name, table->ndim);
    return -1;
  }

  if (table->type_num != NPY_UINT8 && table->type_num != NPY_UINT16) {
    PyErr_Format(PyExc_TypeError, "`%s' only accepts 1-dimensional `uint8' or `uint16' arrays as quantization tables (not `%s' arrays)", Py_TYPE(self)->tp_name,
        PyBlitzArray_TypenumAsString(table->type_num));
    return -1;
  }

  try {
    self->type_num = table->type_num;
    if (table->type_num == NPY_UINT8) {
      self->cxx.reset(new bob::sp::Quantization<uint8_t>(*PyBlitzArrayCxx_AsBlitz<uint8_t,1>(table)));
    }
    else {
      self->cxx.reset(new bob::sp::Quantization<uint16_t>(*PyBlitzArrayCxx_AsBlitz<uint16_t,1>(table)));
    }
    if (!self->cxx) {
      PyErr_Format(PyExc_MemoryError, "cannot create new object of type `%s' - no more memory", Py_TYPE(self)->tp_name);
      return -1;
    }
  }
  catch (std::exception& ex) {
    PyErr_SetString(PyExc_RuntimeError, ex.what());
    return -1;
  }
  catch (...) {
    PyErr_Format(PyExc_RuntimeError, "cannot create new object of type `%s' - unknown exception thrown", Py_TYPE(self)->tp_name);
    return -1;
  }

  return 0; ///< SUCCESS

}

static int PyBobSpQuantization_Init(PyBobSpQuantizationObject* self,
    PyObject* args, PyObject* kwds) {

  Py_ssize_t nargs = (args?PyTuple_Size(args):0) + (kwds?PyDict_Size(kwds):0);

  switch (nargs) {

    case 1:

      {

        PyObject* arg = 0; ///< borrowed (don't delete)
        if (PyTuple_Size(args)) arg = PyTuple_GET_ITEM(args, 0);
        else {
          PyObject* tmp = PyDict_Values(kwds);
          auto tmp_ = make_safe(tmp);
          arg = PyList_GET_ITEM(tmp, 0);
        }

        int type_num = NPY_NOTYPE;
        int* type_num_ptr = &type_num;
        if (PyBlitzArray_TypenumConverter(arg, &type_num_ptr)) {
          return PyBobSpQuantization_InitDiscrete(self, args, kwds);
        }
        else if (PyBobSpQuantization_Check(arg)) {
          return PyBobSpQuantization_InitCopy(self, args, kwds);
        }
        else {
          return PyBobSpQuantization_InitTable(self, args, kwds);
        }

        PyErr_Format(PyExc_TypeError, "cannot initialize `%s' with `%s' (see help)", Py_TYPE(self)->tp_name, Py_TYPE(arg)->tp_name);

      }

      break;

    case 2:
    case 3:
    case 4:
    case 5:
      return PyBobSpQuantization_InitDiscrete(self, args, kwds);

    default:

      PyErr_Format(PyExc_RuntimeError, "number of arguments mismatch - %s requires 1, 2, 3, 4 or 5 arguments for initialization, but you provided %" PY_FORMAT_SIZE_T "d (see help)", Py_TYPE(self)->tp_name, nargs);

  }

  return -1;

}

PyTypeObject PyBobSpQuantization_Type = {
    PyVarObject_HEAD_INIT(0, 0)
    s_quantization_str,                       /* tp_name */
    sizeof(PyBobSpQuantizationObject),        /* tp_basicsize */
    0,                                        /* tp_itemsize */
    (destructor)PyBobSpQuantization_Delete,   /* tp_dealloc */
    0,                                        /* tp_print */
    0,                                        /* tp_getattr */
    0,                                        /* tp_setattr */
    0,                                        /* tp_compare */
    0,                                        /* tp_repr */
    0,                                        /* tp_as_number */
    0,                                        /* tp_as_sequence */
    0,                                        /* tp_as_mapping */
    0,                                        /* tp_hash */
    0,                                        /* tp_call */
    0,                                        /* tp_str */
    0,                                        /* tp_getattro */
    0,                                        /* tp_setattro */
    0,                                        /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
    s_quantization_doc,                       /* tp_doc */
    0,                                        /* tp_traverse */
    0,                                        /* tp_clear */
    0,                                        /* tp_richcompare */
    0,                                        /* tp_weaklistoffset */
    0,                                        /* tp_iter */
    0,                                        /* tp_iternext */
    0,                                        /* tp_methods */
    0,                                        /* tp_members */
    0, //PyBobSpQuantization_getseters,            /* tp_getset */
    0,                                        /* tp_base */
    0,                                        /* tp_dict */
    0,                                        /* tp_descr_get */
    0,                                        /* tp_descr_set */
    0,                                        /* tp_dictoffset */
    (initproc)PyBobSpQuantization_Init,       /* tp_init */
    0,                                        /* tp_alloc */
    0,                                        /* tp_new */
};

/**
    .add_property("thresholds", &call_get_thresholds<uint8_t>, "1D numpy.ndarray of dtype='int' containing the thresholds of the quantization. Eg. array([ 0,  5, 10]) means quantization in 3 levels. Input values in the range [0,4] will be quantized to level 0, input values in the range[5,9] will be quantized to level 1 and input values in the range [10-max_level] will be quantized to level 2.")
    .add_property("max_level", &bob::sp::Quantization<uint8_t>::getMaxLevel, "Input values greater then this value are scaled to this value prior to quantization. As a result, they will be quantized in the highest quantization level. The default is the maximum value permitted by the data type of the class.")
    .add_property("min_level", &bob::sp::Quantization<uint8_t>::getMinLevel, "Input values smaller than or equal to this value are scaled to this value prior to quantization. As a result, they will be scaled in the lowest qunatization level. The default is the minimum value permitted by the data type. ")
    .add_property("num_levels", &bob::sp::Quantization<uint8_t>::getNumLevels, "Number of quantization levels. The default is the total number of discreet values permitted by the data type of the class.")
    .add_property("type", &bob::sp::Quantization<uint8_t>::getType, "Type of quantization: 1 - uniform quantization, 2 - uniform quantization with rounding, 3 - quantization by user-specified quantization table. Default is 1.")
    
    .def("__call__", &call_quantization_c<uint8_t>, (arg("self"),arg("input"), arg("output")), "Calls an object of this type to perform quantization for the input signal.")
    
    .def("quantization_level", &bob::sp::Quantization<uint8_t>::quantization_level, (arg("self"),arg("input")), "Calculates the quantization level for a single input value, given the current thresholds table.")
    ;
**/
