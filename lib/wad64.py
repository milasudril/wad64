import wad64py

class ArchiveFile:
	def __init__(self, path, io_mode, file_creation_mode):
		self.path = path
		self.io_mode = io_mode
		self.file_creation_mode = file_creation_mode

	def __enter__(self):
		self.handle = wad64py.open_archive_file(self.path, self.io_mode, self.file_creation_mode)
		return self

	def __exit__(self, type, value, traceback):
		wad64py.close_archive_file(self.handle)

if __name__ == '__main__':
	with ArchiveFile('test.wad64', 'rw', 'co') as archive:
		pass
