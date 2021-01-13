# WAD64

WAD64 is a modern version of the classic DOOM archive format. It is more or less an identical
archive file format, with larger fields.

## Features

* Full random access to individual lumps
* 64-bit file offsets
* Up to 2^63 - 1 directory entries
* 255 character long lump names

## Features that are not supported nativly

* Hierarchical directory structures. However, since it is possible to have longer lump names, it
  can be emulated by follwing some convtion. For example, it is possible to us `/` in lump names.

* Compression or encryption. It is possible to add compressed/encrypted lumps (in fact, a lump can contain anything), provided that the reader knows what compression/encryption scheme is used. The directory cannot be compressed/encryption.