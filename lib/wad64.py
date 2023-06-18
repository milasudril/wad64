import wad64py

class ArchiveFile:
	def __init__(self, path):
		self.path = path

	def __enter__(self):
		self.handle = wad64py.open_file_for_insertion(self.path)
		return self

	def __exit__(self, type, value, traceback):
		wad64py.close_archive_file_rw(self.handle)

if __name__ == '__main__':
	with ArchiveFile('wad64') as archive:
		pass
