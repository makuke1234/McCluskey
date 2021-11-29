#ifndef MC_CLUSKEY_H
#define MC_CLUSKEY_H

#include <stdint.h>
#include <stdbool.h>


typedef uint8_t McCluskeyState_t;

///
///	@brief Enumerator to denote McCluskey's bit state
///
enum McCluskeyState
{
	McCluskeyState_terminator,
	McCluskeyState_zero,
	McCluskeyState_one,
	McCluskeyState_undefined,
};

#define MCCLUSKEYSTATE_NUM 4

#define MCCLUSKEYSTATE_LAST_CHAR ((uint8_t)'1')


extern const McCluskeyState_t McCluskeyStateChars[MCCLUSKEYSTATE_LAST_CHAR + 1];
extern const char McCluskeyStateCharsRev[MCCLUSKEYSTATE_NUM];

///
///	@returns Corresponding McCluskeyState to character, 0, if illegal state
///
McCluskeyState_t McCluskey_getRawState(char stateCh);
///
///	@returns Corresponding character to McCluskeyState, -1, if illegal state
///
char McCluskey_getRawChar(McCluskeyState_t state);

///
///	@returns Corresponding McCluskeyState to character, 0, if illegal state
///	adjusts to value mode
///
McCluskeyState_t McCluskey_getState(char stateCh);
///
///	@returns Corresponding character to McCluskeyState, -1, if illegal state
///	adjusts to value mode
///
char McCluskey_getChar(McCluskeyState_t state);

#define MCCLUSKEYVAL_MAX_ARGVEC 32

///
///	@brief Data structure to hold 1 McCluskey's node
///
typedef struct McCluskeyVal
{
	uint64_t argVec;
	uint8_t numOnes:6;
	McCluskeyState_t state:2;

	const struct McCluskeyVal ** fromVec;
	size_t cap_fromVec, n_fromVec;

} McCluskeyVal_t;


typedef uint8_t McCluskeyMode_t;

///
///	@brief Enumerator for McCluskey's display modes
///
enum McCluskeyMode
{
	McCluskeyMode_one,
	McCluskeyMode_zero
};

///
///	@brief Set mode for McCluskey
///	@param modeCh character representing mode, mode defaults to McCluskeyMode_one
///
void McCluskeyVal_setMode(char modeCh);

///
///	@brief Creates a McCluskeyVal_t object, converting characters to internal values
///	@param This Pointer to the current node
///	@param argvec Null-terminated character array of argument vector
///	@param value Character denoting value of said argument vector
///	@returns Whether the operation was 100% successful
///
bool McCluskeyVal_make(McCluskeyVal_t * restrict This, const char * argvec, char value);
///
///	@param This Pointer to the current node
///	@param idx McCluskey's bit index
///	@returns The bit state for chosen bit
///
McCluskeyState_t McCluskeyVal_getArgBit(const McCluskeyVal_t * restrict This, uint8_t idx);
///
///	@brief Sets the bit state for chosen bit. Increments/decrements the number of ones
///	depending on the situation
///	@param This Pointer to the current node
///	@param idx McCluskey's bit index
///
void McCluskeyVal_putArgBit(McCluskeyVal_t * restrict This, uint8_t idx, McCluskeyState_t value);

///
///	@brief Adds a parent node to selected McCluskey's node
///	@param This Pointer to the current node
///	@param parent Pointer to the "parent" node to add
///	@returns Whether the operation was successful
///
bool McCluskeyVal_addParent(McCluskeyVal_t * restrict This, const McCluskeyVal_t * restrict parent);
///
///	@brief Shrinks the memory usage of parent nodes to the minimum
///	@param This Pointer to the current node
///	@returns Whether the operation was successful, data preserve guarantee
///
bool McCluskeyVal_shrinkParents(McCluskeyVal_t * restrict This);
///
///	@brief De-allocates/"destroys" the memory of objects' parent nodes
///	@param This Pointer to the current node
///
void McCluskeyVal_destroy(McCluskeyVal_t * restrict This);
///
///	@param lhs first operand
///	@param rhs second operand
///	@returns Whether 2 given argument vectors can be merged
///
bool McCluskeyVal_canMerge(const McCluskeyVal_t * restrict lhs, const McCluskeyVal_t * restrict rhs);
///
///	@brief Merges 2 argument vectors if possible
///	@param This Pointer to current node
///	@param vec1 first argument vector
///	@param vec2 second argument vector
///	@returns Whether 2 given argument vectors were merged
///
bool McCluskeyVal_merge(
	McCluskeyVal_t * restrict This,
	const McCluskeyVal_t * restrict vec1,
	const McCluskeyVal_t * restrict vec2
);
///
///	@param This Pointer to current node
///	@returns Length of argument vector
///
uint8_t McCluskeyVal_getLen(const McCluskeyVal_t * restrict This);
///
///	@param vec1 first argument vector
///	@param vec2 second argument vector
///	@returns Lengths of supposed equal-tested argument vector
///
uint8_t McCluskeyVal_getMinLen(
	const McCluskeyVal_t * restrict vec1,
	const McCluskeyVal_t * restrict vec2
);
///
///	@param vec1 first argument vector
///	@param vec2 second argument vector
///	@returns Length of supposed merged argument vector
///
uint8_t McCluskeyVal_getMaxLen(
	const McCluskeyVal_t * restrict vec1,
	const McCluskeyVal_t * restrict vec2
);
///
///	@brief Converts internally stored argument vectors to string
///	@param This Pointer to current node
///	@param str character array to receive string
///	@param maxStr maximum size of given character array
///	@returns Whether the given array was big enough for said string
///
bool McCluskeyVal_getVecStr(const McCluskeyVal_t * restrict This, char * restrict str, size_t maxStr);
///
///	@param This Pointer to current node
///	@returns Character representing value state
///
char McCluskeyVal_getStateCh(const McCluskeyVal_t * restrict This);
///
///	@param vec1 Pointer to first operand
///	@param vec2 Pointer to second operand
///	@returns If vec1 and vec2 are equal
///
bool McCluskeyVal_equal(const McCluskeyVal_t * restrict vec1, const McCluskeyVal_t * restrict vec2);



///
///	@brief Data structure to hold an array of McCluskey's nodes
///
typedef struct McCluskeyVals
{
	McCluskeyVal_t ** vals;
	size_t cap_vals, n_vals;

} McCluskeyVals_t;

///
///	@brief Initialises McCluskeyVals_t object
///	@param This Pointer to the object to be initialised
///
void McCluskeyVals_make(McCluskeyVals_t * restrict This);
///
///	@brief Adds/"pushes" a McCluskey's node to end of the array of nodes
///	@param This Pointer to object
///	@returns Whether the operation was successful
///
bool McCluskeyVals_push(McCluskeyVals_t * restrict This, const McCluskeyVal_t * restrict val);
///
///	@brief Removes/"pops" a McCluskey's node from the end of array of nodes
///	@param This Pointer to object
///	@returns Whether the operation was successful
///
bool McCluskeyVals_pop(McCluskeyVals_t * restrict This);
///
///	@param This Pointer to object
///	@returns Last McCluskey's node, NULL, if no objects exist
///
McCluskeyVal_t * McCluskeyVals_getLast(McCluskeyVals_t * restrict This);
///
///	@brief Shrinks memory usage of McCluskey's nodes to a minimum
///	@param This Poiner to object
///	@returns Whether the shrinking was successful, data preserve guarantee
///
bool McCluskeyVals_shrink(McCluskeyVals_t * restrict This);
///
///	@brief De-allocates/"destroys" the memory of McCluskey's array of nodes
///	@param This Pointer to the current node
///
void McCluskeyVals_destroy(McCluskeyVals_t * restrict This);
///
///	@brief De-allocates/"destroys" the memory of McCluskey's array of nodes,
///	while preserving the allocated memory of parent nodes
///	@param This Pointer to the current node
///
void McCluskeyVals_destroyPreserveParents(McCluskeyVals_t * restrict This);
///
///	@brief Removes duplicates from nodes
///	@param This Pointer to the current node
///
void McCluskeyVals_removeDuplicates(McCluskeyVals_t * restrict This);
///
///	@brief Sorts the array of McCluskey's nodes by the number of ones in the
///	input argument vectors
///	@param This Pointer to the current node
///
void McCluskeyVals_sort(McCluskeyVals_t * restrict This);



///
///	@brief Data structure to hold a set of McCluskeyVals_t objects helping to
///	solve the problem
///
typedef struct McCluskeySolver
{
	McCluskeyVals_t * set;
	size_t cap_set, n_set;

} McCluskeySolver_t;

///
///	@brief Creates a solver object from source data
///	@param This Pointer to destination object
///	@param source Pointer to source node
///	@returns Whether the operation was successful
///
bool McCluskeySolver_make(McCluskeySolver_t * restrict This, McCluskeyVals_t * restrict source);
///
///	@brief Creates a new layer for solving the problem
///	@param This Pointer to the object
///	@returns Whether the operation was successful
///
bool McCluskeySolver_pushLayer(McCluskeySolver_t * restrict This);
///
///	@brief Removes a layer
///	@param This Pointer to the object
///	@returns Whether the operation was successful
///
bool McCluskeySolver_popLayer(McCluskeySolver_t * restrict This);
///
///	@brief Destroys the object
///	@param This Pointer to the object
///
void McCluskeySolver_destroy(McCluskeySolver_t * restrict This);

///
///	@brief Solve one layer
///	@param This Pointer to the solver object
///	@returns Whether the operation was successful
///
bool McCluskeySolver_solveLayer(McCluskeySolver_t * restrict This);
///
///	@brief Optimise one layer
///	@param This Pointer to the solver object
///	@param newLayers Pointer to variable that denotes the number of new layers added
///	which are all optimal
///	@returns Whether the operation was successful
///
bool McCluskeySolver_optimiseLayer(McCluskeySolver_t * restrict This, size_t * restrict newLayers);
///
///	@param This Pointer to the solver object
///	@returns Pointer to the current layer, NULL if no layers
///
McCluskeyVals_t * McCluskeySolver_getLayer(McCluskeySolver_t * restrict This);

///
///	@brief A node data structure for MCHashSet_t
///
typedef struct MCHashSetNode MCHashSetNode_t;

///
///	@param This Pointer to hash-set node
///	@returns Pointer to data of hash-set node if applicable
///
void * MCHashSetNode_getData(MCHashSetNode_t * restrict This);
///
///	@param This Pointer to hash-set node
///	@returns Pointer to data of hash-set node if applicable
///
const void * MCHashSetNode_getConstData(const MCHashSetNode_t * restrict This);

///
///	@brief A hash-set to hold McCluskey intervals
///
typedef struct MCHashSet
{
	MCHashSetNode_t ** nodes;
	size_t n_nodes, n_elems;

} MCHashSet_t;

///
///	@brief Creates MCHashSet_t object
///	@param This Pointer to object
///	@param defSize default size of hash-set
///	@returns Whether the operation was successful
///
bool MCHashSet_make(MCHashSet_t * restrict This, size_t defSize);
///
///	@returns Whether the given number is prime
///
bool MCHashSet_isPrime(size_t num);
///
///	@returns Next prime number including lowerBound, if lowerBound is prime already
///
size_t MCHashSet_findNextPrime(size_t lowerBound);
///
///	@brief Resizes hash-set
///	@param This Pointer to hash-set
///	@param size New minimum size
///	@returns Whether resizing was successful
///
bool MCHashSet_resize(MCHashSet_t * restrict This, size_t size);
///
///	@brief Inserts an element to hash-set
///	@param This Pointer to hash-set
///	@param key Insertable key
///	@returns Pointer to inserted object, Pointer to previous object if was inserted
///	already, NULL if insertion wasn't successful
///
MCHashSetNode_t * MCHashSet_push(MCHashSet_t * restrict This, uint64_t key);
///
///	@brief Removes an element from hash-set
///	@param This Pointer to hash-set
///	@param key Removable key
///	@returns Whether the key was found & removed from hash-set
///
bool MCHashSet_pop(MCHashSet_t * restrict This, uint64_t key);
///
///	@param This Pointer to hash-set
///	@param key Key to be searched
///	@returns Whether the key was found in hash-set
///
bool MCHashSet_exists(const MCHashSet_t * restrict This, uint64_t key);
///
///	@brief Searches for node with given key
///	@param This Pointer to hash-set
///	@param key Key to be searched
///	@returns Pointer to found node, NULL if object was not found
///
MCHashSetNode_t * MCHashSet_get(MCHashSet_t * restrict This, uint64_t key);
///
///	@returns Element count in hash-set
///
size_t MCHashSet_getCount(const MCHashSet_t * restrict This);
///
///	@brief Fills given array with hash-set elements
///	@param This Pointer to hash-set
///	@param array Given hash-set node array
///	@param arrMax Maximum array size
///	@returns true if array was big enough to be filled, false if array was too small
///
bool MCHashSet_getArr(MCHashSet_t * restrict This, MCHashSetNode_t ** array, size_t arrMax);
///
///	@brief Destroys hash-set
///	@param This Pointer to hash-set
///
void MCHashSet_destroy(MCHashSet_t * restrict This);

#endif
