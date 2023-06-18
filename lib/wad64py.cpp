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
	struct Archive
	{
		template<class ... FileArgs>
		explicit Archive(char const* path, FileArgs&&... file_args):
			file{path, std::forward<FileArgs>(file_args)...},
			archive{Wad64::FileReference{std::ref(file)}}
		{}

		~Archive()
		{ }

		Wad64::FdOwner file;
		Wad64::Archive archive;
	};

	PyObject* open_archive_from_path(PyObject*, PyObject* args)
	{
		try
		{
			char const* src_file{};
			char const* io_mode_str{};
			char const* file_creation_mode_str{};
			if(!PyArg_ParseTuple(args, "sss", &src_file, &io_mode_str, &file_creation_mode_str))
			{ return nullptr; }

			auto const io_mode = fromString(std::type_identity<Wad64::IoMode>{}, io_mode_str);

			auto const file_creation_mode = fromString(std::type_identity<Wad64::FileCreationMode>{},
				file_creation_mode_str
			);

			return PyLong_FromVoidPtr(new Archive{src_file, io_mode, file_creation_mode});
		}
		catch(std::exception const& err)
		{
			PyErr_SetString(PyExc_RuntimeError, err.what());
			return nullptr;
		}

		return Py_None;
	}

	PyObject* close_archive(PyObject*, PyObject* args)
	{
		PyObject* obj{};
		if(!PyArg_ParseTuple(args, "O", &obj))
		{ return nullptr; }

		auto const ptr = PyLong_AsVoidPtr(obj);
		assert(ptr != nullptr);

		delete reinterpret_cast<Archive*>(ptr);

		return Py_None;
	}

	constinit std::array<PyMethodDef, 3> method_table
	{
		PyMethodDef{"open_archive_from_path", open_archive_from_path, METH_VARARGS, ""},
		PyMethodDef{"close_archive", close_archive, METH_VARARGS, ""},
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