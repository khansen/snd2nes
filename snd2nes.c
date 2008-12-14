#include <math.h>
#include <sndfile.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/**
  Prints \a size bytes of data defined by \a buf to \a out.
*/
static void print_chunk(FILE *out, const char *label,
                        const unsigned char *buf, int size, int cols)
{
    int i, j, m;
    int pos = 0;
    if (label)
        fprintf(out, "%s:\n", label);
    for (i = 0; i < size / cols; ++i) {
        fprintf(out, ".db ");
        for (j = 0; j < cols-1; ++j)
            fprintf(out, "$%.2X,", buf[pos++]);
        fprintf(out, "$%.2X\n", buf[pos++]);
    }
    m = size % cols;
    if (m > 0) {
        fprintf(out, ".db ");
        for (j = 0; j < m-1; ++j)
            fprintf(out, "$%.2X,", buf[pos++]);
        fprintf(out, "$%.2X\n", buf[pos++]);
    }
}

static void convert_to_dmc(SF_INFO *info, short *frames,
                           float samplerate, unsigned char **data_out, int *size_out)
{
    unsigned char bit;
    unsigned char encoded;
    short prev, curr;
    float pos, step;
    int bitcount;
    unsigned char *buf;
    int buf_size;
    step = samplerate / info->samplerate;
    encoded = 0;
    bit = 0;
    prev = 0;
    bitcount = 0;
    buf_size = 1024;
    buf = (unsigned char *)malloc(buf_size);
    for (pos = 0 ; pos < info->frames; prev = curr, pos += step, ++bitcount) {
        if (!(bitcount & 7) && bitcount) {
            if ((bitcount / 8) >= buf_size) {
                buf_size += 1024;
                buf = (unsigned char *)realloc(buf, buf_size);
            }
            buf[(bitcount-1) / 8] = encoded;
            encoded = 0;
        }
        curr = frames[(int)pos];
        if (curr < prev)
            bit = 0;
        else if (curr > prev)
            bit = 1;
        else
            bit = bit ^ 1;
        encoded |= bit << (bitcount & 7);
    }
    /* pad to 64-byte boundary */
    while (bitcount & 511) {
        if (!(bitcount & 7)) {
            if ((bitcount / 8) >= buf_size) {
                buf_size += 1024;
                buf = (unsigned char *)realloc(buf, buf_size);
            }
            buf[(bitcount-1) / 8] = encoded;
            encoded = 0;
        }
        bit = 0; /*bit ^ 1;*/
        encoded |= bit << (bitcount & 7);
        ++bitcount;
    }
    if ((bitcount / 8) >= buf_size) {
        buf_size += 1024;
        buf = (unsigned char *)realloc(buf, buf_size);
    }
    buf[(bitcount-1) / 8] = encoded;
    *data_out = buf;
    *size_out = bitcount / 8;
}

static float note_hz(int note)
{
    return pow(2, (note - 57) / 12.0) * 440;
}

static void print_sample_table(const char *table_label, const char *sample_label_prefix,
                               int *sizes, FILE *out)
{
    static const unsigned char freqs[] = {
        0,0,
        1,1,
        2,2,
        3,3,
        4,4,
        5,5,
        6,
        7,7,
        8,8,8,
        9,9,
        10,10,
        11,11,11,
        12,12,12,12,
        13,13,13,
        14,14,14,14,14,
        15,15,15,15,15
    };
    static const unsigned char samples[] = {
        4,3,
        4,3,
        4,3,
        4,3,
        4,3,
        4,3,
        4,
        4,3,
        4,3,2,
        4,3,
        4,3,
        4,3,2,
        4,3,2,1,
        4,3,2,
        4,3,2,1,0,
        4,3,2,1,0
    };
    int i;
    if (table_label)
        fprintf(out, "%s:\n", table_label);
    for (i = 0; i < 42; ++i) {
        unsigned char f = freqs[i];
        unsigned char s = samples[i];
        fprintf(out, ".db $%.2X,$%.2X,(%s%d-$C000)/64,(%s%d-%s%d)/16\n",
                f, 0, sample_label_prefix, s, sample_label_prefix,
                s+1, sample_label_prefix, s);
    }
}

void snd2nes(const char *input_filename, FILE *out)
{
    short *frames;
    SNDFILE *sf;
    SF_INFO info;
    sf_count_t count;
    int i;
    unsigned char *bufs[5];
    int sizes[5];

    info.format = 0;
    sf = sf_open(input_filename, SFM_READ, &info);
    assert(info.channels == 1);

    frames = (short *)malloc(info.frames * sizeof(short));
    count = sf_readf_short(sf, frames, info.frames);
    assert(count == info.frames);

    for (i = 0; i < 5; ++i) {
        float c8_hz = note_hz(12*8);
        float hz = note_hz(12*8 - i);
	convert_to_dmc(&info, frames, (hz / c8_hz) * info.samplerate, &bufs[i], &sizes[i]);
    }
    free(frames);
    sf_close(sf);

    fprintf(out, ".public dmc_sample_table\n");

    for (i = 0; i < 5; ++i) {
        char label[32];
        sprintf(label, "sample%d", i);
        print_chunk(out, label, bufs[i], sizes[i], 16);
    }
    fprintf(out, "sample%d:\n", 5);

    print_sample_table("dmc_sample_table", "sample", sizes, out);

    for (i = 0; i < 5; ++i)
        free(bufs[i]);
}
