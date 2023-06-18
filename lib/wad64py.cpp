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
		{ }

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

	Archive& get_archive_ref(PyObject* args)
	{
		PyObject* obj{};
		assert(PyArg_ParseTuple(args, "O", &obj));

		auto const ptr = PyLong_AsVoidPtr(obj);
		assert(ptr != nullptr);

		return *reinterpret_cast<Archive*>(ptr);
	}

	PyObject* close_archive(PyObject*, PyObject* args)
	{
		delete &get_archive_ref(args);
		return Py_None;
	}

	PyObject* list_archive(PyObject*, PyObject* args)
	{
		auto& archive = get_archive_ref(args);
		auto& content = archive.archive.ls();

		auto ret = PyDict_New();

		for(auto& item : content)
		{
			auto tup = PyTuple_New(2);
			PyTuple_SET_ITEM(tup, 0, PyLong_FromSsize_t(item.second.begin));
			PyTuple_SET_ITEM(tup, 1, PyLong_FromSsize_t(item.second.end));
			PyDict_SetItemString(ret, item.first.c_str(), tup);
		}

		return ret;
	}

	PyObject* extract_file(PyObject*, PyObject* args)
	{
		PyObject* obj{};
		char const* file_creation_mode_str{};
		char const* src_name{};
		char const* dest_name{};

		assert(PyArg_ParseTuple(args, "Osss", &obj, &file_creation_mode_str, &src_name, &dest_name));

		auto const obj_ptr = PyLong_AsVoidPtr(obj);
		assert(obj_ptr != nullptr);
		auto& archive = *reinterpret_cast<Archive*>(obj_ptr);

		try
		{
			auto const file_creation_mode = fromString(std::type_identity<Wad64::FileCreationMode>{},
				file_creation_mode_str
			);
			extract(archive.archive, file_creation_mode, src_name, dest_name);
		}
		catch(std::exception const& err)
		{
			PyErr_SetString(PyExc_RuntimeError, err.what());
			return nullptr;
		}

		return Py_None;
	}

	PyObject* insert_file(PyObject*, PyObject* args)
	{
		PyObject* obj{};
		char const* file_creation_mode_str{};
		char const* src_name{};
		char const* dest_name{};

		assert(PyArg_ParseTuple(args, "Osss", &obj, &file_creation_mode_str, &src_name, &dest_name));

		auto const obj_ptr = PyLong_AsVoidPtr(obj);
		assert(obj_ptr != nullptr);
		auto& archive = *reinterpret_cast<Archive*>(obj_ptr);

		try
		{
			auto const file_creation_mode = fromString(std::type_identity<Wad64::FileCreationMode>{},
				file_creation_mode_str
			);
			insert(archive.archive, file_creation_mode, src_name, dest_name);
		}
		catch(std::exception const& err)
		{
			PyErr_SetString(PyExc_RuntimeError, err.what());
			return nullptr;
		}

		return Py_None;
	}

	PyObject* insert_data(PyObject*, PyObject* args)
	{
		PyObject* obj{};
		char const* file_creation_mode_str{};
		std::byte const* src_ptr{};
		ssize_t src_size{};
		char const* dest_name{};

		assert(PyArg_ParseTuple(args, "Osy#s", &obj, &file_creation_mode_str, &src_ptr, &src_size, &dest_name));

		auto const obj_ptr = PyLong_AsVoidPtr(obj);
		assert(obj_ptr != nullptr);
		auto& archive = *reinterpret_cast<Archive*>(obj_ptr);

		try
		{
			auto const file_creation_mode = fromString(std::type_identity<Wad64::FileCreationMode>{},
				file_creation_mode_str
			);

			insert(archive.archive, file_creation_mode, std::span{src_ptr, static_cast<size_t>(src_size)}, dest_name);
		}
		catch(std::exception const& err)
		{
			PyErr_SetString(PyExc_RuntimeError, err.what());
			return nullptr;
		}

		return Py_None;
	}

	PyObject* remove_file(PyObject*, PyObject* args)
	{
		PyObject* obj{};
		char const* filename{};

		assert(PyArg_ParseTuple(args, "Os", &obj, &filename));

		auto const obj_ptr = PyLong_AsVoidPtr(obj);
		assert(obj_ptr != nullptr);
		auto& archive = *reinterpret_cast<Archive*>(obj_ptr);

		try
		{
			archive.archive.remove(filename);
		}
		catch(std::exception const& err)
		{
			PyErr_SetString(PyExc_RuntimeError, err.what());
			return nullptr;
		}

		return Py_None;
	}

	PyObject* wipe_file(PyObject*, PyObject* args)
	{
		PyObject* obj{};
		char const* filename{};

		assert(PyArg_ParseTuple(args, "Os", &obj, &filename));

		auto const obj_ptr = PyLong_AsVoidPtr(obj);
		assert(obj_ptr != nullptr);
		auto& archive = *reinterpret_cast<Archive*>(obj_ptr);

		try
		{
			archive.archive.secureRemove(filename);
		}
		catch(std::exception const& err)
		{
			PyErr_SetString(PyExc_RuntimeError, err.what());
			return nullptr;
		}

		return Py_None;
	}

	constinit std::array<PyMethodDef, 9> method_table
	{
		PyMethodDef{"open_archive_from_path", open_archive_from_path, METH_VARARGS, ""},
		PyMethodDef{"close_archive", close_archive, METH_VARARGS, ""},
		PyMethodDef{"list_archive", list_archive, METH_VARARGS, ""},
		PyMethodDef{"extract_file", extract_file, METH_VARARGS, ""},
		PyMethodDef{"insert_file", insert_file, METH_VARARGS, ""},
		PyMethodDef{"insert_data", insert_data, METH_VARARGS, ""},
		PyMethodDef{"remove_file", remove_file, METH_VARARGS, ""},
		PyMethodDef{"wipe_file", wipe_file, METH_VARARGS, ""},
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