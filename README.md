# dx Achive readers
A minimalistic non compressed archive file readers written in ANSI C. Supported formats: [GNU tar](http://www.gnu.org/software/tar/manual/html_node/Standard.html) (tape archive) and [PAX](http://pubs.opengroup.org/onlinepubs/009695299/utilities/pax.html) in tar-compatibility mode, [GNU ar](https://en.wikipedia.org/wiki/Ar_(Unix)), [Cpio](https://en.wikipedia.org/wiki/Cpio) (binary for little- and big-endian machines, old an new ACSII).

* public domain
* single file with one function for each format ([tar](dxTarRead.c), [ar](dxArRead.c), binary [cpio](dxCpioRead.c))
* less than 50 lines of code, including comments, license and blanks
* or one file for [all formats](dxArchiveRead.c) - more complicated, but more powerful
* easy to use, just returns a pointer to your file inside archive
* no dependencies (even stdlibc not used), embedded friendly
* requires no memory/malloc or copying, uses only what is passed in
* optimized for high speed routing of small archive files, stops parsing upon match
* designed to work in pair with [tinfl.c](https://github.com/richgel999/miniz/blob/master/tinfl.c) from [miniz](https://github.com/richgel999/miniz)

## Functions:

````c
const char* dxTarRead(const void* tarData, const long tarSize, 
                      const char* fileName, long* fileSize)
````

Function returns pointer to `tarData`, where file data stored. If your file size can be divided evenly by 512, the data will be not NULL terminated! 
`fileSize` will be `0`, if there is no such file in the archive. `fileSize` will be `0` too if the archive is broken or unsupported.

For usage example see the [dxTarRead.c](dxTarRead.c). 

````c
const char* dxArRead(const void* arData, const long arSize, 
                      const char* fileName, long* fileSize)
````
The same. Data will *never* be NULL-terminated.

````c
const char* dxCpioRead(const void* cpioData, const long cpioSize,
                      const char* fileName, long* fileSize)
````
The same. Binary format only. If you need an ASCII-ones - use [dxArchiveRead](dxArchiveRead.c). Data will be NULL terminated only if `fileSize` is odd.

````c
const char* dxArchiveIterate(long* pp, const void* archiveData,
    const long archiveSize, long* fileSize, long* nameLength, long* dataDelta)
````
Available only in [dxArchiveRead.c](dxArchiveRead.c). Per-file iterator. `pp` - is the iterator itself, points to new file in archive. Just init it to `0` before using.  `fileSize`, `nameLength` and `dataDelta` are function outputs, all optional, can be replaced by `NULL`. `nameLength` is a name string length. Equals to `strlen(name)` in general case. `dataDelta` - pointer to current file content from the start of archive in bytes. See [dxArchiveRead.c](dxArchiveRead.c) for usage example.

## Building example:
Just define `DXTARREAD_EXAMPLE` or `DXARREAD_EXAMPLE` or `DXCPIOREAD_EXAMPLE` and compile:
```bash
  gcc -DDXTARREAD_EXAMPLE -Wall dxTarRead.c -o dxTarReadExample
```

## File naming and folders:
The `fileName` string must be the same, as archieving-time arguments. Example:
````bash
  tar -cvf myarchive.tar file1.txt dir/subfile.txt ./file2.txt ./dir2/*
  ar -cq myarchive.ar file1.txt dir/subfile.txt ./file2.txt ./dir2/file9.txt
  find . -depth -print | cpio -o > /path/myarchive.cpio
````
The `dxTarRead` will find `file1.txt` correctly, but not `./file1.txt`. The `dir/subfile.txt` is correct too. To find a file inside of `dir2`: `./dir2/file`.

You can get only files in directories, but not directory listing in TAR. The function will find `dir/`, but it's `fileSize` will be `0`. CPIO works the same.

The `dxArRead` will find `file1.txt`, `file2.txt` and `file9.txt` correctly. But *not* `./file2.txt`, `dir2/.file9.txt` or `./dir2/file9.txt`.

 There is no directories in AR.

## Supported versions
Only [GNU tar](http://www.gnu.org/software/tar/manual/html_node/Standard.html), GNU/System5 ar and GNU Cpio (binary, odc, newc). It's most modern formats, used by default in Linux (checked in Ubuntu and Arch Linux). Ar and Cpio may work on other format specifications, but not tested. 

## Supported compilers
The code is not compiler dependent. The `sizeof(char)` must be `1` on your compiler. Theoretically, you can use it with any compiler and operation system (OpenCL for example).

Tested compilers: GCC, Clang, TCC, Visual Studio.

## Credits:
Developed by Dmitry Hrabrov a.k.a. DeXPeriX and every direct or indirect contributor to the GitHub.

## License:
This software is dual-licensed to the public domain and under the following license: you are granted a perpetual, irrevocable license to copy, modify, publish and distribute this file as you see fit
