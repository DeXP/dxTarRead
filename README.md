# dxTarRead
A minimalistic GNU tar (tape archive) file reader written in ANSI C.

* public domain, single [dxTarRead.c](dxTarRead.c) file with one function
* less than 50 lines of code, including comments and blanks
* easy to use, just returns a pointer to your file inside [GNU tar](http://www.gnu.org/software/tar/manual/html_node/Standard.html)
* no dependencies (even stdlibc not used), embedded friendly
* requires no memory/malloc or copying, uses only what is passed in
* optimized for high speed routing of small tar-files, stops parsing upon match
* designed to work in pair with [tinfl.c](https://github.com/richgel999/miniz/blob/master/tinfl.c) from [miniz](https://github.com/richgel999/miniz)

````c
const char* dxTarRead(const void* tarData, const long tarSize, 
                      const char* fileName, long* fileSize)
````

Function returns pointer to `tarData`, where file data stored. If your file size can be divided evenly by 512, the data will be not NULL terminated! 
`fileSize` will be `0`, if there is no such file in the archive. `fileSize` will be `0` too if the archive is broken or unsupported.

For usage example see the [dxTarRead.c](dxTarRead.c). 


## Building example:
Just define `DXTARREAD_EXAMPLE` and compile:
```bash
  gcc -DDXTARREAD_EXAMPLE -Wall dxTarRead.c -o dxTarReadExample
```

## File naming and folders:
The `fileName` string must be the same, as achieving-time arguments. Example:
````bash
  tar -cvf myarchive.tar file1.txt dir/subfile.txt ./file2.txt ./dir2/*
````
The function will find `file1.txt` correctly, but not `./file1.txt`. The `dir/subfile.txt` is correct too. To find a file inside of `dir2`: `./dir2/file`.

You can get only files in directories, but not directory listing. The function will find `dir/`, but it's `fileSize` will be `0`.

## Supported tar versions
Only [GNU tar](http://www.gnu.org/software/tar/manual/html_node/Standard.html). But it's most modern format, used by default in Linux (checked in Ubuntu and Arch Linux).

## Supported compilers
The code is not compiler dependent. The `sizeof(char)` must be `1` on your compiler. Theoretically, you can use it with any compiler and operation system (OpenCL for example).

Tested compilers: GCC, Clang, TCC, Visual Studio.

## Credits:
Developed by Dmitry Hrabrov a.k.a. DeXPeriX and every direct or indirect contributor to the GitHub.

## License:
This software is dual-licensed to the public domain and under the following license: you are granted a perpetual, irrevocable license to copy, modify, publish and distribute this file as you see fit
