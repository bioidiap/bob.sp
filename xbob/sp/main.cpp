/**
 * @author Andre Anjos <andre.anjos@idiap.ch>
 * @date Fri 25 Oct 16:54:55 2013
 *
 * @brief Bindings to bob::sp
 */

#ifdef NO_IMPORT_ARRAY
#undef NO_IMPORT_ARRAY
#endif
#include <xbob.blitz/capi.h>
#include <xbob.blitz/cleanup.h>

extern PyTypeObject PyBobSpFFT1D_Type;
extern PyTypeObject PyBobSpIFFT1D_Type;
extern PyTypeObject PyBobSpFFT2D_Type;
extern PyTypeObject PyBobSpIFFT2D_Type;
extern PyTypeObject PyBobSpDCT1D_Type;
extern PyTypeObject PyBobSpIDCT1D_Type;
extern PyTypeObject PyBobSpDCT2D_Type;
extern PyTypeObject PyBobSpIDCT2D_Type;
extern PyTypeObject PyBobSpExtrapolationBorder_Type;
extern PyTypeObject PyBobSpQuantization_Type;

PyDoc_STRVAR(s_extrapolate_str, "extrapolate");
PyDoc_STRVAR(s_extrapolate__doc__,
"extrapolate(src, dst, [[border=" XBOB_EXT_MODULE_PREFIX ".BorderSize.Zero], value=0.]) -> None\n\
\n\
Extrapolates values in the given array using the specified border\n\
type. Works for 1 or 2D input arrays. The parameter ``value`` is\n\
only used if the border type is set to\n\
:py:attr:`" XBOB_EXT_MODULE_PREFIX ".BorderSize.Constant`. It is,\n\
by default, set to ``0.``, or the equivalent on the datatype passed\n\
as input. For example, ``False``, if the input is boolean and\n\
0+0j, if it is complex.\n\
");
PyObject* extrapolate(PyObject*, PyObject* args, PyObject* kwds);

static PyMethodDef module_methods[] = {
    {
      s_extrapolate_str,
      (PyCFunction)extrapolate,
      METH_VARARGS|METH_KEYWORDS,
      s_extrapolate__doc__
    },
    {0}  /* Sentinel */
};

PyDoc_STRVAR(module_docstr, "Bob signal processing toolkit");

#if PY_VERSION_HEX >= 0x03000000
static PyModuleDef module_definition = {
  PyModuleDef_HEAD_INIT,
  XBOB_EXT_MODULE_NAME,
  module_docstr,
  -1,
  module_methods, 
  0, 0, 0, 0
};
#endif

static PyObject* create_module (void) {

  PyBobSpFFT1D_Type.tp_new = PyType_GenericNew;
  if (PyType_Ready(&PyBobSpFFT1D_Type) < 0) return 0;

  PyBobSpIFFT1D_Type.tp_new = PyType_GenericNew;
  if (PyType_Ready(&PyBobSpIFFT1D_Type) < 0) return 0;

  PyBobSpFFT2D_Type.tp_new = PyType_GenericNew;
  if (PyType_Ready(&PyBobSpFFT2D_Type) < 0) return 0;

  PyBobSpIFFT2D_Type.tp_new = PyType_GenericNew;
  if (PyType_Ready(&PyBobSpIFFT2D_Type) < 0) return 0;

  PyBobSpDCT1D_Type.tp_new = PyType_GenericNew;
  if (PyType_Ready(&PyBobSpDCT1D_Type) < 0) return 0;

  PyBobSpIDCT1D_Type.tp_new = PyType_GenericNew;
  if (PyType_Ready(&PyBobSpIDCT1D_Type) < 0) return 0;

  PyBobSpDCT2D_Type.tp_new = PyType_GenericNew;
  if (PyType_Ready(&PyBobSpDCT2D_Type) < 0) return 0;

  PyBobSpIDCT2D_Type.tp_new = PyType_GenericNew;
  if (PyType_Ready(&PyBobSpIDCT2D_Type) < 0) return 0;

  PyBobSpExtrapolationBorder_Type.tp_new = PyType_GenericNew;
  if (PyType_Ready(&PyBobSpExtrapolationBorder_Type) < 0) return 0;

  PyBobSpQuantization_Type.tp_new = PyType_GenericNew;
  if (PyType_Ready(&PyBobSpQuantization_Type) < 0) return 0;

# if PY_VERSION_HEX >= 0x03000000
  PyObject* m = PyModule_Create(&module_definition);
# else
  PyObject* m = Py_InitModule3(XBOB_EXT_MODULE_NAME, module_methods, module_docstr);
# endif
  if (!m) return 0;
  auto m_ = make_safe(m); ///< protects against early returns

  if (PyModule_AddStringConstant(m, "__version__", XBOB_EXT_MODULE_VERSION) < 0)
    return 0;

  /* register the types to python */
  Py_INCREF(&PyBobSpFFT1D_Type);
  if (PyModule_AddObject(m, "FFT1D", (PyObject *)&PyBobSpFFT1D_Type) < 0) return 0;

  Py_INCREF(&PyBobSpIFFT1D_Type);
  if (PyModule_AddObject(m, "IFFT1D", (PyObject *)&PyBobSpIFFT1D_Type) < 0) return 0;

  Py_INCREF(&PyBobSpFFT2D_Type);
  if (PyModule_AddObject(m, "FFT2D", (PyObject *)&PyBobSpFFT2D_Type) < 0) return 0;

  Py_INCREF(&PyBobSpIFFT2D_Type);
  if (PyModule_AddObject(m, "IFFT2D", (PyObject *)&PyBobSpIFFT2D_Type) < 0) return 0;

  Py_INCREF(&PyBobSpDCT1D_Type);
  if (PyModule_AddObject(m, "DCT1D", (PyObject *)&PyBobSpDCT1D_Type) < 0) return 0;

  Py_INCREF(&PyBobSpIDCT1D_Type);
  if (PyModule_AddObject(m, "IDCT1D", (PyObject *)&PyBobSpIDCT1D_Type) < 0) return 0;

  Py_INCREF(&PyBobSpDCT2D_Type);
  if (PyModule_AddObject(m, "DCT2D", (PyObject *)&PyBobSpDCT2D_Type) < 0) return 0;

  Py_INCREF(&PyBobSpIDCT2D_Type);
  if (PyModule_AddObject(m, "IDCT2D", (PyObject *)&PyBobSpIDCT2D_Type) < 0) return 0;

  Py_INCREF(&PyBobSpExtrapolationBorder_Type);
  if (PyModule_AddObject(m, "BorderType", (PyObject *)&PyBobSpExtrapolationBorder_Type) < 0) return 0;

  Py_INCREF(&PyBobSpQuantization_Type);
  if (PyModule_AddObject(m, "Quantization", (PyObject *)&PyBobSpQuantization_Type) < 0) return 0;

  /* imports xbob.blitz C-API + dependencies */
  if (import_xbob_blitz() < 0) return 0;

  Py_INCREF(m);
  return m;

}

PyMODINIT_FUNC XBOB_EXT_ENTRY_NAME (void) {
# if PY_VERSION_HEX >= 0x03000000
  return
# endif
    create_module();
}
