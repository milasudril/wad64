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

	def ls(self):
		return wad64py.list_archive(self.handle)

	def extract_file(self, file_creation_mode, src_name ,dest_name):
		wad64py.extract_file(self.handle, file_creation_mode, src_name, dest_name)

	def extract_data(self, src_file):
		return wad64py.extract_data(self.handle, src_file)

	def insert_file(self, file_creation_mode, src_name, dest_name):
		wad64py.insert_file(self.handle, file_creation_mode, src_name, dest_name)

	def insert_data(self, file_creation_mode, src_data, dest_name):
		wad64py.insert_data(self.handle, file_creation_mode, src_data, dest_name)

	def remove_file(self, filename):
		wad64py.remove_file(self.handle, filename)

	def wipe_file(self, filename):
		wad64py.wipe_file(self.handle, filename)

if __name__ == '__main__':
	with Archive(path = 'test.wad64', io_mode = 'rw', file_creation_mode = 'cot') as archive:
		archive.insert_file('toc', '/etc/fstab', 'fstab')
