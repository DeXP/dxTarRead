/*
 dxArchiveRead - 0.7 - public domain
 no warrenty implied; use at your own risk.
 authored from 2017 by Dmitry Hrabrov a.k.a. DeXPeriX, http://dexperix.net

LICENSE: This software is dual-licensed to the public domain and under the
    following license: you are granted a perpetual, irrevocable license to
    copy, modify, publish and distribute this file as you see fit.
*/
const char* dxArchiveRead(const void* archiveData, const long archiveSize,
                      const char* fileName, long* fileSize)
{
    const int FILE_SZ = 0, NAME_SZ = 1, DX_BIN_CPIO = 1, DX_NEWC_CPIO = 3;
    const char bm[][2] = { {0xC7, 0x71}, {0x71, 0xC7}, {0x60, 0x0A} };
    enum { MAGS=0,MAGO,MUL,BSIZE,FNAME,SZOF,SZSIZE,NLOF,NLSIZE,DIVTO,BIT,IDC };
    static const int inf[6][IDC]={ {2, 0,  10, 26,  26,  22,  0,  20, 0, 2, 0},
     /* Binary CPIO, big endian */ {2, 0,  10, 26,  26,  22,  0,  20, 0, 2, 1},
       /* Old Ascii CPIO = odc  */ {6, 0,   8, 76,  76,  65,  11, 59, 6, 0, 1},
       /* New Ascii CPIO = newc */ {6, 0,  16, 110, 110, 54,  8,  94, 8, 4, 0},
                     /* GNU tar */ {5, 257, 8, 512, 0,   124, 11, 0,  0,512,0},
                      /* GNU ar */ {2, 58, 10, 60,  0,   48,  10, 0,  0, 2,0}};
    const char* MAGIC[6] = { bm[0], bm[1], "070707", "070701", "ustar", bm[2]};
    const unsigned char* u = (const unsigned char*) archiveData;
    long p =0, newOffset = 0, found = 0, v[2], fmt, i, j, mul;

    *fileSize = 0; /* will be zero if archive wrong or there is no such file */
    if( (u[0]=='!') && (u[1]=='<') && (u[2]=='a') ) p = 8; /* possible "ar" */
    do { /* "Load" data from archive - just point to passed memory */
        p += newOffset; fmt = -1; /* Move to new offset. Guess format again */
        for(j=0; j<6; j++){ /* Circle on formats */
            for(i=0, mul=1; i<inf[j][MAGS]; i++){ /* compare with magic */
                if( u[i + p + inf[j][MAGO] ] != (unsigned char)MAGIC[j][i] )
                    { mul = 0; break; } /* chars are not equal, exit */
            } if( mul ){ fmt = j; break; } /* correct format found */
        };
        if( fmt >=0 ){
            const int* cf = inf[fmt]; /* Pointer to format's offsets etc */
            const unsigned char* name = u + cf[FNAME] + p;
            const unsigned char* s[2] = { u + p + cf[SZOF], u + p + cf[NLOF] };
            const long strSizes[2] = { cf[SZSIZE], cf[NLSIZE] };

            /* Convert file and name size from string */
            v[NAME_SZ] = 0; v[FILE_SZ] = 0; /* integer sizes of name and file*/
            for(j=0; j<2; j++){ /* get it from strings, stored in s */
                long k = strSizes[j]-1; while( s[j][k] == ' ' ) k--;
                for(i=k, mul=1; i>=0; mul *= cf[MUL], i--){ /* str to int */
              if((s[j][i]>='1') && (s[j][i]<='9')) v[j]+=(s[j][i]-'0') * mul;
              if((s[j][i]>='a') && (s[j][i]<='f')) v[j]+=(s[j][i]-'a'+10)*mul;
              if((s[j][i]>='A') && (s[j][i]<='F')) v[j]+=(s[j][i]-'A'+10)*mul;
            }};

            newOffset = 0; j = cf[DIVTO]; /* Calculating offset size */
            if( fmt <= DX_BIN_CPIO ){ /* Binary CPIO's specific stuff */
                long b = 1 - cf[BIT], s = !b, k = p + cf[SZOF];
                v[NAME_SZ] = (u[p+cf[NLOF] + b] << 8) | (u[p+cf[NLOF] + s]);
                if( (v[NAME_SZ] % 2) != 0 ) v[NAME_SZ]++;
                v[FILE_SZ] =(u[k+b]<<24)|(u[k+s]<<16)|(u[k+2+b]<<8)|(u[k+2+s]);
            } else if( fmt == DX_NEWC_CPIO ){ /* New ascii CPIO = newc */
                i = (cf[BSIZE] + v[NAME_SZ]) % 4;
                if( i > 0 ) v[NAME_SZ] += 4 - i;
            };
            if( (j>1) && (v[FILE_SZ]%j)>0 ) newOffset += j - v[FILE_SZ]%j;
            newOffset += v[FILE_SZ] + v[NAME_SZ] + cf[BSIZE];

            i = 0; /* strcmp */
            while( (fileName[i]!=0) && (name[i]==fileName[i]) ) i++;
            if( (i > 0) && ((name[i]==0) || (name[i]=='/')) &&
                (fileName[i] == 0) ) found = 1;
      } else return 0; /* Format is not known */
    } while( !found && (p + newOffset + inf[fmt][BSIZE] <= archiveSize) );
    if( found ){
        *fileSize = v[FILE_SZ];
        return (const char*)u + p + inf[fmt][BSIZE] + v[NAME_SZ];
    } else return 0; /* No file found in archive - return NULL */
}


#if defined(DXARCHIVEREAD_EXAMPLE)
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv){
    long fsize, mysize, bytes_read;
    FILE* f;
    char* data;
    const char* myfile;

    if(argc < 2){
        printf(
            "dxArchiveRead.c usage example. Public domain.\n"
            "Usage: dxArchiveReadExample archiveName inArchiveFile\n"
        );
        return 0;
    }

    f = fopen(argv[1], "rb");
    fseek(f, 0, SEEK_END);
    fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    data = (char*) malloc(fsize + 1);
    bytes_read = fread(data, 1, fsize, f);
    fclose(f);
    if( bytes_read != fsize ){
        printf("File was not read correctly!\n");
        free(data);
        return -1;
    }
    data[fsize] = 0;

    myfile = dxArchiveRead(data, fsize, argv[2], &mysize);
    if( myfile ){
        printf("File content:\n");
        fwrite(myfile, mysize, 1, stdout);
        printf("\nFile size: %ld\n", mysize);
    } else printf("No file '%s' in archive. Or incorrect archive\n", argv[2]);
    free(data); /* free data after you finished using archive reader */
    return 0;
}
#endif /* DXARCHIVEREAD_EXAMPLE */
