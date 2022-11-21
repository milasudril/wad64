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
		               Wad64::FileCreationMode::AllowOverwriteWithTruncation().allowCreation());
		assert(fd.fd != -1);
		write(fd, std::as_bytes(std::span{content}), 0);
		close(fd);
	}

	using ReadFunc  = ssize_t (*)(int, void* buf, size_t count, off_t offset);
	using WriteFunc = ssize_t (*)(int, void const* buf, size_t count, off_t offset);
	using WriteFuncFd = ssize_t (*)(int fdIn, loff_t* offIn,  int fdOut, loff_t* offOut, size_t len, unsigned int flags);

	using FailTest = bool (*)();

	constinit FailTest failCopyFileRange = [](){return false;};
}


extern "C"
{
	ssize_t pread(int fd, void* buf, size_t count, off_t offset)
	{
		auto real_func = reinterpret_cast<ReadFunc>(dlsym(RTLD_NEXT, "pread"));
		return real_func(fd, buf, std::min(count, static_cast<size_t>(4)), offset);
	}

	ssize_t pwrite(int fd, void const* buf, size_t count, off_t offset)
	{
		auto real_func = reinterpret_cast<WriteFunc>(dlsym(RTLD_NEXT, "pwrite"));
		return real_func(fd, buf, std::min(count, static_cast<size_t>(3)), offset);
	}

	ssize_t copy_file_range(int fdIn, loff_t* offIn,  int fdOut, loff_t* offOut, size_t count, unsigned int flags)
	{
		if(failCopyFileRange())
		{ return -1;}

		auto real_func = reinterpret_cast<WriteFuncFd>(dlsym(RTLD_NEXT, "copy_file_range"));

		return real_func(fdIn, offIn, fdOut, offOut, std::min(count, static_cast<size_t>(5)), flags);
	}
}

namespace Testcases
{
	void wad64FdAdapterOpenFileDoesNotExistReadAllowedOverwriteAllowed()
	{
		auto const test_dir = std::filesystem::path{MAIKE_BUILDINFO_TARGETDIR};
		auto const filename = test_dir / X_STR(MAIKE_TASKID);
		(void)unlink(filename.c_str());
		auto fd = open(filename.c_str(),
		               Wad64::IoMode::AllowRead(),
		               Wad64::FileCreationMode::AllowOverwriteWithTruncation());
		assert(fd.fd == -1);
	}

	void wad64FdAdapterOpenFileDoesNotExistReadAllowedCreationAllowed()
	{
		auto const test_dir = std::filesystem::path{MAIKE_BUILDINFO_TARGETDIR};
		auto const filename = test_dir / X_STR(MAIKE_TASKID);
		(void)unlink(filename.c_str());
		auto fd = open(
		    filename.c_str(), Wad64::IoMode::AllowRead(), Wad64::FileCreationMode::AllowCreation());
		assert(fd.fd == -1);
	}

	void wad64FdAdapterOpenFileDoesNotExistReadAllowedCreationAllowedOverwriteAllowed()
	{
		auto const test_dir = std::filesystem::path{MAIKE_BUILDINFO_TARGETDIR};
		auto const filename = test_dir / X_STR(MAIKE_TASKID);
		(void)unlink(filename.c_str());
		auto fd = open(filename.c_str(),
		               Wad64::IoMode::AllowRead(),
		               Wad64::FileCreationMode::AllowOverwriteWithTruncation().allowCreation());
		assert(fd.fd == -1);
	}

	void wad64FdAdapterOpenFileDoesNotExistWriteAllowedOverwriteAllowed()
	{
		auto const test_dir = std::filesystem::path{MAIKE_BUILDINFO_TARGETDIR};
		auto const filename = test_dir / X_STR(MAIKE_TASKID);
		(void)unlink(filename.c_str());
		auto fd = open(filename.c_str(),
		               Wad64::IoMode::AllowWrite(),
		               Wad64::FileCreationMode::AllowOverwriteWithTruncation());
		assert(fd.fd == -1);
	}

	void wad64FdAdapterOpenFileDoesNotExistWriteAllowedCreationAllowed()
	{
		auto const test_dir = std::filesystem::path{MAIKE_BUILDINFO_TARGETDIR};
		auto const filename = test_dir / X_STR(MAIKE_TASKID);
		(void)unlink(filename.c_str());
		auto fd = open(filename.c_str(),
		               Wad64::IoMode::AllowWrite(),
		               Wad64::FileCreationMode::AllowCreation());
		assert(fd.fd != -1);
		close(fd);
	}

	void wad64FdAdapterOpenFileDoesNotExistWriteAllowedCreationAllowedOverwriteAllowed()
	{
		auto const test_dir = std::filesystem::path{MAIKE_BUILDINFO_TARGETDIR};
		auto const filename = test_dir / X_STR(MAIKE_TASKID);
		auto fd = open(filename.c_str(),
		               Wad64::IoMode::AllowWrite(),
		               Wad64::FileCreationMode::AllowCreation().allowOverwriteWithoutTruncation());
		(void)unlink(filename.c_str());
		assert(fd.fd != -1);
		close(fd);
	}

	void wad64FdAdapterOpenFileExistsReadAllowedOverwriteAllowed()
	{
		auto const test_dir = std::filesystem::path{MAIKE_BUILDINFO_TARGETDIR};
		auto const filename = test_dir / X_STR(MAIKE_TASKID);
		createFile(filename.c_str(), "This is a test file");

		auto fd = open(filename.c_str(),
		               Wad64::IoMode::AllowRead(),
		               Wad64::FileCreationMode::AllowOverwriteWithTruncation());
		(void)unlink(filename.c_str());
		assert(fd.fd != -1);
		close(fd.fd);
	}

	void wad64FdAdapterOpenFileExistsReadAllowedCreationAllowed()
	{
		auto const test_dir = std::filesystem::path{MAIKE_BUILDINFO_TARGETDIR};
		auto const filename = test_dir / X_STR(MAIKE_TASKID);
		createFile(filename.c_str(), "This is a test file");

		auto fd = open(
		    filename.c_str(), Wad64::IoMode::AllowRead(), Wad64::FileCreationMode::AllowCreation());
		(void)unlink(filename.c_str());

		assert(fd.fd != -1);
		close(fd.fd);
	}

	void wad64FdAdapterOpenFileExistsReadAllowedCreationAllowedOverwriteAllowed()
	{
		auto const test_dir = std::filesystem::path{MAIKE_BUILDINFO_TARGETDIR};
		auto const filename = test_dir / X_STR(MAIKE_TASKID);
		createFile(filename.c_str(), "This is a test file");

		auto fd = open(filename.c_str(),
		               Wad64::IoMode::AllowRead(),
		               Wad64::FileCreationMode::AllowOverwriteWithTruncation().allowCreation());
		(void)unlink(filename.c_str());

		assert(fd.fd != -1);
		close(fd.fd);
	}

	void wad64FdAdapterOpenFileExistsWriteAllowedOverwriteAllowed()
	{
		auto const test_dir = std::filesystem::path{MAIKE_BUILDINFO_TARGETDIR};
		auto const filename = test_dir / X_STR(MAIKE_TASKID);
		createFile(filename.c_str(), "This is a test file");
		struct stat statbuf;
		stat(filename.c_str(), &statbuf);
		auto const old_size = statbuf.st_size;
		assert(old_size != 0);
		auto fd = open(filename.c_str(),
		               Wad64::IoMode::AllowWrite(),
		               Wad64::FileCreationMode::AllowOverwriteWithTruncation());
		(void)unlink(filename.c_str());
		assert(fd.fd != -1);
		fstat(fd.fd, &statbuf);
		assert(statbuf.st_size == 0);
		close(fd.fd);
	}

	void wad64FdAdapterOpenFileExistsWriteAllowedCreationAllowed()
	{
		auto const test_dir = std::filesystem::path{MAIKE_BUILDINFO_TARGETDIR};
		auto const filename = test_dir / X_STR(MAIKE_TASKID);
		createFile(filename.c_str(), "This is a test file");

		auto fd = open(filename.c_str(),
		               Wad64::IoMode::AllowWrite(),
		               Wad64::FileCreationMode::AllowCreation());
		(void)unlink(filename.c_str());
		assert(fd.fd == -1);
	}

	void wad64FdAdapterOpenFileExistsWriteAllowedCreationAllowedOverwriteAllowed()
	{
		auto const test_dir = std::filesystem::path{MAIKE_BUILDINFO_TARGETDIR};
		auto const filename = test_dir / X_STR(MAIKE_TASKID);
		createFile(filename.c_str(), "This is a test file");
		struct stat statbuf;
		stat(filename.c_str(), &statbuf);
		auto const old_size = statbuf.st_size;
		assert(old_size != 0);
		auto fd = open(filename.c_str(),
		               Wad64::IoMode::AllowWrite(),
		               Wad64::FileCreationMode::AllowCreation().allowOverwriteWithoutTruncation());
		(void)unlink(filename.c_str());
		assert(fd.fd != -1);
		fstat(fd.fd, &statbuf);
		assert(statbuf.st_size == old_size);
		close(fd.fd);
	}

	void wad64FdAdapterReadFromOffset()
	{
		auto const test_dir = std::filesystem::path{MAIKE_BUILDINFO_TARGETDIR};
		auto const filename       = test_dir / X_STR(MAIKE_TASKID);
		constexpr auto content_sv = std::string_view{"This is a test file"};
		auto content              = std::string{content_sv};
		createFile(filename.c_str(), content);

		std::array<char, 2 * std::size(content_sv)> buffer{};
		auto fd = open(
		    filename.c_str(), Wad64::IoMode::AllowRead(), Wad64::FileCreationMode::AllowCreation());
		unlink(filename.c_str());
		assert(fd.fd != -1);
		auto n = read(fd, std::as_writable_bytes(std::span{buffer}), 5);
		assert(n == strlen("is a test file"));
		assert((std::equal(std::begin(buffer),
		                   std::begin(buffer) + n,
		                   std::begin(std::string_view{"is a test file"}))));
	}

	void wad64FdAdapterReadCompleted()
	{
		auto const test_dir = std::filesystem::path{MAIKE_BUILDINFO_TARGETDIR};
		auto const filename       = test_dir / X_STR(MAIKE_TASKID);
		constexpr auto content_sv = std::string_view{"This is a test file"};
		auto content              = std::string{content_sv};
		createFile(filename.c_str(), content);

		std::array<char, std::size(content_sv)> buffer{};
		auto fd = open(
		    filename.c_str(), Wad64::IoMode::AllowRead(), Wad64::FileCreationMode::AllowCreation());
		unlink(filename.c_str());
		assert(fd.fd != -1);
		auto n = read(fd, std::as_writable_bytes(std::span{buffer}), 0);
		assert(n == std::size(content_sv));
		assert(std::equal(std::begin(buffer), std::begin(buffer) + n, std::begin(content_sv)));
	}

	void wad64FdAdapterCreateTempFile()
	{
		auto fd = Wad64::createTempFile("/tmp");
		assert(fd.fd != -1);
		close(fd);
	}

	void wad64FdAdapterWriteFromFdEmptyFile()
	{
		failCopyFileRange = [](){return false;};

		auto const test_dir = std::filesystem::path{MAIKE_BUILDINFO_TARGETDIR};
		auto filename_a       = test_dir / X_STR(MAIKE_TASKID);
		filename_a.concat("_a");
		auto  filename_b       = test_dir / X_STR(MAIKE_TASKID);
		filename_b.concat("_b");

		fflush(stdout);
		auto fd_a = open(filename_a.c_str(),
				Wad64::IoMode::AllowWrite().allowRead(),
				Wad64::FileCreationMode::AllowOverwriteWithTruncation().allowCreation());
		unlink(filename_a.c_str());
		assert(fd_a.fd != -1);

		auto fd_b = open(filename_b.c_str(),
				Wad64::IoMode::AllowWrite().allowRead(),
				Wad64::FileCreationMode::AllowOverwriteWithTruncation().allowCreation());
		unlink(filename_b.c_str());
		assert(fd_b.fd != -1);

		auto res = write(fd_b, fd_a, 3);
		assert(res == size(fd_a));
		assert(size(fd_b) == 0);

		close(fd_a);
		close(fd_b);
	}

	void wad64FdAdapterWriteFromFdAdapterKernelTransfer()
	{
		failCopyFileRange = [](){return false;};

		auto const test_dir = std::filesystem::path{MAIKE_BUILDINFO_TARGETDIR};
		auto filename_a       = test_dir / X_STR(MAIKE_TASKID);
		filename_a.concat("_a");
		auto  filename_b       = test_dir / X_STR(MAIKE_TASKID);
		filename_b.concat("_b");
		constexpr auto content_sv = std::string_view{"This is a test file"};
		auto content              = std::string{content_sv};

		fflush(stdout);
		auto fd_a = open(filename_a.c_str(),
				Wad64::IoMode::AllowWrite().allowRead(),
				Wad64::FileCreationMode::AllowOverwriteWithTruncation().allowCreation());
		unlink(filename_a.c_str());
		assert(fd_a.fd != -1);
		write(fd_a, std::as_bytes(std::span{content}), 0);

		auto fd_b = open(filename_b.c_str(),
				Wad64::IoMode::AllowWrite().allowRead(),
				Wad64::FileCreationMode::AllowOverwriteWithTruncation().allowCreation());
		unlink(filename_b.c_str());
		assert(fd_b.fd != -1);

		auto res = write(fd_b, fd_a, 5);
		assert(res == size(fd_a));
		assert(size(fd_b) == 5 + size(fd_a));

		close(fd_a);

		std::array<char, std::size(content_sv)> buffer;

		auto n = read(fd_b, std::as_writable_bytes(std::span{buffer}), 0);
		assert(n == std::size(content_sv));
		assert((std::equal(std::begin(buffer) + 5,
		                   std::end(buffer),
		                   std::begin(content_sv))));
		close(fd_b);
	}

	void wad64FdAdapterWriteFromFdAdapterKernelTransferFailSecondWrite()
	{
		static size_t count = 0;
		failCopyFileRange = []()
		{
			++count;
			if(count < 2) { return false; }

			errno = EIO;
			return true;
		};

		auto const test_dir = std::filesystem::path{MAIKE_BUILDINFO_TARGETDIR};
		auto filename_a       = test_dir / X_STR(MAIKE_TASKID);
		filename_a.concat("_a");
		auto  filename_b       = test_dir / X_STR(MAIKE_TASKID);
		filename_b.concat("_b");
		constexpr auto content_sv = std::string_view{"This is a test file"};
		auto content              = std::string{content_sv};

		fflush(stdout);
		auto fd_a = open(filename_a.c_str(),
				Wad64::IoMode::AllowWrite().allowRead(),
				Wad64::FileCreationMode::AllowOverwriteWithTruncation().allowCreation());
		unlink(filename_a.c_str());
		assert(fd_a.fd != -1);
		write(fd_a, std::as_bytes(std::span{content}), 0);

		auto fd_b = open(filename_b.c_str(),
				Wad64::IoMode::AllowWrite().allowRead(),
				Wad64::FileCreationMode::AllowOverwriteWithTruncation().allowCreation());
		unlink(filename_b.c_str());
		assert(fd_b.fd != -1);

		try
		{
			write(fd_b, fd_a, 5);
			abort();
		}
		catch(...)
		{}

		close(fd_a);
		close(fd_b);
	}

	void wad64FdAdapterWriteFromFdAdapterFailKernelTransfer()
	{
		failCopyFileRange = [](){return true;};

		auto const test_dir = std::filesystem::path{MAIKE_BUILDINFO_TARGETDIR};
		auto filename_a       = test_dir / X_STR(MAIKE_TASKID);
		filename_a.concat("_a");
		auto  filename_b       = test_dir / X_STR(MAIKE_TASKID);
		filename_b.concat("_b");
		constexpr auto content_sv = std::string_view{"This is a test file"};
		auto content              = std::string{content_sv};

		fflush(stdout);
		auto fd_a = open(filename_a.c_str(),
				Wad64::IoMode::AllowWrite().allowRead(),
				Wad64::FileCreationMode::AllowOverwriteWithTruncation().allowCreation());
		unlink(filename_a.c_str());
		assert(fd_a.fd != -1);
		write(fd_a, std::as_bytes(std::span{content}), 0);

		auto fd_b = open(filename_b.c_str(),
				Wad64::IoMode::AllowWrite().allowRead(),
				Wad64::FileCreationMode::AllowOverwriteWithTruncation().allowCreation());
		unlink(filename_b.c_str());
		assert(fd_b.fd != -1);

		auto res = write(fd_b, fd_a, 5);
		assert(res == size(fd_a));
		assert(size(fd_b) == 5 + size(fd_a));

		close(fd_a);

		std::array<char, std::size(content_sv)> buffer;

		auto n = read(fd_b, std::as_writable_bytes(std::span{buffer}), 0);
		assert(n == std::size(content_sv));
		assert((std::equal(std::begin(buffer) + 5,
		                   std::end(buffer),
		                   std::begin(content_sv))));
		close(fd_b);
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

	Testcases::wad64FdAdapterCreateTempFile();

	Testcases::wad64FdAdapterWriteFromFdEmptyFile();
	Testcases::wad64FdAdapterWriteFromFdAdapterKernelTransfer();
	Testcases::wad64FdAdapterWriteFromFdAdapterKernelTransferFailSecondWrite();
	Testcases::wad64FdAdapterWriteFromFdAdapterFailKernelTransfer();
	return 0;
}