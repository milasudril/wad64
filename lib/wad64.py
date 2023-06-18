import wad64py

class Archive:
	def __init__(self, path, io_mode, file_creation_mode):
		self.path = path
		self.io_mode = io_mode
		self.file_creation_mode = file_creation_mode

	def __enter__(self):
		self.handle = wad64py.open_archive_from_path(self.path, self.io_mode, self.file_creation_mode)
		return self

	def __exit__(self, type, value, traceback):
		wad64py.close_archive(self.handle)

if __name__ == '__main__':
	with Archive(path = 'test.wad64', io_mode = 'rw', file_creation_mode = 'co') as archive:
		pass
