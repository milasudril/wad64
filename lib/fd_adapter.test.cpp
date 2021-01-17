//@	{"targets":[{"name":"fd_adapter.test","type":"application", "dependencies":[{"ref":"dl","rel":"external"}], "autorun":1}]}

#include "./fd_adapter.hpp"
#include "./io_policy.hpp"

#include <sys/stat.h>
#include <dlfcn.h>

#include <cassert>
#include <cstdio>
#include <filesystem>
#include <cstring>

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

using ReadFunc = ssize_t (*)(int, void* buf, size_t count, off_t offset);
using WriteFunc   = ssize_t (*)(int, void const* buf, size_t count, off_t offset);

extern "C"
{
	ssize_t pread(int fd, void* buf, size_t count, off_t offset)
	{
		auto real_func = reinterpret_cast<ReadFunc>(dlsym(RTLD_NEXT, "pread"));
		return real_func(fd, buf, std::max(count, static_cast<size_t>(4)), offset);
	}

	ssize_t pwrite(int fd, void const* buf, size_t count, off_t offset)
	{
		auto real_func = reinterpret_cast<WriteFunc>(dlsym(RTLD_NEXT, "pwrite"));
		return real_func(fd, buf, std::max(count, static_cast<size_t>(3)), offset);
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

	void wad64FdAdapterReadFromOffset()
	{
		auto const test_dir =
		    std::filesystem::path{X_STR(MAIKE_TARGET_DIRECTORY)} / X_STR(MAIKE_CURRENT_DIRECTORY);

		auto const filename = test_dir / "my_file";
		constexpr auto content_sv = std::string_view{"This is a test file"};
		auto content = std::string{content_sv};
		createFile(filename.c_str(), content);

		std::array<char, 2*std::size(content_sv)> buffer{};
		auto fd = open(filename.c_str(), Wad64::IoMode::AllowRead(), Wad64::FileCreationMode::AllowCreation());
		unlink(filename.c_str());
		assert(fd.fd != -1);
		auto n = read(fd, std::as_writable_bytes(std::span{buffer}), 5);
		assert(n == strlen("is a test file"));
		assert((std::equal(std::begin(buffer), std::begin(buffer) + n, std::begin(std::string_view{"is a test file"}))));
	}

	void wad64FdAdapterReadCompleted()
	{
		auto const test_dir =
		    std::filesystem::path{X_STR(MAIKE_TARGET_DIRECTORY)} / X_STR(MAIKE_CURRENT_DIRECTORY);

		auto const filename = test_dir / "my_file";
		constexpr auto content_sv = std::string_view{"This is a test file"};
		auto content = std::string{content_sv};
		createFile(filename.c_str(), content);

		std::array<char, std::size(content_sv)> buffer{};
		auto fd = open(filename.c_str(), Wad64::IoMode::AllowRead(), Wad64::FileCreationMode::AllowCreation());
		unlink(filename.c_str());
		assert(fd.fd != -1);
		auto n = read(fd, std::as_writable_bytes(std::span{buffer}), 0);
		assert(n == std::size(content_sv));
		assert(std::equal(std::begin(buffer), std::begin(buffer) + n, std::begin(content_sv)));
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

	Testcases::wad64FdAdapterReadFromOffset();
	Testcases::wad64FdAdapterReadCompleted();
	return 0;
}