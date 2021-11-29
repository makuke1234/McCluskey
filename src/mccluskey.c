#include "mccluskey.h"

#include <string.h>
#include <stdlib.h>
#include <math.h>

const McCluskeyState_t McCluskeyStateChars[] = {
	['\0'] = McCluskeyState_terminator,
	['0']  = McCluskeyState_zero,
	['1']  = McCluskeyState_one,
	['-']  = McCluskeyState_undefined
};
const char McCluskeyStateCharsRev[MCCLUSKEYSTATE_NUM] = {
	'\0',
	'0',
	'1',
	'-'
};

static McCluskeyMode_t s_McCluskeyMode = McCluskeyMode_one;

McCluskeyState_t McCluskey_getRawState(char stateCh)
{
	if (stateCh > MCCLUSKEYSTATE_LAST_CHAR)
	{
		return 0;
	}
	else
	{
		return McCluskeyStateChars[(uint8_t)stateCh];
	}
}
char McCluskey_getRawChar(McCluskeyState_t state)
{
	if (state > MCCLUSKEYSTATE_NUM)
	{
		return -1;
	}
	else
	{
		return McCluskeyStateCharsRev[state];
	}
}

McCluskeyState_t McCluskey_getState(char stateCh)
{
	if (stateCh > MCCLUSKEYSTATE_LAST_CHAR)
	{
		return 0;
	}
	else if (s_McCluskeyMode == McCluskeyMode_zero)
	{
		McCluskeyState_t state = McCluskeyStateChars[(uint8_t)stateCh];
		if (state == McCluskeyState_zero)
		{
			return McCluskeyState_one;
		}
		else if (state == McCluskeyState_one)
		{
			return McCluskeyState_zero;
		}
		else
		{
			return state;
		}
	}
	else
	{
		return McCluskeyStateChars[(uint8_t)stateCh];
	}
}
char McCluskey_getChar(McCluskeyState_t state)
{
	if (state > MCCLUSKEYSTATE_NUM)
	{
		return -1;
	}
	else if (s_McCluskeyMode == McCluskeyMode_zero)
	{
		if (state == McCluskeyState_zero)
		{
			return McCluskeyStateCharsRev[McCluskeyState_one];
		}
		else if (state == McCluskeyState_one)
		{
			return McCluskeyStateCharsRev[McCluskeyState_zero];
		}
		else
		{
			return McCluskeyStateCharsRev[state];
		}
	}
	else
	{
		return McCluskeyStateCharsRev[state];
	}
}

void McCluskeyVal_setMode(char modeCh)
{
	s_McCluskeyMode = (McCluskey_getRawState(modeCh) == McCluskeyState_zero);
}

bool McCluskeyVal_make(McCluskeyVal_t * restrict This, const char * argvec, char value)
{
	memset(This, 0, sizeof(McCluskeyVal_t));
	// Check for abnormalities in argvec
	size_t len = 0;
	for (size_t i = 0; argvec[i] != '\0' && i < MCCLUSKEYVAL_MAX_ARGVEC; ++i)
	{
		++len;
		if (McCluskey_getRawState(argvec[i]) == 0)
		{
			return false;
		}
	}
	// Too long!
	if (argvec[len] != '\0')
	{
		return false;
	}

	// Check for abnormalities in value
	This->state = (uint8_t)(McCluskey_getState(value) & 0b11);
	if (This->state == 0 || This->state == McCluskeyState_zero)
	{
		return false;
	}

	for (uint8_t i = 0; i < len; ++i)
	{
		// Store the value backwards
		McCluskeyVal_putArgBit(This, i, McCluskey_getRawState(argvec[len-i-1]));
	}
	return true;
}
McCluskeyState_t McCluskeyVal_getArgBit(const McCluskeyVal_t * restrict This, uint8_t idx)
{
	return (McCluskeyState_t)((This->argVec >> (idx * 2)) & 0b11);
}
void McCluskeyVal_putArgBit(McCluskeyVal_t * restrict This, uint8_t idx, McCluskeyState_t value)
{
	value &= 0b11;
	const uint64_t zeroMask = ((uint64_t)0b11)  << (idx * 2),
	               valMask  = ((uint64_t)value) << (idx * 2);
	McCluskeyState_t prevState = McCluskeyVal_getArgBit(This, idx);
	if (value == McCluskeyState_one && prevState != McCluskeyState_one)
	{
		++This->numOnes;
	}
	else if (value != McCluskeyState_one && prevState == McCluskeyState_one)
	{
		--This->numOnes;
	}
	This->argVec = (This->argVec & ~zeroMask) | valMask;	// OR or XOR can be used in the second part
}

bool McCluskeyVal_addParent(McCluskeyVal_t * restrict This, const McCluskeyVal_t * restrict parent)
{
	if (This->n_fromVec >= This->cap_fromVec)
	{
		size_t newcap = (This->n_fromVec + 1) * 2;
		const McCluskeyVal_t ** newMem = realloc(This->fromVec, sizeof(const McCluskeyVal_t *) * newcap);

		if (newMem == NULL)
		{
			return false;
		}

		This->fromVec     = newMem;
		This->cap_fromVec = newcap;
	}

	This->fromVec[This->n_fromVec] = parent;
	++This->n_fromVec;

	return true;
}
bool McCluskeyVal_shrinkParents(McCluskeyVal_t * restrict This)
{
	if (This->cap_fromVec == This->n_fromVec)
	{
		return true;
	}
	const McCluskeyVal_t ** newMem = realloc(
		This->fromVec,
		sizeof(const McCluskeyVal_t *) * This->n_fromVec
	);

	if (newMem == NULL)
	{
		return false;
	}

	This->fromVec     = newMem;
	This->cap_fromVec = This->n_fromVec;

	return true;
}
void McCluskeyVal_destroy(McCluskeyVal_t * restrict This)
{
	if (This->cap_fromVec > 0)
	{
		free(This->fromVec);
		This->fromVec     = NULL;
		This->cap_fromVec = 0;
		This->n_fromVec   = 0;
	}
}
bool McCluskeyVal_canMerge(const McCluskeyVal_t * restrict lhs, const McCluskeyVal_t * restrict rhs)
{
	if (((lhs->state == McCluskeyState_one)  & (rhs->state == McCluskeyState_zero)) ||
		((lhs->state == McCluskeyState_zero) & (rhs->state == McCluskeyState_one))
	)
	{
		return false;
	}

	uint8_t dCounter = 0;
	for (uint8_t i = 0; i < MCCLUSKEYVAL_MAX_ARGVEC; ++i)
	{
		McCluskeyState_t val1 = McCluskeyVal_getArgBit(lhs, i),
		                 val2 = McCluskeyVal_getArgBit(rhs, i);
		if (val1 == McCluskeyState_terminator && val2 == McCluskeyState_terminator)
		{
			break;
		}
		else if (val1 == McCluskeyState_terminator)
		{
			val1 = McCluskeyState_zero;
		}
		else if (val2 == McCluskeyState_terminator)
		{
			val2 = McCluskeyState_zero;
		}

		// Check for differences
		if (val1 != val2)
		{
			++dCounter;
		}
	}
	return !(dCounter > 1);
}
bool McCluskeyVal_merge(
	McCluskeyVal_t * restrict This,
	const McCluskeyVal_t * restrict vec1,
	const McCluskeyVal_t * restrict vec2
)
{
	if (McCluskeyVal_canMerge(vec1, vec2) == false)
	{
		return false;
	}

	memset(This, 0, sizeof(McCluskeyVal_t));

	// Merging process
	uint8_t len = McCluskeyVal_getMaxLen(vec1, vec2);
	for (uint8_t i = 0; i < len; ++i)
	{
		McCluskeyState_t val1 = McCluskeyVal_getArgBit(vec1, i),
		                 val2 = McCluskeyVal_getArgBit(vec2, i);
	
		if (val1 == McCluskeyState_terminator)
		{
			val1 = McCluskeyState_zero;
		}
		else if (val2 == McCluskeyState_terminator)
		{
			val2 = McCluskeyState_zero;
		}

		// Check for differences
		if (val1 != val2)
		{
			McCluskeyVal_putArgBit(This, i, McCluskeyState_undefined);
		}
		else
		{
			McCluskeyVal_putArgBit(This, i, val1);
		}
	}

	if (vec1->state == McCluskeyState_undefined &&
		vec2->state == McCluskeyState_undefined
	)
	{
		This->state = McCluskeyState_undefined;
	}
	else
	{
		This->state = McCluskeyState_one;
	}

	if (vec1->n_fromVec == 0)
	{
		if (vec1->state != McCluskeyState_undefined)
		{
			McCluskeyVal_addParent(This, vec1);
		}
	}
	else
	{
		for (size_t i = 0; i < vec1->n_fromVec; ++i)
		{
			if (vec1->fromVec[i]->state != McCluskeyState_undefined)
			{
				McCluskeyVal_addParent(This, vec1->fromVec[i]);
			}
		}
	}
	if (vec2->n_fromVec == 0)
	{
		if (vec2->state != McCluskeyState_undefined)
		{
			McCluskeyVal_addParent(This, vec2);
		}
	}
	else
	{
		for (size_t i = 0; i < vec2->n_fromVec; ++i)
		{
			if (vec2->fromVec[i]->state != McCluskeyState_undefined)
			{
				McCluskeyVal_addParent(This, vec2->fromVec[i]);
			}
		}
	}
	
	McCluskeyVal_shrinkParents(This);

	return true;
}
uint8_t McCluskeyVal_getLen(const McCluskeyVal_t * restrict This)
{
	uint8_t len = 0;
	for (uint8_t i = 0; i < MCCLUSKEYVAL_MAX_ARGVEC; ++i)
	{
		if (McCluskeyVal_getArgBit(This, i) == McCluskeyState_terminator)
		{
			break;
		}
		else
		{
			len = i;
		}
	}

	return len + 1;
}
uint8_t McCluskeyVal_getMinLen(
	const McCluskeyVal_t * restrict vec1,
	const McCluskeyVal_t * restrict vec2
)
{
	uint8_t len = 0;
	for (uint8_t i = 0; i < MCCLUSKEYVAL_MAX_ARGVEC; ++i)
	{
		McCluskeyState_t val1 = McCluskeyVal_getArgBit(vec1, i),
		                 val2 = McCluskeyVal_getArgBit(vec2, i);
		if ((val1 == McCluskeyState_terminator) & (val2 == McCluskeyState_terminator))
		{
			break;
		}
		else if ((val1 > McCluskeyState_zero) | (val2 > McCluskeyState_zero))
		{
			len = i;
		}
	}

	return len + 1;
}
uint8_t McCluskeyVal_getMaxLen(
	const McCluskeyVal_t * restrict vec1,
	const McCluskeyVal_t * restrict vec2
)
{
	uint8_t len = 0;
	for (uint8_t i = 0; i < MCCLUSKEYVAL_MAX_ARGVEC; ++i)
	{
		McCluskeyState_t val1 = McCluskeyVal_getArgBit(vec1, i),
		                 val2 = McCluskeyVal_getArgBit(vec2, i);
		if ((val1 == McCluskeyState_terminator) & (val2 == McCluskeyState_terminator))
		{
			break;
		}
		else
		{
			len = i;
		}
	}

	return len + 1;
}

///
///	@param a first num
///	@param b second num
///	@returns The minimum of a and b of type "size_t"
///
static inline size_t minSz_t(size_t a, size_t b)
{
	return (a < b) ? a : b;
}
bool McCluskeyVal_getVecStr(const McCluskeyVal_t * restrict This, char * restrict str, size_t maxStr)
{
	uint8_t len = McCluskeyVal_getLen(This), max = (uint8_t)minSz_t(maxStr - 1, (size_t)len);
	if (len > max)
	{
		return false;
	}


	for (uint8_t i = 0; i < max; ++i)
	{
		str[i] = McCluskey_getRawChar(McCluskeyVal_getArgBit(This, (uint8_t)(max-i-(uint8_t)1)));
		if (str[i] == '\0')
		{
			break;
		}
	}
	str[max] = '\0';

	return true;
}
char McCluskeyVal_getStateCh(const McCluskeyVal_t * restrict This)
{
	return McCluskey_getChar(This->state);
}
bool McCluskeyVal_equal(const McCluskeyVal_t * restrict vec1, const McCluskeyVal_t * restrict vec2)
{
	uint8_t len = McCluskeyVal_getMinLen(vec1, vec2);
	uint64_t eqMask = (~((uint64_t)0)) >> (2 * (MCCLUSKEYVAL_MAX_ARGVEC - len));
	return (vec1->argVec & eqMask) == (vec2->argVec & eqMask);
}



void McCluskeyVals_make(McCluskeyVals_t * restrict This)
{
	memset(This, 0, sizeof(McCluskeyVals_t));
}
bool McCluskeyVals_push(McCluskeyVals_t * restrict This, const McCluskeyVal_t * restrict val)
{
	if (This->n_vals >= This->cap_vals)
	{
		size_t newcap = (This->n_vals + 1) * 2;
		McCluskeyVal_t ** newmem = realloc(This->vals, sizeof(McCluskeyVal_t *) * newcap);

		if (newmem == NULL)
		{
			return false;
		}

		This->vals     = newmem;
		This->cap_vals = newcap;
	}


	This->vals[This->n_vals] = malloc(sizeof(McCluskeyVal_t));
	if (This->vals[This->n_vals] == NULL)
	{
		return false;
	}
	*This->vals[This->n_vals] = *val;
	++This->n_vals;

	return true;
}
bool McCluskeyVals_pop(McCluskeyVals_t * restrict This)
{
	if (This->n_vals == 0)
	{
		return false;
	}

	--This->n_vals;
	McCluskeyVal_destroy(This->vals[This->n_vals]);
	free(This->vals[This->n_vals]);

	return true;
}
McCluskeyVal_t * McCluskeyVals_getLast(McCluskeyVals_t * restrict This)
{
	if (This->n_vals == 0)
	{
		return NULL;
	}
	else
	{
		return This->vals[This->n_vals - 1];
	}
}
bool McCluskeyVals_shrink(McCluskeyVals_t * restrict This)
{
	if (This->cap_vals == This->n_vals)
	{
		return true;
	}

	McCluskeyVal_t ** newmem = realloc(This->vals, sizeof(McCluskeyVal_t *) * This->n_vals);

	if (newmem == NULL)
	{
		return false;
	}

	This->vals     = newmem;
	This->cap_vals = This->n_vals;

	return true;
}
void McCluskeyVals_destroy(McCluskeyVals_t * restrict This)
{
	if (This->cap_vals > 0)
	{
		for (size_t i = 0; i < This->n_vals; ++i)
		{
			McCluskeyVal_destroy(This->vals[i]);
			free(This->vals[i]);
		}
		free(This->vals);
		This->vals     = NULL;
		This->cap_vals = 0;
		This->n_vals   = 0;
	}
}
void McCluskeyVals_destroyPreserveParents(McCluskeyVals_t * restrict This)
{
	if (This->cap_vals > 0)
	{
		for (size_t i = 0; i < This->n_vals; ++i)
		{
			free(This->vals[i]);
		}
		free(This->vals);
		This->vals     = NULL;
		This->cap_vals = 0;
		This->n_vals   = 0;
	}
}
void McCluskeyVals_removeDuplicates(McCluskeyVals_t * restrict This)
{
	if (This->n_vals < 2)
	{
		return;
	}

	for (size_t i = 1; i < This->n_vals;)
	{
		bool found = false;
		for (size_t j = i; j > 0; --j)
		{
			if (This->vals[j - 1]->numOnes != This->vals[i]->numOnes)
			{
				break;
			}
			else if (McCluskeyVal_equal(This->vals[j - 1], This->vals[i]))
			{
				found = true;
				break;
			}
		}
		if (found)
		{
			McCluskeyVal_destroy(This->vals[i]);
			free(This->vals[i]);
			memmove(&This->vals[i], &This->vals[i+1], sizeof(McCluskeyVal_t *) * (This->n_vals - i - 1));
			--This->n_vals;
		}
		else
		{
			++i;
		}
	}
}

///
///	@brief Compare function for sorting McCluskeyVal_t objects by the number of ones
///	in the input argument vectors
///
static inline int McCluskeyVals_comp(const void * a, const void * b)
{
	return (int)(*((const McCluskeyVal_t **)a))->numOnes - (int)(*((const McCluskeyVal_t **)b))->numOnes;
}
void McCluskeyVals_sort(McCluskeyVals_t * restrict This)
{
	if (This->n_vals < 2)
	{
		return;
	}

	qsort(This->vals, This->n_vals, sizeof(McCluskeyVal_t *), &McCluskeyVals_comp);
}


bool McCluskeySolver_make(McCluskeySolver_t * restrict This, McCluskeyVals_t * restrict source)
{
	memset(This, 0, sizeof(McCluskeySolver_t));
	if (McCluskeySolver_pushLayer(This) == false)
	{
		return false;
	}

	This->set[0] = *source;
	memset(source, 0, sizeof(McCluskeyVals_t));
	
	return true;
}
bool McCluskeySolver_pushLayer(McCluskeySolver_t * restrict This)
{
	if (This->n_set >= This->cap_set)
	{
		size_t newcap = (This->n_set + 1) * 2;
		McCluskeyVals_t * newmem = realloc(This->set, sizeof(McCluskeyVals_t) * newcap);
		
		if (newmem == NULL)
		{
			return false;
		}

		This->set     = newmem;
		This->cap_set = newcap;
	}

	McCluskeyVals_make(&This->set[This->n_set]);
	++This->n_set;

	return true;
}
bool McCluskeySolver_popLayer(McCluskeySolver_t * restrict This)
{
	if (This->n_set == 0)
	{
		return false;
	}

	--This->n_set;
	McCluskeyVals_destroy(&This->set[This->n_set]);

	return true;
}
void McCluskeySolver_destroy(McCluskeySolver_t * restrict This)
{
	if (This->cap_set > 0)
	{
		for (size_t i = 0; i < This->n_set; ++i)
		{
			McCluskeyVals_destroy(&This->set[i]);
		}
		free(This->set);
		This->set     = NULL;
		This->cap_set = 0;
		This->n_set   = 0;
	}
}

bool McCluskeySolver_solveLayer(McCluskeySolver_t * restrict This)
{
	if (McCluskeySolver_getLayer(This) == NULL || McCluskeySolver_pushLayer(This) == false)
	{
		return false;
	}
	McCluskeyVals_t * layer = McCluskeySolver_getLayer(This);
	const McCluskeyVals_t * prevLayer = layer - 1;

	// Create boundaries
	size_t boundaries[MCCLUSKEYVAL_MAX_ARGVEC] = { 0 };
	uint8_t prevOnes = 0;
	for (size_t i = 0; i < prevLayer->n_vals; ++i)
	{
		if (prevLayer->vals[i]->numOnes != prevOnes)
		{
			prevOnes = prevLayer->vals[i]->numOnes;
			boundaries[prevOnes] = i;
		}
	}

	bool * hasConnected = calloc(prevLayer->n_vals, sizeof(bool));
	if (hasConnected == NULL)
	{
		// Remove layer
		McCluskeySolver_popLayer(This);
		return false;
	}

	for (uint8_t i = 0; i < prevOnes; ++i)
	{
		size_t b1 = boundaries[i], b2 = boundaries[i+1];
		if ((prevLayer->vals[b1]->numOnes + 1) !=  prevLayer->vals[b2]->numOnes)
		{
			continue;
		}
		for (size_t j = b1; j < b2; ++j)
		{
			uint8_t baseOnes = prevLayer->vals[b2]->numOnes;
			for (size_t k = b2; k < prevLayer->n_vals; ++k)
			{
				if (prevLayer->vals[k]->numOnes != baseOnes)
				{
					break;
				}
				// Start fiddling

				McCluskeyVal_t newVec;
				if (McCluskeyVal_merge(&newVec, prevLayer->vals[j], prevLayer->vals[k]) == true)
				{
					if (McCluskeyVals_push(layer, &newVec) == false)
					{
						// Adding member failed
						free(hasConnected);
						// Remove layer
						McCluskeySolver_popLayer(This);

						return false;
					}
					hasConnected[j] = true;
					hasConnected[k] = true;
				}
			}
		}
	}

	bool ret = false;
	// Sort out all elements that have been left out unconnected
	// (by using array hasConnected)
	for (size_t i = 0; i < prevLayer->n_vals; ++i)
	{
		if (hasConnected[i] == false)
		{
			const McCluskeyVal_t * restrict current = prevLayer->vals[i];
			McCluskeyVal_t * member;
			if (McCluskeyVals_push(layer, current) == false ||
				(	(member = McCluskeyVals_getLast(layer))->n_fromVec == 0 &&
					member->state != McCluskeyState_undefined &&
					McCluskeyVal_addParent(member, current) == false
				)
			)
			{
				free(hasConnected);
				// Remove layer
				McCluskeySolver_popLayer(This);
				return false;
			}
		}
		else
		{
			ret = true;
		}
	}

	free(hasConnected);

	// Sort new data
	McCluskeyVals_sort(layer);

	// Remove duplicates
	McCluskeyVals_removeDuplicates(layer);

	McCluskeyVals_shrink(layer);

	return ret;
}

///
///	@brief Change interval parent count
///	@param baseItem Pointer to interval object
///	@param hashSet Pointer to hash-set
///	@param deltaCount Amount by which the interval parent count would be changed
///
static inline void optimiseLayer_changeCount_impl(
	const McCluskeyVal_t * restrict baseItem,
	MCHashSet_t * restrict hashSet,
	intptr_t deltaCount
)
{
	for (size_t i = 0; i < baseItem->n_fromVec; ++i)
	{
		MCHashSetNode_t * node = MCHashSet_get(hashSet, baseItem->fromVec[i]->argVec);
		if (deltaCount > 0)
		{
			*((uint32_t *)MCHashSetNode_getData(node)) += (uint32_t)deltaCount;
		}
		else
		{
			*((uint32_t *)MCHashSetNode_getData(node)) -= (uint32_t)(-deltaCount);
		}
	}
}
///
///	@brief Checks if set has "full house"
///	@param arr Array to hash-set's interval nodes
/// @param numElems Number of nodes
///	@returns Whether set is complete
///
static inline bool optimiseLayer_checkSet_impl(MCHashSetNode_t * const restrict * const restrict arr, size_t numElems)
{
	for (size_t i = 0; i < numElems; ++i)
	{
		if (*((const uint32_t *)MCHashSetNode_getConstData(arr[i])) == 0)
		{
			return false;
		}
	}
	return true;
}
///
///	@brief Recurse through all possibilities till the best variant is found
///
void optimiseLayer_recursiveVariations_impl(
	size_t startDepth,
	size_t startVal,
	size_t * restrict maxDepth,
	MCHashSet_t * restrict hashSet,
	MCHashSetNode_t * const restrict * const restrict arr,
	size_t * restrict indexArr,
	size_t numElems,
	McCluskeySolver_t * restrict This,
	const McCluskeyVals_t * restrict baseLayer,
	size_t * restrict newLayers
)
{
	if (startDepth >= *maxDepth)
	{
		return;
	}
	for (size_t i = startVal; i < baseLayer->n_vals; ++i)
	{
		// add something in respect to arr[startDepth] and i

		indexArr[startDepth] = i;
		optimiseLayer_changeCount_impl(
			baseLayer->vals[i],
			hashSet,
			1
		);

		// check whole array
		if (optimiseLayer_checkSet_impl(arr, numElems) == true)
		{
			if (*maxDepth != (startDepth + 1))
			{
				// Clear all previous layers
				while (*newLayers > 0)
				{
					McCluskeyVals_destroyPreserveParents(McCluskeySolver_getLayer(This));
					McCluskeySolver_popLayer(This);
					--*newLayers;
				}

				*maxDepth = startDepth + 1;
			}

			// Add new "awesome" layer of size startDepth + 1
			if (McCluskeySolver_pushLayer(This) == false)
			{
				exit(1);
			}
			McCluskeyVals_t * layer = McCluskeySolver_getLayer(This);

			for (size_t j = 0, sz = startDepth + 1; j < sz; ++j)
			{
				// element is baseLayer->vals[indexArr[j]]

				if (McCluskeyVals_push(layer, baseLayer->vals[indexArr[j]]) == false)
				{
					exit(1);
				}
			}

			McCluskeyVals_shrink(layer);

			++*newLayers;
		}
		optimiseLayer_recursiveVariations_impl(
			startDepth + 1,
			i + 1,
			maxDepth,
			hashSet,
			arr,
			indexArr,
			numElems,
			This,
			baseLayer,
			newLayers
		);

		optimiseLayer_changeCount_impl(
			baseLayer->vals[i],
			hashSet,
			-1
		);
	}
}
bool McCluskeySolver_optimiseLayer(McCluskeySolver_t * restrict This, size_t * restrict newLayers)
{
	*newLayers = 0;
	McCluskeyVals_t baseLayer;
	McCluskeyVals_make(&baseLayer);

	const McCluskeyVals_t * prevLayer = McCluskeySolver_getLayer(This);
	if (prevLayer == NULL)
	{
		return false;
	}

	for (size_t i = 0; i < prevLayer->n_vals; ++i)
	{
		if (prevLayer->vals[i]->state == McCluskeyState_one)
		{
			if (McCluskeyVals_push(&baseLayer, prevLayer->vals[i]) == false)
			{
				McCluskeyVals_destroyPreserveParents(&baseLayer);
				return false;
			}
		}
	}

	// All totally unnecessary items eliminated, already sorted array
	if (baseLayer.n_vals < 2)
	{
		if (McCluskeySolver_pushLayer(This) == false)
		{
			return false;
		}
		else
		{
			(*McCluskeySolver_getLayer(This)) = baseLayer;
			*newLayers = 1;
			return true;
		}
	}

	MCHashSet_t countSet;
	MCHashSet_make(&countSet, baseLayer.n_vals);

	for (size_t i = 0; i < baseLayer.n_vals; ++i)
	{
		const McCluskeyVal_t * restrict val = baseLayer.vals[i];
		for (size_t j = 0; j < val->n_fromVec; ++j)
		{
			if (MCHashSet_push(&countSet, val->fromVec[j]->argVec) == NULL)
			{
				MCHashSet_destroy(&countSet);
				McCluskeyVals_destroyPreserveParents(&baseLayer);
				return false;
			}
		}
	}

	size_t n_countSetElems = MCHashSet_getCount(&countSet);
	MCHashSetNode_t ** countSetElems = malloc(sizeof(MCHashSetNode_t *) * n_countSetElems);
	size_t * indexArr = malloc(sizeof(size_t) * n_countSetElems);

	if (countSetElems == NULL || indexArr == NULL)
	{
		if (countSetElems != NULL)
		{
			free(countSetElems);
		}
		else if (indexArr != NULL)
		{
			free(indexArr);
		}
		MCHashSet_destroy(&countSet);
		McCluskeyVals_destroyPreserveParents(&baseLayer);
		return false;
	}

	// Fill in the array with unique elements from hash-set
	MCHashSet_getArr(&countSet, countSetElems, n_countSetElems);

	/**** Start combining different intervals ****/


	// Combining business, at first with 1 element, then 2, etc until reaches first
	// "full house"
	// then return all "full houses" with that size

	size_t maxDepth = n_countSetElems;
	optimiseLayer_recursiveVariations_impl(
		0,
		0,
		&maxDepth,
		&countSet,
		countSetElems,
		indexArr,
		n_countSetElems,
		This,
		&baseLayer,
		newLayers
	);

	McCluskeyVals_destroyPreserveParents(&baseLayer);

	// Free hash-set elems LUT
	free(countSetElems);
	free(indexArr);

	// Destroy hash-set
	MCHashSet_destroy(&countSet);

	return true;
}
McCluskeyVals_t * McCluskeySolver_getLayer(McCluskeySolver_t * restrict This)
{
	if (This->n_set == 0)
	{
		return NULL;
	}
	else
	{
		return &This->set[This->n_set - 1];
	}
}


struct MCHashSetNode
{
	uint64_t key;
	// no data
	MCHashSetNode_t * next;

	uint32_t data;
};

void * MCHashSetNode_getData(MCHashSetNode_t * restrict This)
{
	return &This->data;
}
const void * MCHashSetNode_getConstData(const MCHashSetNode_t * restrict This)
{
	return &This->data;
}


///
///	@brief Destroys hash-set node recursively
///	@param node Pointer to node
///
static inline void MCHashSetNode_destroy(MCHashSetNode_t * restrict node)
{
	if (node == NULL)
	{
		return;
	}
	MCHashSetNode_destroy(node->next);
	free(node);
}

bool MCHashSet_make(MCHashSet_t * restrict This, size_t size)
{
	memset(This, 0, sizeof(MCHashSet_t));
	// Find prime number hash-set size
	This->n_nodes = MCHashSet_findNextPrime(size);

	This->nodes = calloc(This->n_nodes, sizeof(MCHashSetNode_t *));
	if (This->nodes == NULL)
	{
		This->n_nodes = 0;
		return false;
	}

	return true;
}
bool MCHashSet_isPrime(size_t num)
{
	if (num == 2)
	{
		return true;
	}
	else if ((num % 2) == 0)
	{
		return false;
	}

	size_t root = (size_t)sqrtf((float)num) + 1;
	for (size_t i = 3; i < root; i += 2)
	{
		if ((num % i) == 0)
		{
			return false;
		}
	}

	return true;
}
size_t MCHashSet_findNextPrime(size_t lowerBound)
{
	if ((lowerBound % 2) == 0)
	{
		++lowerBound;
	}

	while (MCHashSet_isPrime(lowerBound) == false)
	{
		lowerBound += 2;
	}

	return lowerBound;
}
///
///	@param This Pointer to hash-set
///	@param key 64-bit key to index the member in hash-set
///	@returns Hash to represent slot index of given key
///
static inline size_t MCHashSet_hash(const MCHashSet_t * restrict This, uint64_t key)
{
	return (size_t)(key % (uint64_t)This->n_nodes);
}
bool MCHashSet_resize(MCHashSet_t * restrict This, size_t size)
{
	size = MCHashSet_findNextPrime(size);
	// Make new hash-set
	MCHashSet_t newset;
	if (MCHashSet_make(&newset, size) == false)
	{
		return false;
	}

	// Iterate through all elements of current hash-set
	for (size_t i = 0; i < This->n_nodes; ++i)
	{
		MCHashSetNode_t * node = This->nodes[i];
		while (node != NULL)
		{
			if (MCHashSet_push(&newset, node->key) == false)
			{
				MCHashSet_destroy(&newset);
				return false;
			}
			node = node->next;
		}
	}

	MCHashSet_destroy(This);
	*This = newset;
	return true;
}
MCHashSetNode_t * MCHashSet_push(MCHashSet_t * restrict This, uint64_t key)
{
	if (This->n_elems > This->n_nodes)
	{
		MCHashSet_resize(This, This->n_elems * 2 + 1);
	}
	size_t index = MCHashSet_hash(This, key);
	
	// Search if node already exists
	MCHashSetNode_t * prevnode = NULL, * node = This->nodes[index];
	while (node != NULL)
	{
		// Found previously pushed node
		if (node->key == key)
		{
			return node;
		}
		prevnode = node;
		node = node->next;
	}
	// Create new node
	MCHashSetNode_t * newnode = calloc(1, sizeof(MCHashSetNode_t));
	if (newnode == NULL)
	{
		return NULL;
	}

	newnode->key = key;

	if (prevnode == NULL)
	{
		This->nodes[index] = newnode;
	}
	else
	{
		prevnode->next = newnode;
	}

	++This->n_elems;
	return newnode;
}
bool MCHashSet_pop(MCHashSet_t * restrict This, uint64_t key)
{
	if (This->n_elems == 0)
	{
		return false;
	}
	
	size_t index = MCHashSet_hash(This, key);
	MCHashSetNode_t * prevnode = NULL, * node = This->nodes[index];
	while (node != NULL)
	{
		if (node->key == key)
		{
			if (prevnode == NULL)
			{
				This->nodes[index] = node->next;
			}
			else
			{
				prevnode->next = node->next;
			}
			free(node);

			--This->n_elems;
			return true;
		}

		prevnode = node;
		node = node->next;
	}

	return false;
}
bool MCHashSet_exists(const MCHashSet_t * restrict This, uint64_t key)
{
	if (This->n_elems == 0)
	{
		return false;
	}
	
	size_t index = MCHashSet_hash(This, key);
	MCHashSetNode_t * node = This->nodes[index];
	while (node != NULL)
	{
		if (node->key == key)
		{
			return true;
		}

		node = node->next;
	}

	return false;
}
MCHashSetNode_t * MCHashSet_get(MCHashSet_t * restrict This, uint64_t key)
{
	if (This->n_elems == 0)
	{
		return NULL;
	}
	
	size_t index = MCHashSet_hash(This, key);
	MCHashSetNode_t * node = This->nodes[index];
	while (node != NULL)
	{
		if (node->key == key)
		{
			return node;
		}

		node = node->next;
	}

	return NULL;
}
size_t MCHashSet_getCount(const MCHashSet_t * restrict This)
{
	return This->n_elems;
}
bool MCHashSet_getArr(MCHashSet_t * restrict This, MCHashSetNode_t ** array, size_t arrMax)
{
	if (This->n_elems > arrMax)
	{
		return false;
	}

	size_t arrIdx = 0;
	for (size_t i = 0; i < This->n_nodes; ++i)
	{
		MCHashSetNode_t * node = This->nodes[i];
		while (node != NULL)
		{
			array[arrIdx] = node;
			++arrIdx;

			node = node->next;
		}
	}
	
	return true;
}

void MCHashSet_destroy(MCHashSet_t * restrict This)
{
	if (This->n_nodes > 0)
	{
		for (size_t i = 0; i < This->n_nodes; ++i)
		{
			if (This->nodes[i] != NULL)
			{
				MCHashSetNode_destroy(This->nodes[i]);
			}
		}
		free(This->nodes);
		
		This->nodes   = NULL;
		This->n_nodes = 0;
		This->n_elems = 0;
	}
}
