#include "return_codes.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef ZLIB
	#include <zlib.h>
#elif LIBDEFLATE
	#error "This library is not supported"
#elif ISAL
	#error "This library is not supported""
#else
	#error "No libraries selected"
#endif

struct chunk_structure
{
	uint8_t *len;
	uint8_t *type_of_chunk;
	uint8_t *width;
	uint8_t *height;
	uint8_t *depth;
	uint8_t *color_type;
	uint8_t *compression;
	uint8_t *filter;
	uint8_t *Interlace;
	uint8_t *data;
	uint8_t *for_crc;
};

int calc(const uint8_t *num)
{
	int len = 0;
	for (int i = 0; i < 4; i++)
	{
		len += num[i] * pow(16, ((3 - i) * 2));
	}
	return len;
}

struct chunk_structure *read_file_and_add_IHDR(FILE *in)
{
	struct chunk_structure *chunk_png = malloc(sizeof(struct chunk_structure));
	uint8_t checker;

	chunk_png->len = malloc(sizeof(uint8_t) * 4);
	checker = fread(chunk_png->len, 1, 4, in);
	if (checker != 4)
	{
		return NULL;
	}
	chunk_png->type_of_chunk = malloc(sizeof(uint8_t) * 4);
	checker = fread(chunk_png->type_of_chunk, 1, 4, in);
	if (checker != 4)
	{
		return NULL;
	}
	chunk_png->width = malloc(sizeof(uint8_t) * 4);
	checker = fread(chunk_png->width, 1, 4, in);
	if (checker != 4)
	{
		return NULL;
	}
	chunk_png->height = malloc(sizeof(uint8_t) * 4);
	checker = fread(chunk_png->height, 1, 4, in);
	if (checker != 4)
	{
		return NULL;
	}
	chunk_png->depth = malloc(sizeof(uint8_t) * 1);
	checker = fread(chunk_png->depth, 1, 1, in);
	if (checker != 1)
	{
		return NULL;
	}
	chunk_png->color_type = malloc(sizeof(uint8_t) * 1);
	checker = fread(chunk_png->color_type, 1, 1, in);
	if (checker != 1)
	{
		return NULL;
	}
	chunk_png->compression = malloc(sizeof(uint8_t) * 1);
	checker = fread(chunk_png->compression, 1, 1, in);
	if (checker != 1)
	{
		return NULL;
	}
	chunk_png->filter = malloc(sizeof(uint8_t) * 1);
	checker = fread(chunk_png->filter, 1, 1, in);
	if (checker != 1)
	{
		return NULL;
	}
	chunk_png->Interlace = malloc(sizeof(uint8_t) * 1);
	checker = fread(chunk_png->Interlace, 1, 1, in);
	if (checker != 1)
	{
		return NULL;
	}
	chunk_png->for_crc = malloc(sizeof(uint8_t) * 4);
	checker = fread(chunk_png->for_crc, 1, 4, in);
	if (checker != 4)
	{
		return NULL;
	}

	return chunk_png;
}

struct chunk_structure *read_file_and_add(int ind, const uint8_t *file)
{
	struct chunk_structure *chunk_png = malloc(sizeof(struct chunk_structure));
	chunk_png->len = malloc(sizeof(uint8_t) * 4);
	chunk_png->for_crc = malloc(sizeof(uint8_t) * 4);
	chunk_png->type_of_chunk = malloc(sizeof(uint8_t) * 4);
	int q = 4;
	for (int i = 0; i < 4; i++)
	{
		chunk_png->len[i] = file[ind - q];
		q--;
	}

	for (int i = 0; i < 4; i++)
	{
		chunk_png->type_of_chunk[i] = file[ind];
		ind++;
	}

	int length = calc(chunk_png->len);
	chunk_png->data = malloc(sizeof(uint8_t) * length);

	for (int i = 0; i < length; i++)
	{
		chunk_png->data[i] = file[ind];
		ind++;
	}

	for (int i = 0; i < 4; i++)
	{
		chunk_png->for_crc[i] = file[ind];
		ind++;
	}
	return chunk_png;
}

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		fprintf(stderr, "Wrong number of string arguments");
		return ERROR_INVALID_PARAMETER;
	}
	FILE *in = fopen(argv[1], "rb");
	FILE *out = fopen(argv[2], "wb");
	if (out == NULL)
	{
		fclose(in);
		fprintf(stderr, "Not enough memory to create pnm file");
		return ERROR_NOT_ENOUGH_MEMORY;
	}
	uint8_t sign[8];
	fread(sign, 1, 8, in);
	if (sign[0] != 0x89 || sign[1] != 0x50 || sign[2] != 0x4e || sign[3] != 0x47 || sign[4] != 0x0d ||
		sign[5] != 0x0a || sign[6] != 0x1a || sign[7] != 0x0a)
	{
		fclose(in);
		fclose(out);
		fprintf(stderr, "This file is not png type");
		return ERROR_INVALID_PARAMETER;
	}

	struct chunk_structure *png_file = malloc(sizeof(struct chunk_structure) * 3);
	if (png_file == NULL)
	{
		fprintf(stderr, "Not enough memory");
		fclose(in);
		fclose(out);
		return ERROR_MEMORY;
	}

	png_file[0] = *read_file_and_add_IHDR(in);
	if (png_file[0].len == NULL || png_file[0].type_of_chunk == NULL || png_file[0].width == NULL || png_file[0].height == NULL ||
		png_file[0].depth == NULL || png_file[0].color_type == NULL || png_file[0].compression == NULL ||
		png_file[0].filter == NULL || png_file[0].Interlace == NULL || png_file[0].for_crc == NULL)
	{
		fprintf(stderr, "Not enough memory");
		free(png_file);
		fclose(in);
		fclose(out);
		return ERROR_MEMORY;
	}
	if (png_file[0].filter[0] != 0 || png_file[0].compression[0] != 0 || png_file[0].Interlace[0] != 0)
	{
		fprintf(stderr, "incorrect data");
		free(png_file);
		fclose(in);
		fclose(out);
		return ERROR_INVALID_DATA;
	}

	if (png_file[0].color_type[0] != 0)
	{
		if (png_file[0].color_type[0] != 2)
		{
			fprintf(stderr, "incorrect data");
			free(png_file);
			fclose(in);
			fclose(out);
			return ERROR_INVALID_DATA;
		}
	}

	fpos_t ft;
	fgetpos(in, &ft);
	long long size_file;
	fseek(in, 0, SEEK_END);
	size_file = ftell(in);
	fseek(in, ft, SEEK_SET);
	fpos_t ff;
	fgetpos(in, &ff);
	uint8_t *file = malloc(sizeof(uint8_t) * size_file);

	if (file == NULL)
	{
		fprintf(stderr, "Not enough memory");
		free(png_file);
		fclose(in);
		fclose(out);
		return ERROR_MEMORY;
	}

	fread(file, 1, size_file - ft, in);
	fclose(in);

	int flag = 0;
	int ind = 0;
	int index = 1;
	int count = 0;
	while (flag != 1)
	{
		if (file[ind] == 0x49 && file[ind + 1] == 0x44 && file[ind + 2] == 0x41 && file[ind + 3] == 0x54)
		{
			png_file[index] = *read_file_and_add(ind, file);
			if (png_file[index].len == NULL || png_file[index].type_of_chunk == NULL || png_file[index].data == NULL ||
				png_file[index].for_crc == NULL)
			{
				fprintf(stderr, "Not enough memory");
				free(file);
				free(png_file);
				fclose(out);
				return ERROR_MEMORY;
			}
			ind += calc(png_file[index].len);
			index++;
		}
		if (file[ind] == 0x49 && file[ind + 1] == 0x45 && file[ind + 2] == 0x4e && file[ind + 3] == 0x44)
		{
			png_file[index] = *read_file_and_add(ind, file);
			if (png_file[index].len == NULL || png_file[index].type_of_chunk == NULL || png_file[index].data == NULL ||
				png_file[index].for_crc == NULL)
			{
				fprintf(stderr, "Not enough memory");
				free(file);
				free(png_file);
				fclose(out);
				return ERROR_MEMORY;
			}
			flag = 1;
			index++;
		}
		else
		{
			count++;
			ind++;
		}
		if (count == size_file)
		{
			break;
		}
	}

	int count_IDAT = index - 2;
	uint8_t *data_of_IDAT;
	int count_length = 0;
	int index_for_data = 0;
	if (count_IDAT >= 1)
	{
		for (int i = 1; i < count_IDAT + 1; i++)
		{
			if (index != i)
			{
				count_length += calc(png_file[i].len);
			}
		}
		data_of_IDAT = malloc(sizeof(uint8_t) * count_length);
		if (data_of_IDAT == NULL)
		{
			fprintf(stderr, "Not enough memory");
			free(file);
			free(png_file);
			fclose(out);
			return ERROR_MEMORY;
		}
		for (int i = 1; i < count_IDAT + 1; i++)
		{
			if (index != i)
			{
				for (int j = 0; j < calc(png_file[i].len); j++)
				{
					data_of_IDAT[index_for_data] = png_file[i].data[j];
					index_for_data++;
				}
			}
		}

		unsigned long v = calc(png_file[0].height) * (calc(png_file[0].width) * png_file[0].depth[0] + 1);
		uint8_t *data_uncompress = malloc(sizeof(uint8_t) * v);

		if (data_uncompress == NULL)
		{
			fprintf(stderr, "Not enough memory");
			free(file);
			free(data_of_IDAT);
			free(png_file);
			fclose(out);
			return ERROR_MEMORY;
		}

		uncompress(data_uncompress, &v, data_of_IDAT, count_length);

		uint8_t *result =
			malloc(sizeof(uint8_t) * calc(png_file[0].height) * (calc(png_file[0].width) * png_file[0].depth[0] + 1));

		if (result == NULL)
		{
			fprintf(stderr, "Not enough memory");
			free(file);
			free(data_uncompress);
			free(data_of_IDAT);
			free(png_file);
			fclose(out);
			return ERROR_MEMORY;
		}

		int num;
		int byt_pix = calc(png_file[0].width) * png_file[0].depth[0];
		int index2 = 0;
		int q = 0;
		for (int i = 0; i < calc(png_file[0].height); i++)
		{
			int helper = data_uncompress[q];
			q += 1;
			for (int j = 0; j < byt_pix; j++)
			{
				int helper2 = data_uncompress[q];
				q += 1;
				if (helper == 0)
				{
					num = helper2;
				}
				else if (helper == 1)
				{
					if (j >= png_file[0].depth[0])
					{
						num = data_uncompress[i * byt_pix + j - png_file[0].depth[0]] + helper2;
					}
					else
					{
						num = helper2;
					}
				}
				else if (helper == 2)
				{
					if (i > 0)
					{
						num = data_uncompress[(i - 1) * byt_pix + j] + helper2;
					}
					else
					{
						num = helper2;
					}
				}
				else if (helper == 3)
				{
					if (i > 0 && j >= png_file[0].depth[0])
					{
						num = ((data_uncompress[(i - 1) * byt_pix + j] + data_uncompress[i * byt_pix + j - png_file[0].depth[0]]) + helper2) / 2;
					}
					else if (i > 0)
					{
						num = data_uncompress[(i - 1) * byt_pix + j] / 2 + helper2;
					}
					else if (j >= png_file[0].depth[0])
					{
						num = (data_uncompress[i * byt_pix + j - png_file[0].depth[0]]) / 2 + helper2;
					}
					else
					{
						num = helper2;
					}
				}
				else if (helper == 4)
				{
					int a = 0;
					int b = 0;
					int c = 0;
					if (j >= png_file[0].depth[0])
					{
						a = data_uncompress[i * byt_pix + j - png_file[0].depth[0]];
					}
					if (i > 0)
					{
						b = data_uncompress[(i - 1) * byt_pix + j];
					}
					if (j >= png_file[0].depth[0] && i > 0)
					{
						c = (data_uncompress[(i - 1) * byt_pix + j] + data_uncompress[i * byt_pix + j - png_file[0].depth[0]]) / 2;
					}
					int w = a + b - c;
					int wa = abs(w - a);
					int wb = abs(w - b);
					int wc = abs(w - c);
					if (wa <= wb && wa <= wc)
					{
						num = a + helper2;
					}
					else if (wb <= wc)
					{
						num = b + helper2;
					}
					else
					{
						num = c + helper2;
					}
				}
				else
				{
					fprintf(stderr, "this type is not supported");
					free(file);
					free(result);
					free(data_uncompress);
					free(data_of_IDAT);
					free(png_file);
					fclose(out);
					return ERROR_INVALID_DATA;
				}
				result[index2] = (num & 255);
				index2++;
			}
		}
		int t = (png_file[0].color_type[0] == 0) ? 5 : 6;
		fprintf(out, "P%d\n%d %d\n%d", t, calc(png_file[0].width), calc(png_file[0].height), 255);
		fwrite(result, 1, calc(png_file[0].height) * (calc(png_file[0].width) * png_file[0].depth[0] + 1), out);
		free(result);
		free(data_uncompress);
		free(data_of_IDAT);
	}
	else
	{
		fprintf(stderr, "input file is corrupted");
		free(file);
		free(png_file);
		fclose(out);
		return ERROR_INVALID_DATA;
	}
	free(file);
	free(png_file);
	fclose(out);
	return ERROR_SUCCESS;
}