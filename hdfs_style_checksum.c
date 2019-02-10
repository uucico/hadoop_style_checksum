/* calculates hdfs-style MD5-of-0MD5-CRC32C checksums on a list of files
 * based on https://github.com/srch07/HDFSChecksumForLocalfile
 * uses Mark Adler's crc32c ia32 implementation
 * by r. sugawara (2019)
 * 
 * compiles with gmake & clang. dependencies are openssl and libpthreads
 */

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <openssl/md5.h>
#include <assert.h>

extern uint32_t crc32c(uint32_t crc, const void *buf, size_t len);

int main(int argc, char *argv[]) {
    unsigned char c[MD5_DIGEST_LENGTH*2], d[MD5_DIGEST_LENGTH]; // yes, I know its crazy, but it seems hadoop applies md5 to a 32 char padded buffer
    char mdString[33];
    int bytesPerCrc = 512;
    int i;
    FILE *inFile;
    MD5_CTX mdContext, mdOfmdContext;
    int bytes;
    unsigned char data[1024*1024];

	uint32_t crc;

	if (argc < 2) {
		printf("Usage: hdfs_style_checksum <filename> ...\n");
		return 1;
	}

	for (i=1; i<argc; i++) {
		// tries to open the file
		inFile = fopen(argv[i], "rb");
		if (inFile == NULL) {
			printf("%s: cannot open file.\n", argv[i]);
			continue;
		}

		// gets the file size for calculations...
		fseek(inFile, 0 , SEEK_END);
		long size = ftell(inFile);
		fseek(inFile, 0 , SEEK_SET);

		// calculates length of crc32c buffer and allocates it
		int numOfChecksums = size / bytesPerCrc;
		if (size % bytesPerCrc > 0) numOfChecksums += 1;
		unsigned char *crcBuf = malloc(numOfChecksums * 4); // 4 bytes per crc32c checksum

		int bufWriteOffset = 0;

		for (;;) {
			int bytesRead = 0;
			bytes = fread(data, 1, bytesPerCrc*1024, inFile); // reads a multiple of the bytesPerCRC for a large chunk
			//printf("  fread returned %d bytes.", bytes);

			int bytesRemaining = bytes;
		
			while (bytesRemaining) {
				//printf("  read %d bytes adding to a total of %d to complete.\n", bytesRead, bytesRemaining);

				int toCrc = bytesPerCrc;
				if (bytesRemaining < bytesPerCrc) toCrc = bytesRemaining;

				crc = crc32c(0, &data[bytesRead], toCrc); // calculares over offset for toCrc bytes
                                //printf("  crc for offset %d and len %d is %08x\n", bytesRead, toCrc, crc); printf("\n");

				// inverts endianess and applies to the buffer
				crcBuf[bufWriteOffset++]= (crc >> 24) & 0xff;
				crcBuf[bufWriteOffset++]= (crc >> 16) & 0xff;
				crcBuf[bufWriteOffset++]= (crc >> 8)  & 0xff;
				crcBuf[bufWriteOffset++]=  crc	& 0xff; 

				assert(bufWriteOffset < numOfChecksums*4+1); // do not cross the buffer boundary

				bytesRemaining -= toCrc;
				bytesRead += toCrc;
			}	

			// eof?
			if (bytes == 0) break;
		}

		MD5_Init (&mdOfmdContext);
		MD5_Init (&mdContext);

		/* debug 
		int j;
		printf("  crcBuf: ");
                for (int j = 0; j < bufWriteOffset; j++)
                        printf("%02x", crcBuf[j]);
                printf("\n");
		*/

		// writes zeros to MD5 buffer
		memset(c, 0, sizeof(c));
		MD5_Update(&mdContext, crcBuf, bufWriteOffset);

		MD5_Final(c, &mdContext);

		/* debug 
		printf("  cBuf: ");
                for (int j = 0; j < MD5_DIGEST_LENGTH*2; j++)
                        printf("%02x", c[j]);
                printf("\n");
		*/

		MD5_Update(&mdOfmdContext, c, MD5_DIGEST_LENGTH*2);

		MD5_Final (d,&mdOfmdContext);

		for (int j = 0; j < MD5_DIGEST_LENGTH; j++)
                	sprintf(&mdString[j*2], "%02x", d[j]);
                printf("%s: %s\n", argv[i], mdString);

		// free resources
		fclose(inFile);
		free(crcBuf);
	}

	return 0;
}
