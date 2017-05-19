#include "py_column.h"
#include "py_types.h"
#include "py_utils.h"

static PyObject* py_rowmappingtypes[4];


Column_PyObject* pyColumn_from_Column(Column *col)
{
    Column_PyObject *pycol = Column_PyNew();
    if (pycol == NULL) return NULL;
    pycol->ref = col;
    return pycol;
}


static PyObject* get_mtype(Column_PyObject *self) {
    return incref(py_rowmappingtypes[self->ref->mtype]);
}


static PyObject* get_stype(Column_PyObject *self) {
    return incref(py_stype_names[self->ref->stype]);
}


static PyObject* get_ltype(Column_PyObject *self) {
    return incref(py_ltype_names[stype_info[self->ref->stype].ltype]);
}


static PyObject* get_isview(Column_PyObject *self) {
    return incref(self->ref->mtype == MT_VIEW? Py_True : Py_False);
}


static PyObject* get_srcindex(Column_PyObject *self) {
    ViewColumn *col = (ViewColumn*) self->ref;
    return (col->mtype == MT_VIEW)
            ? PyLong_FromLong(col->srcindex)
            : none();
}


static PyObject* get_data_size(Column_PyObject *self) {
    Column *col = self->ref;
    return PyLong_FromSize_t(col->mtype == MT_VIEW? 0 : col->alloc_size);
}


static PyObject* get_meta(Column_PyObject *self) {
    Column *col = self->ref;
    void *meta = col->meta;
    switch (col->stype) {
        case ST_STRING_I4_VCHAR:
        case ST_STRING_I8_VCHAR:
            return PyUnicode_FromFormat("offoff=%lld",
                                        ((VarcharMeta*)meta)->offoff);
        case ST_STRING_FCHAR:
            return PyUnicode_FromFormat("n=%d", ((FixcharMeta*)meta)->n);
        case ST_REAL_I2:
        case ST_REAL_I4:
        case ST_REAL_I8:
            return PyUnicode_FromFormat("scale=%d",
                                        ((DecimalMeta*)meta)->scale);
        case ST_STRING_U1_ENUM:
        case ST_STRING_U2_ENUM:
        case ST_STRING_U4_ENUM: {
            EnumMeta *m = (EnumMeta*) meta;
            return PyUnicode_FromFormat("offoff=%lld&dataoff=%lld&nlevels=%d",
                                        m->offoff, m->dataoff, m->nlevels);
        }
        default:
            if (meta == NULL)
                return none();
            else
                return PyUnicode_FromFormat("%p", meta);
    }
}


int init_py_column(PyObject *module) {
    // Register Column_PyType on the module
    Column_PyType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&Column_PyType) < 0) return 0;
    Py_INCREF(&Column_PyType);
    PyModule_AddObject(module, "Column", (PyObject*) &Column_PyType);

    py_rowmappingtypes[0] = NULL;
    py_rowmappingtypes[MT_DATA] = PyUnicode_FromString("data");
    py_rowmappingtypes[MT_MMAP] = PyUnicode_FromString("mmap");
    py_rowmappingtypes[MT_VIEW] = PyUnicode_FromString("view");
    return 1;
}



//==============================================================================
// Column type definition
//==============================================================================

PyDoc_STRVAR(dtdoc_ltype, "'Logical' type of the column");
PyDoc_STRVAR(dtdoc_stype, "'Storage' type of the column");
PyDoc_STRVAR(dtdoc_mtype, "'Memory' type of the column: data, memmap or view");
PyDoc_STRVAR(dtdoc_isview, "Is this a view column?");
PyDoc_STRVAR(dtdoc_srcindex, "View column's index in the source datatable");
PyDoc_STRVAR(dtdoc_data_size, "The amount of memory taken by column's data");
PyDoc_STRVAR(dtdoc_meta, "String representation of the column's `meta` struct");

#define GETSET1(name) {#name, (getter)get_##name, NULL, dtdoc_##name, NULL}
static PyGetSetDef column_getseters[] = {
    GETSET1(mtype),
    GETSET1(stype),
    GETSET1(ltype),
    GETSET1(isview),
    GETSET1(srcindex),
    GETSET1(data_size),
    GETSET1(meta),
    {NULL, NULL, NULL, NULL, NULL}  /* sentinel */
};
#undef GETSET1

PyTypeObject Column_PyType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "_datatable.Column",                /* tp_name */
    sizeof(Column_PyObject),            /* tp_basicsize */
    0,                                  /* tp_itemsize */
    0,                                  /* tp_dealloc */
    0,                                  /* tp_print */
    0,                                  /* tp_getattr */
    0,                                  /* tp_setattr */
    0,                                  /* tp_compare */
    0,                                  /* tp_repr */
    0,                                  /* tp_as_number */
    0,                                  /* tp_as_sequence */
    0,                                  /* tp_as_mapping */
    0,                                  /* tp_hash  */
    0,                                  /* tp_call */
    0,                                  /* tp_str */
    0,                                  /* tp_getattro */
    0,                                  /* tp_setattro */
    0,                                  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,                 /* tp_flags */
    "Column object",                    /* tp_doc */
    0,                                  /* tp_traverse */
    0,                                  /* tp_clear */
    0,                                  /* tp_richcompare */
    0,                                  /* tp_weaklistoffset */
    0,                                  /* tp_iter */
    0,                                  /* tp_iternext */
    0,                                  /* tp_methods */
    0,                                  /* tp_members */
    column_getseters,                   /* tp_getset */
    0,                                  /* tp_base */
    0,                                  /* tp_dict */
    0,                                  /* tp_descr_get */
    0,                                  /* tp_descr_set */
    0,                                  /* tp_dictoffset */
    0,                                  /* tp_init */
    0,                                  /* tp_alloc */
    0,                                  /* tp_new */
    0,                                  /* tp_free */
    0,                                  /* tp_is_gc */
    0,                                  /* tp_bases */
    0,                                  /* tp_mro */
    0,                                  /* tp_cache */
    0,                                  /* tp_subclasses */
    0,                                  /* tp_weaklist */
    0,                                  /* tp_del */
    0,                                  /* tp_version_tag */
    0,                                  /* tp_finalize */
};
