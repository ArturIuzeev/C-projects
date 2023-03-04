#include "return_codes.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

float *count_result(int n, float *array_2, float *result)
{
	for (int i = 0; i < n; i++)
	{
		float first_element = 0;
		float second_element = 0;
		for (int j = 0; j < n + 1; ++j)
		{
			if (fabsf(array_2[i * (n + 1) + j]) != 0 && first_element == 0)
			{
				first_element = array_2[i * (n + 1) + j];
			}
			else if (fabsf(array_2[i * (n + 1) + j]) != 0)
			{
				second_element = array_2[i * (n + 1) + j];
			}
		}
		result[i] = second_element / first_element;
	}
	free(array_2);
	return result;
}

float *gauss(int n, float *array_2, float *result, FILE *out)
{
	for (int i = 0; i < n - 1; i++)
	{
		for (int j = i + 1; j < n; j++)
		{
			float coefficient;
			if (fabsf(array_2[i * (n + 1) + i]) < 1e-5)
			{
				coefficient = 0;
			}
			else
			{
				coefficient = array_2[j * (n + 1) + i] / array_2[i * (n + 1) + i];
			}
			for (int k = 0; k < n + 1; k++)
			{
				if (fabsf(array_2[j * (n + 1) + k] - (coefficient * array_2[i * (n + 1) + k])) < 1e-5)
				{
					array_2[j * (n + 1) + k] = 0;
				}
				else
				{
					array_2[j * (n + 1) + k] -= (coefficient * array_2[i * (n + 1) + k]);
				}
			}
		}
	}

	int count_rang_no_extended_matrix = 0;
	int count_rang_extended_matrix = 0;
	for (int i = 0; i < n; i++)
	{
		int count_zero_one = 0;
		int count_zero_two = 0;
		for (int j = 0; j < n + 1; j++)
		{
			if (fabsf(array_2[i * (n + 1) + j]) < 1e-5 && j != n)
			{
				count_zero_one++;
			}
			if (fabsf(array_2[i * (n + 1) + j]) < 1e-5)
			{
				count_zero_two++;
			}
		}
		if (count_zero_one != n)
		{
			count_rang_no_extended_matrix++;
		}
		if (count_zero_two != (n + 1))
		{
			count_rang_extended_matrix++;
		}
	}

	if (count_rang_no_extended_matrix != count_rang_extended_matrix)
	{
		fprintf(out, "no solution");
		free(array_2);
		free(result);
		fclose(out);
		return NULL;
	}

	if (count_rang_no_extended_matrix == count_rang_extended_matrix && count_rang_extended_matrix < n)
	{
		fprintf(out, "many solutions");
		free(array_2);
		free(result);
		fclose(out);
		return NULL;
	}

	for (int i = n - 1; i > -1; i--)
	{
		for (int j = i - 1; j > -1; j--)
		{
			float coefficient;
			if (fabsf(array_2[i * (n + 1) + i]) < 1e-5)
			{
				coefficient = 0;
			}
			else
			{
				coefficient = array_2[j * (n + 1) + i] / array_2[i * (n + 1) + i];
			}
			for (int k = n; k > -1; k--)
			{
				if (fabsf(array_2[j * (n + 1) + k] - (coefficient * array_2[i * (n + 1) + k])) < 1e-5)
				{
					array_2[j * (n + 1) + k] = 0;
				}
				else
				{
					array_2[j * (n + 1) + k] -= (coefficient * array_2[i * (n + 1) + k]);
				}
			}
		}
	}
	result = count_result(n, array_2, result);
	return result;
}

int main(int argc, char **argv)
{
	char *input = argv[1];
	char *output = argv[2];

	if (argc != 3)
	{
		printf("Wrong number of string arguments");
		return ERROR_INVALID_PARAMETER;
	}

	if (argv[1] == NULL || argv[2] == NULL)
	{
		printf("Arguments are invalid");
		return ERROR_INVALID_PARAMETER;
	}

	int n;

	FILE *in = fopen(input, "r");
	if (in == NULL)
	{
		printf("Input file not found");
		return ERROR_FILE_NOT_FOUND;
	}

	FILE *out = fopen(output, "w");

	if (out == NULL)
	{
		fclose(in);
		printf("Failed to allocate memory for output file");
		return ERROR_MEMORY;
	}

	fscanf(in, "%i", &n);

	float *array = malloc(sizeof(float) * n * (n + 1));

	if (array == NULL)
	{
		fclose(in);
		printf("Failed to allocate memory");
		return ERROR_MEMORY;
	}

	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < n + 1; j++)
		{
			fscanf(in, "%f", &array[i * (n + 1) + j]);
		}
	}

	fclose(in);

	float **array_2;
	array_2 = &array;

	float *result = malloc(sizeof(float) * n);

	if (result == NULL)
	{
		printf("Failed to allocate memory");
		return ERROR_MEMORY;
	}

	result = gauss(n, *array_2, result, out);

	if (result == NULL)
	{
		return ERROR_SUCCESS;
	}

	for (int i = 0; i < n; i++)
	{
		fprintf(out, "%g\n", result[i]);
	}

	free(result);
	fclose(out);
	return ERROR_SUCCESS;
}