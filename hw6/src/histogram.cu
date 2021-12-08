#include <stdio.h>
#include <stdlib.h>
#include <string.h>

__global__ void histogram_kernel(
    unsigned char *img, int height, int width,
    unsigned long long *histogram)
{
    const int histogram_size = 255 * 3;
    __shared__ unsigned long long shared_histogram[histogram_size];

    if (threadIdx.x < 16 && threadIdx.y < 16)
    {
        int thread_index = threadIdx.x * 16 + threadIdx.y;

        shared_histogram[thread_index * 3] = 0;
        shared_histogram[thread_index * 3 + 1] = 0;
        shared_histogram[thread_index * 3 + 2] = 0;
    }

    __syncthreads();

    int row_ind = threadIdx.x + blockIdx.x * blockDim.x;
    int col_ind = threadIdx.y + blockIdx.y * blockDim.y;

    if (row_ind < height && col_ind < width)
    {
        for (int ch = 0; ch < 3; ++ch)
        {
            unsigned int value = img[(row_ind * width + col_ind) * 3 + ch];

            atomicAdd(&shared_histogram[value * 3 + ch], 1);
        }
    }

    __syncthreads();

    if (threadIdx.x < 16 && threadIdx.y < 16)
    {
        int thread_index = threadIdx.x * 16 + threadIdx.y;

        atomicAdd(
            &histogram[thread_index * 3],
            shared_histogram[thread_index * 3]);

        atomicAdd(
            &histogram[thread_index * 3 + 1],
            shared_histogram[thread_index * 3 + 1]);

        atomicAdd(
            &histogram[thread_index * 3 + 2],
            shared_histogram[thread_index * 3 + 2]);
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf(
            "Usage: %s input_img.bmp\n", argv[0]);

        return 1;
    }

    FILE *input_img = fopen(argv[1], "rb");

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

    unsigned char *h_img = (unsigned char *) malloc(img_sizeof);

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

            h_img[(i * width + j) * 3] = r;
            h_img[(i * width + j) * 3 + 1] = g;
            h_img[(i * width + j) * 3 + 2] = b;
        }

        if (padding_size)
        {
            fread(&h_padding, padding_size, 1, input_img);
        }
    }

    fclose(input_img);

    int histogram_sizeof = 256 * 3 * sizeof(unsigned long long);

    unsigned long long *h_histogram = (
        (unsigned long long *) malloc(histogram_sizeof));

    unsigned char *d_img;
    unsigned long long *d_histogram;

    cudaSetDevice(0);

    cudaMalloc((void **) &d_img, img_sizeof);
    cudaMalloc((void **) &d_histogram, histogram_sizeof);

    cudaMemcpy(
        d_img, h_img, img_sizeof, cudaMemcpyHostToDevice);

    dim3 gridSize((int)(height / 16) + 1, int(width / 16) + 1);
    dim3 blockSize(16, 16);

    histogram_kernel<<< gridSize, blockSize >>>(
        d_img, height, width,
        d_histogram);

    cudaDeviceSynchronize();

    cudaMemcpy(
        h_histogram, d_histogram, histogram_sizeof, cudaMemcpyDeviceToHost);

    for (int ch = 0; ch < 3; ++ch)
    {
        for (int i = 0; i < 256; ++i)
        {
            printf("%llu ", h_histogram[i * 3 + ch]);
        }

        printf("\n");
    }

    free(h_padding);
    free(h_histogram);
    free(h_img);

    cudaFree(d_img);
    cudaFree(d_histogram);

    return 0;
}
