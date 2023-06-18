//@	{
//@	 "target":{"name":"wad64py.o", "pkgconfig_libs":["python3"]}
//@	,"dependencies":[{"ref":"python3", "origin":"pkg-config"}]
//@	}

#include "./wad64.hpp"
#include "./fd_owner.hpp"

#define PY_SSIZE_T_CLEAN  /* Make "s#" use Py_ssize_t rather than int. */
#include <Python.h>
#include <array>

namespace
{
	struct ArchiveFileRw
	{
		template<class ... FileArgs>
		explicit ArchiveFileRw(char const* path, FileArgs&&... file_args):
			file{path, std::forward<FileArgs>(file_args)...},
			archive{Wad64::FileReference{std::ref(file)}}
		{
			printf("created %p\n", this);
		}

		~ArchiveFileRw()
		{ printf("destroyed %p\n", this); }

		Wad64::FdOwner file;
		Wad64::Archive archive;
	};

	PyObject* open_file_for_insertion(PyObject*, PyObject* args)
	{
		try
		{
			char const* src_file{};
			if(!PyArg_ParseTuple(args, "s", &src_file))
			{ return nullptr; }

			return PyLong_FromVoidPtr(
				new ArchiveFileRw{src_file,
					Wad64::IoMode::AllowRead().allowWrite(),
					Wad64::FileCreationMode::AllowOverwriteWithoutTruncation().allowCreation()
				}
			);
		}
		catch(std::exception const& err)
		{
			PyErr_SetString(PyExc_RuntimeError, err.what());
			return nullptr;
		}

		return Py_None;
	}

	PyObject* close_archive_file_rw(PyObject*, PyObject* args)
	{
		PyObject* obj{};
		if(!PyArg_ParseTuple(args, "O", &obj))
		{ return nullptr; }

		auto const ptr = PyLong_AsVoidPtr(obj);
		assert(ptr != nullptr);

		delete reinterpret_cast<ArchiveFileRw*>(ptr);

		return Py_None;
	}

	constinit std::array<PyMethodDef, 3> method_table
	{
		PyMethodDef{"open_file_for_insertion", open_file_for_insertion, METH_VARARGS, ""},
		PyMethodDef{"close_archive_file_rw", close_archive_file_rw, METH_VARARGS, ""},
		PyMethodDef{nullptr, nullptr, 0, nullptr}
	};

	constinit PyModuleDef module_info
	{
		PyModuleDef_HEAD_INIT,
		"wad64py",
		nullptr,
		-1,
		std::data(method_table),
		nullptr,
		nullptr,
		nullptr,
		nullptr
	};
}

PyMODINIT_FUNC PyInit_wad64py()
{
	return PyModule_Create(&module_info);
}