#include "mccluskey.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_RIDA 256

void showTruthTable(const McCluskeyVals_t * layer);

int main()
{
	McCluskeyVals_t sourcemap;
	McCluskeyVals_make(&sourcemap);

	printf("Sisesta t6ev22rtustabel kujul [argumentvektor] [v22rtus]:\n");
	

	uint8_t firstIter = 0;

	do
	{
		if (firstIter < 2)
		{
			++firstIter;
		}

		char rida[MAX_RIDA];
		// loe rida
		fgets(rida, MAX_RIDA, stdin);
		{
			char * end = strrchr(rida, '\n');
			if (end != NULL)
			{
				*end = '\0';
			}
		}

		if (strlen(rida) == 0 || feof(stdin) != 0)
		{
			break;
		}

		char * argvec    = strtok(rida, " "),
		     * valueStr  = strtok(NULL, " ");

		McCluskeyVal_t value;

		if (argvec == NULL || valueStr == NULL || strlen(valueStr) != 1)
		{
			printf("Sisesta korrektne rida!\n");
			continue;
		}
		else if (strncmp(argvec, "mode", 4) == 0)
		{
			if (firstIter == 1)
			{
				McCluskeyVal_setMode(valueStr[0]);
			}
			else
			{
				printf("Sisesta korrektne rida!\n");
			}
			continue;
		}
		else if (McCluskeyVal_make(&value, argvec, valueStr[0]) == false)
		{
			printf("Sisesta korrektne rida!\n");
			continue;
		}

		if (McCluskeyVals_push(&sourcemap, &value) == false)
		{
			printf("Ootamatu viga! Ei saa rohkem andmeid sisestada! V2ljun...\n");
			exit(1);
		}
	} while (1);

	printf("L2hteandmed sisse loetud!\n");

	McCluskeyVals_sort(&sourcemap);

	printf("L2hteandmed on sorteeritud!\n");

	McCluskeySolver_t solver;

	if (McCluskeySolver_make(&solver, &sourcemap) == false)
	{
		printf("Ei suutnud lahendaja objekti teha! V2ljun...\n");
		exit(1);
	}

	// Lahendamine

	while (McCluskeySolver_solveLayer(&solver) == true);

	McCluskeyVals_t * layer = McCluskeySolver_getLayer(&solver);

	if (layer == NULL)
	{
		printf("Illegaalsed andmed!\n");
		exit(1);
	}

	printf("Esialgne \"lahendatud\" intervallide tabel:\n");
	showTruthTable(layer);

	size_t newLayers;
	if (McCluskeySolver_optimiseLayer(&solver, &newLayers) == false)
	{
		printf("Tabeli optimeerimine ei 6nnestunud!\n");
		exit(1);
	}

	printf("Tabel optimeeritud!\n");

	printf("L6plik(ud) optimeeritud intervallide tabel(id):\n");
	McCluskeyVals_t * optimalLayer = McCluskeySolver_getLayer(&solver) - (newLayers - 1);
	for (size_t i = 0; i < newLayers; ++i)
	{
		printf("Tabel #%zu:\n", i + 1);
		showTruthTable(optimalLayer);
		++optimalLayer;
	}

	McCluskeySolver_destroy(&solver);

	printf("M2lu puhastatud!\n");

	return 0;
}

void showTruthTable(const McCluskeyVals_t * layer)
{
	for (size_t i = 0; i < layer->n_vals; ++i)
	{
		char arg[MCCLUSKEYVAL_MAX_ARGVEC + 1], value;
		McCluskeyVal_getVecStr(layer->vals[i], arg, MCCLUSKEYVAL_MAX_ARGVEC + 1);
		value = McCluskeyVal_getStateCh(layer->vals[i]);
		
		printf("%*s | %c (", MCCLUSKEYVAL_MAX_ARGVEC, arg, value);
		const McCluskeyVal_t * val = layer->vals[i];
		for (size_t j = 0; j < val->n_fromVec; ++j)
		{
			if (val->fromVec[j]->state != McCluskeyState_one)
			{
				putchar('*');
			}
			else
			{
				McCluskeyVal_getVecStr(val->fromVec[j], arg, MCCLUSKEYVAL_MAX_ARGVEC + 1);
				printf("%s", arg);
			}
			if (j < val->n_fromVec - 1)
			{
				putchar(',');
			}
		}
		if (val->n_fromVec == 0)
		{
			putchar('*');
		}
		printf(")\n");
	}
}
