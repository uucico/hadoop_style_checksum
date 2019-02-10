# hdfs_style_checksum
This is a C program to calculate HDFS-like checksums on local files using MD5-of-0MD5-CRC32C method (only Castagnoli crc32 supported now). It uses OpenSSL's MD5 and Mark Adler's CRC32C digest implementations.

Based on https://github.com/srch07/HDFSChecksumForLocalfile.

Don't ask why I had to code this, it's a long story :-)
