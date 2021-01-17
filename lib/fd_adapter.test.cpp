//@	{"targets":[{"name":"fd_adapter.test","type":"application", "autorun":1}]}

#include "./fd_adapter.hpp"
#include "./io_policy.hpp"

#include <sys/stat.h>

#include <cassert>
#include <cstdio>
#include <filesystem>

static_assert(Wad64::RandomAccessFile<Wad64::FdAdapter>);

#define X_STR(s) STR(s)
#define STR(s) #s

namespace
{
	void createFile(char const* filename, std::string const& content)
	{
		auto fd = open(filename,
		               Wad64::IoMode::AllowWrite(),
		               Wad64::FileCreationMode::AllowOverwrite().allowCreation());
		assert(fd.fd != -1);
		write(fd, std::as_bytes(std::span{content}), 0);
		close(fd);
	}
}

namespace Testcases
{
	void wad64FdAdapterOpenFileDoesNotExistReadAllowedOverwriteAllowed()
	{
		auto const test_dir =
		    std::filesystem::path{X_STR(MAIKE_TARGET_DIRECTORY)} / X_STR(MAIKE_CURRENT_DIRECTORY);

		auto const filename = test_dir / "my_file";

		(void)unlink(filename.c_str());
		auto fd = open(filename.c_str(),
		               Wad64::IoMode::AllowRead(),
		               Wad64::FileCreationMode::AllowOverwrite());
		assert(fd.fd == -1);
	}

	void wad64FdAdapterOpenFileDoesNotExistReadAllowedCreationAllowed()
	{
		auto const test_dir =
		    std::filesystem::path{X_STR(MAIKE_TARGET_DIRECTORY)} / X_STR(MAIKE_CURRENT_DIRECTORY);

		auto const filename = test_dir / "my_file";

		(void)unlink(filename.c_str());
		auto fd = open(
		    filename.c_str(), Wad64::IoMode::AllowRead(), Wad64::FileCreationMode::AllowCreation());
		assert(fd.fd == -1);
	}

	void wad64FdAdapterOpenFileDoesNotExistReadAllowedCreationAllowedOverwriteAllowed()
	{
		auto const test_dir =
		    std::filesystem::path{X_STR(MAIKE_TARGET_DIRECTORY)} / X_STR(MAIKE_CURRENT_DIRECTORY);

		auto const filename = test_dir / "my_file";

		(void)unlink(filename.c_str());
		auto fd = open(filename.c_str(),
		               Wad64::IoMode::AllowRead(),
		               Wad64::FileCreationMode::AllowOverwrite().allowCreation());
		assert(fd.fd == -1);
	}

	void wad64FdAdapterOpenFileDoesNotExistWriteAllowedOverwriteAllowed()
	{
		auto const test_dir =
		    std::filesystem::path{X_STR(MAIKE_TARGET_DIRECTORY)} / X_STR(MAIKE_CURRENT_DIRECTORY);

		auto const filename = test_dir / "my_file";

		(void)unlink(filename.c_str());
		auto fd = open(filename.c_str(),
		               Wad64::IoMode::AllowWrite(),
		               Wad64::FileCreationMode::AllowOverwrite());
		assert(fd.fd == -1);
	}

	void wad64FdAdapterOpenFileDoesNotExistWriteAllowedCreationAllowed()
	{
		auto const test_dir =
		    std::filesystem::path{X_STR(MAIKE_TARGET_DIRECTORY)} / X_STR(MAIKE_CURRENT_DIRECTORY);

		auto const filename = test_dir / "my_file";

		(void)unlink(filename.c_str());
		auto fd = open(filename.c_str(),
		               Wad64::IoMode::AllowWrite(),
		               Wad64::FileCreationMode::AllowCreation());
		assert(fd.fd != -1);
		close(fd);
	}

	void wad64FdAdapterOpenFileDoesNotExistWriteAllowedCreationAllowedOverwriteAllowed()
	{
		auto const test_dir =
		    std::filesystem::path{X_STR(MAIKE_TARGET_DIRECTORY)} / X_STR(MAIKE_CURRENT_DIRECTORY);

		auto const filename = test_dir / "my_file";

		auto fd = open(filename.c_str(),
		               Wad64::IoMode::AllowWrite(),
		               Wad64::FileCreationMode::AllowCreation().allowOverwrite());
		(void)unlink(filename.c_str());
		assert(fd.fd != -1);
		close(fd);
	}
	//

	void wad64FdAdapterOpenFileExistsReadAllowedOverwriteAllowed()
	{
		auto const test_dir =
		    std::filesystem::path{X_STR(MAIKE_TARGET_DIRECTORY)} / X_STR(MAIKE_CURRENT_DIRECTORY);

		auto const filename = test_dir / "my_file";
		createFile(filename.c_str(), "This is a test file");

		auto fd = open(filename.c_str(),
		               Wad64::IoMode::AllowRead(),
		               Wad64::FileCreationMode::AllowOverwrite());
		(void)unlink(filename.c_str());
		assert(fd.fd != -1);
		close(fd.fd);
	}

	void wad64FdAdapterOpenFileExistsReadAllowedCreationAllowed()
	{
		auto const test_dir =
		    std::filesystem::path{X_STR(MAIKE_TARGET_DIRECTORY)} / X_STR(MAIKE_CURRENT_DIRECTORY);

		auto const filename = test_dir / "my_file";
		createFile(filename.c_str(), "This is a test file");

		auto fd = open(
		    filename.c_str(), Wad64::IoMode::AllowRead(), Wad64::FileCreationMode::AllowCreation());
		(void)unlink(filename.c_str());

		assert(fd.fd != -1);
		close(fd.fd);
	}

	void wad64FdAdapterOpenFileExistsReadAllowedCreationAllowedOverwriteAllowed()
	{
		auto const test_dir =
		    std::filesystem::path{X_STR(MAIKE_TARGET_DIRECTORY)} / X_STR(MAIKE_CURRENT_DIRECTORY);

		auto const filename = test_dir / "my_file";
		createFile(filename.c_str(), "This is a test file");

		auto fd = open(filename.c_str(),
		               Wad64::IoMode::AllowRead(),
		               Wad64::FileCreationMode::AllowOverwrite().allowCreation());
		(void)unlink(filename.c_str());

		assert(fd.fd != -1);
		close(fd.fd);
	}

	void wad64FdAdapterOpenFileExistsWriteAllowedOverwriteAllowed()
	{
		auto const test_dir =
		    std::filesystem::path{X_STR(MAIKE_TARGET_DIRECTORY)} / X_STR(MAIKE_CURRENT_DIRECTORY);

		auto const filename = test_dir / "my_file";
		createFile(filename.c_str(), "This is a test file");
		struct stat statbuf;
		stat(filename.c_str(), &statbuf);
		auto const old_size = statbuf.st_size;
		assert(old_size != 0);
		auto fd = open(filename.c_str(),
		               Wad64::IoMode::AllowWrite(),
		               Wad64::FileCreationMode::AllowOverwrite());
		(void)unlink(filename.c_str());
		assert(fd.fd != -1);
		fstat(fd.fd, &statbuf);
		assert(statbuf.st_size == 0);
		close(fd.fd);
	}

	void wad64FdAdapterOpenFileExistsWriteAllowedCreationAllowed()
	{
		auto const test_dir =
		    std::filesystem::path{X_STR(MAIKE_TARGET_DIRECTORY)} / X_STR(MAIKE_CURRENT_DIRECTORY);

		auto const filename = test_dir / "my_file";
		createFile(filename.c_str(), "This is a test file");

		auto fd = open(filename.c_str(),
		               Wad64::IoMode::AllowWrite(),
		               Wad64::FileCreationMode::AllowCreation());
		(void)unlink(filename.c_str());
		assert(fd.fd == -1);
		close(fd);
	}

	void wad64FdAdapterOpenFileExistsWriteAllowedCreationAllowedOverwriteAllowed()
	{
		auto const test_dir =
		    std::filesystem::path{X_STR(MAIKE_TARGET_DIRECTORY)} / X_STR(MAIKE_CURRENT_DIRECTORY);

		auto const filename = test_dir / "my_file";
		createFile(filename.c_str(), "This is a test file");
		struct stat statbuf;
		stat(filename.c_str(), &statbuf);
		auto const old_size = statbuf.st_size;
		assert(old_size != 0);
		auto fd = open(filename.c_str(),
		               Wad64::IoMode::AllowWrite(),
		               Wad64::FileCreationMode::AllowCreation().allowOverwrite());
		(void)unlink(filename.c_str());
		assert(fd.fd != -1);
		fstat(fd.fd, &statbuf);
		assert(statbuf.st_size == 0);
		close(fd.fd);
	}
}

int main()
{
	Testcases::wad64FdAdapterOpenFileDoesNotExistReadAllowedOverwriteAllowed();
	Testcases::wad64FdAdapterOpenFileDoesNotExistReadAllowedCreationAllowed();
	Testcases::wad64FdAdapterOpenFileDoesNotExistReadAllowedCreationAllowedOverwriteAllowed();
	Testcases::wad64FdAdapterOpenFileDoesNotExistWriteAllowedOverwriteAllowed();
	Testcases::wad64FdAdapterOpenFileDoesNotExistWriteAllowedCreationAllowed();
	Testcases::wad64FdAdapterOpenFileDoesNotExistWriteAllowedCreationAllowedOverwriteAllowed();

	Testcases::wad64FdAdapterOpenFileExistsReadAllowedOverwriteAllowed();
	Testcases::wad64FdAdapterOpenFileExistsReadAllowedCreationAllowed();
	Testcases::wad64FdAdapterOpenFileExistsReadAllowedCreationAllowedOverwriteAllowed();
	Testcases::wad64FdAdapterOpenFileExistsWriteAllowedOverwriteAllowed();
	Testcases::wad64FdAdapterOpenFileExistsWriteAllowedCreationAllowed();
	Testcases::wad64FdAdapterOpenFileExistsWriteAllowedCreationAllowedOverwriteAllowed();
	return 0;
}