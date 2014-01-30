/**
 * @author Andre Anjos <andre.anjos@idiap.ch>
 * @date Fri 25 Oct 16:54:55 2013
 *
 * @brief Bindings to bob::io
 */

#ifdef NO_IMPORT_ARRAY
#undef NO_IMPORT_ARRAY
#endif
#include <xbob.blitz/capi.h>
#include <xbob.blitz/cleanup.h>

extern PyTypeObject PyBobSpFFT1D_Type;
extern PyTypeObject PyBobSpIFFT1D_Type;

static PyMethodDef module_methods[] = {
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
