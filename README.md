# WAD64

WAD64 is a modern version of the classic DOOM archive format. It is more or less an identical
archive file format, with larger fields.

## Features

* Full random access to individual lumps
* 64-bit file offsets
* Up to 2^63 - 1 directory entries
* 255 byte long lump names

## Differences between classic DOOM WAD and WAD64

* WAD64 lump names should be encoded as UTF-8. If a lump name contains any null bytes, the must
  appear last. Also, the lump name must end with a null byte. That is why only 255 bytes are
  allowed. The last byte must always be a null byte.

* WAD64 forbids overlapping lumps

## Common archive features that are not supported nativly

* Hierarchical directory structures. However, since it is possible to have longer lump names, it
  can be emulated by follwing some convtion. For example, it is possible to us `/` in lump names.
  Another possiblity is to have WAD:s inside WAD:s. This works, because the file format allows
  full random access.

* Compression/encryption. It is possible to add compressed/encrypted lumps (in fact, a lump can
  contain anything), provided that the reader knows what compression/encryption scheme is used. The
  directory cannot be compressed/encryption, because then it is not possible to find the data.

* File metadata

* Checksum