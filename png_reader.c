#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Convert 4 bytes in big-endian order to host uint32_t */
uint32_t read_u32_be(const uint8_t *p)
{
    return ((uint32_t)p[0] << 24) |
           ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8)  |
            (uint32_t)p[3];
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <file.png>\n", argv[0]);
        return (1);
    }

    FILE *fp = fopen(argv[1], "rb");
    if (!fp)
    {
        perror("fopen");
        return (1);
    }

    /* 1. Verify the 8-byte PNG signature */
    uint8_t sig[8];
    if (fread(sig, 1, 8, fp) != 8 || memcmp(sig, (uint8_t[]){0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A}, 8) != 0)
    {
        fprintf(stderr, "Not a valid PNG file.\n");
        fclose(fp);
        return (1);
    }

    /* 2. Chunk iteration – look for IHDR (must be the first chunk) */
    uint32_t length;
    char type[5];
    uint8_t *data = NULL;
    int found_ihdr = 0;

    while (1)
    {
        /* Read chunk length */
        uint8_t len_bytes[4];
        if (fread(len_bytes, 1, 4, fp) != 4)
            break;
        length = read_u32_be(len_bytes);

        /* Read chunk type */
        if (fread(type, 1, 4, fp) != 4)
            break;
        type[4] = '\0';

        if (strcmp(type, "IHDR") == 0)
        {
            /* IHDR data is always 13 bytes */
            if (length != 13)
            {
                fprintf(stderr, "Invalid IHDR chunk length.\n");
                break;
            }
            uint8_t ihdr[13];
            if (fread(ihdr, 1, 13, fp) != 13)
                break;
            fseek(fp, 4, SEEK_CUR); /* skip CRC */

            /* Extract metadata */
            uint32_t width  = read_u32_be(ihdr);
            uint32_t height = read_u32_be(ihdr + 4);
            uint8_t depth   = ihdr[8];
            uint8_t coltype = ihdr[9];
            uint8_t compr   = ihdr[10];
            uint8_t filt    = ihdr[11];
            // uint8_t interlace = ihdr[12];

            printf("Width:            %u\n", width);
            printf("Height:           %u\n", height);
            printf("Bit depth:        %u\n", depth);

            printf("Color type:       %u ", coltype);
            switch (coltype)
            {
                case 0: printf("(Grayscale)\n"); break;
                case 2: printf("(RGB)\n"); break;
                case 3: printf("(Indexed)\n"); break;
                case 4: printf("(Grayscale+Alpha)\n"); break;
                case 6: printf("(RGBA)\n"); break;
                default: printf("(Unknown)\n");
            }

            /* Sanity checks */
            if (compr != 0)
                puts("Warning: non-standard compression method.");
            if (filt  != 0)
                puts("Warning: non-standard filter method.");

            found_ihdr = 1;
            break;  /* IHDR found, done */
        } 
        else
            /* Skip chunk data + CRC */
            fseek(fp, (long)length + 4, SEEK_CUR);
    }

    if (!found_ihdr)
    {
        fprintf(stderr, "IHDR chunk not found.\n");
        fclose(fp);
        return (1);
    }

    fclose(fp);
    return (0);
}
