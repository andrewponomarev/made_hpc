#include <stdio.h>
#include <stdlib.h>
#include <string.h>

__global__ void pattern_blur_kernel(
    float *pattern, int pattern_size,
    unsigned char *in_img, int height, int width,
    unsigned char *out_img)
{
    int row_ind = threadIdx.x + blockIdx.x * blockDim.x;
    int col_ind = threadIdx.y + blockIdx.y * blockDim.y;

    if (row_ind >= height || col_ind >= width)
    {
        return;
    }

    int radius = (pattern_size - 1) / 2;

    for (int ch = 0; ch < 3; ++ch)
    {
        float new_value = 0.0;

        for (int di = -radius; di <= radius; ++di)
        {
            for (int dj = -radius; dj <= radius; ++dj)
            {
                int rind = row_ind + di;
                int cind = col_ind + dj;

                if (rind < 0) rind = 0;
                if (cind < 0) cind = 0;

                if (rind >= height) rind = height - 1;
                if (cind >= width) cind = width - 1;

                new_value += (
                    pattern[(di + radius) * pattern_size + (dj + radius)] *
                    in_img[(rind * width + cind) * 3 + ch]
                );
            }
        }

        if (new_value < 0) new_value = 0;
        if (new_value > 255) new_value = 255;

        out_img[(row_ind * width + col_ind) * 3 + ch] = (
            (unsigned char) new_value
        );
    }
}

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        printf(
            "Usage: %s input_pattern.txt input_img.bmp output.bmp\n", argv[0]);

        return 1;
    }

    FILE *input_pattern = fopen(argv[1], "r");

    int pattern_size;
    fscanf(input_pattern, "%d", &pattern_size);

    int pattern_sizeof = pattern_size * pattern_size * sizeof(float);
    float *h_pattern = (float *) malloc(pattern_sizeof);
    float pattern_sum = 0.0;

    for (int i = 0; i < pattern_size; ++i)
    {
        for (int j = 0; j < pattern_size; ++j)
        {
            float value;
            fscanf(input_pattern, "%f", &value);

            h_pattern[i * pattern_size + j] = value;
            pattern_sum += value;
        }
    }

    if (pattern_sum)
    {
        for (int i = 0; i < pattern_size; ++i)
        {
            for (int j = 0; j < pattern_size; ++j)
            {
                h_pattern[i * pattern_size + j] /= pattern_sum;
            }
        }
    }

    fclose(input_pattern);

    FILE *input_img = fopen(argv[2], "rb");

    int width, height;
    unsigned short int bpp;
    unsigned char header[138];

    fseek(input_img, 18, 0);
    fread(&width, sizeof(int), 1, input_img);

    fseek(input_img, 22, 0);
    fread(&height, sizeof(int), 1, input_img);

    fseek(input_img, 28, 0);
    fread(&bpp, sizeof(unsigned char), 1, input_img);

    fseek(input_img, 0, 0);
    fread(&header, sizeof(unsigned char), 138, input_img);

    int img_sizeof = height * width * 3 * sizeof(unsigned char);

    unsigned char *h_in_img = (unsigned char *) malloc(img_sizeof);

    unsigned int padding_size = (int)((width * bpp + 31) / 32) * 4 - width * 3;
    unsigned char *h_padding = (unsigned char *) malloc(padding_size);

    for (int i = 0; i < height; ++i)
    {
        for (int j = 0; j < width; ++j)
        {
            unsigned char b, g, r;

            fread(&b, sizeof(unsigned char), 1, input_img);
            fread(&g, sizeof(unsigned char), 1, input_img);
            fread(&r, sizeof(unsigned char), 1, input_img);

            h_in_img[(i * width + j) * 3] = r;
            h_in_img[(i * width + j) * 3 + 1] = g;
            h_in_img[(i * width + j) * 3 + 2] = b;
        }

        if (padding_size)
        {
            fread(&h_padding, padding_size, 1, input_img);
        }
    }

    fclose(input_img);

    unsigned char *h_out_img = (unsigned char *) malloc(img_sizeof);

    float *d_pattern;
    unsigned char *d_in_img;
    unsigned char *d_out_img;

    cudaSetDevice(0);

    cudaMalloc((void **) &d_pattern, pattern_sizeof);
    cudaMalloc((void **) &d_in_img, img_sizeof);
    cudaMalloc((void **) &d_out_img, img_sizeof);

    cudaMemcpy(d_pattern, h_pattern, pattern_sizeof, cudaMemcpyHostToDevice);
    cudaMemcpy(d_in_img, h_in_img, img_sizeof, cudaMemcpyHostToDevice);

    dim3 gridSize((int)(height / 16) + 1, int(width / 16) + 1);
    dim3 blockSize(16, 16);

    pattern_blur_kernel<<< gridSize, blockSize >>>(
        d_pattern, pattern_size,
        d_in_img, height, width,
        d_out_img);

    cudaDeviceSynchronize();

    cudaMemcpy(h_out_img, d_out_img, img_sizeof, cudaMemcpyDeviceToHost);

    FILE *output_img = fopen(argv[3], "wb");

    fwrite(header, sizeof(unsigned char), 138, output_img);

    for (int i = 0; i < height; ++i)
    {
        for (int j = 0; j < width; ++j)
        {
            unsigned char r = h_out_img[(i * width + j) * 3];
            unsigned char g = h_out_img[(i * width + j) * 3 + 1];
            unsigned char b = h_out_img[(i * width + j) * 3 + 2];

            fwrite(&b, sizeof(unsigned char), 1, output_img);
            fwrite(&g, sizeof(unsigned char), 1, output_img);
            fwrite(&r, sizeof(unsigned char), 1, output_img);
        }

        if (padding_size)
        {
            fwrite(&h_padding, padding_size, 1, output_img);
        }
    }

    fflush(output_img);
    fclose(output_img);

    free(h_pattern);
    free(h_padding);
    free(h_in_img);
    free(h_out_img);

    cudaFree(d_pattern);
    cudaFree(d_in_img);
    cudaFree(d_out_img);

    return 0;
}
