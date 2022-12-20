#include "apriori.h"

#include <search.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_LEVELS 5           // Maximum frequent set size.
#define BUFFER_LEN 50          // Default buffer len
#define HASH_TABLE_SIZE 16384  // Needs to be big enough to contain the hashes of the supports
#define USE_ANTI_MONOTONICITY_SUPPORT
#define USE_ANTI_MONOTONICITY_CONFIDENCE
// #define PRINT_UTILS

/**
 * @brief Struct that describes item sets at a particular level k.
 */
typedef struct LevelSets {
    int **sets;
    int numSets;
    int setSize;
} LevelSets;

/**
 * @brief Prints a message to stderr and exits the program. Works with var args similar to printf.
 *
 * @param format Message to print.
 * @param ... Var args. Use is similar to printf.
 */
static void fatalError(const char *format, ...) {
    fprintf(stderr, "Fatal error: ");
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    exit(EXIT_FAILURE);
}

/**
 * @brief Prints a warning message to stderr.
 *
 * @param format Message to print
 * @param ... Var args. Use is similar to printf.
 */
static void warning(const char *format, ...) {
    fprintf(stderr, "Warning: ");
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
}

/**
 * @brief Utility function to ensure malloc is not called with incorrect values. Verifies that malloc correctly
 * allocates memory.
 *
 * @param size Number of bytes to allocate.
 * @return void* Pointer to the allocated memory.
 */
static void *safeMalloc(int size) {
    void *p = malloc(size);
    if (p == NULL) {
        fatalError("safeMalloc(%d) failed.\n", size);
    }
    return p;
}

inline static void mapPut(char *key, int val) {
    ENTRY item = {key, val};
    hsearch(item, ENTER);
}

inline static int mapGet(char *key) {
    ENTRY item;
    ENTRY *result;
    item.key = key;
    result = hsearch(item, FIND);
    return result == NULL ? -1 : (int)result->data;
}

/**
 * @brief Allocates an integer matrix in such a way that it is continuous in memory and only needs a single free.
 *
 * @param width Width the matrix should have.
 * @param height Height the matrix should have.
 * @return int** Int matrix.
 */
static int **allocIntMatrix(int width, int height) {
    int **matrix = safeMalloc(height * sizeof(int *) + width * height * sizeof(int));
    int *p = (int *)(matrix + height);
    for (int y = 0; y < height; y++) {
        matrix[y] = p + width * y;
    }
    return matrix;
}

/**
 * @brief Converts a set of integers to a string representation. The set {0,2,3} will be converted "023".
 *
 * @param set  The set to convert.
 * @param size Number of elements in the set.
 * @return char* Pointer to the string representation of the set.
 */
static char *setToString(const int *set, int size) {
    char *buf = malloc(BUFFER_LEN * sizeof(char));
    char *tmp = buf;
    for (int i = 0; i < size; i++) {
        tmp += snprintf(tmp, BUFFER_LEN, "%d", set[i]);
    }
    return buf;
}

/**
 * @brief Frees the memory used by a level set.
 *
 * @param LevelSets The level sets to free.
 */
static void freeLevelSet(LevelSets *LevelSets) {
    free(LevelSets->sets);
    free(LevelSets);
}

/**
 * @brief Frees the memory used by the table data.
 *
 * @param data The TableData to free.
 */
static void freeCSV(TableData *data) {
    free(data->headers[0]);
    free(data->headers);
    free(data->data);
    free(data);
}

#ifdef PRINT_UTILS

/**
 * @brief Prints a set in the format `{"item1", "item2", "item3"}`.
 *
 * @param set The set to print.
 * @param setSize The number of elements in the set.
 * @param data Data containing the headers to which the indices in the set corresponds.
 */
static void printSet(const int *set, int setSize, const TableData *data) {
    printf("{");
    for (int k = 0; k < setSize; k++) {
        printf("%s", data->headers[set[k]]);
        if (k != setSize - 1) {
            printf(", ");
        }
    }
    printf("}\n");
}

/**
 * @brief Prints the indices of a set in the format `{1, 2, 3}`.
 *
 * @param set The set to print.
 * @param setSize The number of elements in the set.
 */
static void printSetIndices(const int *set, int setSize) {
    printf("{");
    for (int k = 0; k < setSize; k++) {
        printf("%d", set[k]);
        if (k != setSize - 1) {
            printf(", ");
        }
    }
    printf("}\n");
}

/**
 * @brief Prints the data in the TableData struct. First prints the headers and then data->numRows lines containing the
 * integer data.
 *
 * @param data TableData to print.
 */
static void printTableData(const TableData *data) {
    printf("Num rows: %d\nNum cols: %d\n", data->numRows, data->numCols);
    for (int x = 0; x < data->numCols; x++) {
        printf("%s, ", data->headers[x]);
    }
    printf("\n");
    for (int y = 0; y < data->numRows; y++) {
        for (int x = 0; x < data->numCols; x++) {
            printf("%d, ", data->data[y][x]);
        }
        printf("\n");
    }
}

#endif

/**
 * @brief Prints a formatted association rule in the format `{A,B,C} => {D} Confidence: 0.6`. Every printed association
 * rule is alligned with the previous one.
 *
 * @param numItemsLeft Number of items in the antecedent. In the example of `{A,B,C} => {D}`, this would be 3.
 * @param numItemsRight Number of items in the consequent. In the example of `{A,B,C} => {D}`, this would be 1.
 * @param cols Columns in the complete set. In the example of `{A,B,C} => {D}`,, this set would contain {A, B, C, D}.
 * @param data TableData* Table where each row signifies a transaction and each column a product. An entry in this table
 * is either 1 or 0, depending on whether the product occured in the provided transacion.
 * @param confidence The confidence value to print.
 */
static void printAssociationRule(int numItemsLeft, int numItemsRight, const int *cols, const TableData *data,
                                 float confidence) {
    static char buffer1[BUFFER_LEN];
    char *curBuffer = buffer1;
    for (int i = 0; i < numItemsLeft; i++) {
        curBuffer += sprintf(curBuffer, "%s", data->headers[cols[i]]);
        if (i != numItemsLeft - 1) {
            curBuffer += sprintf(curBuffer, ", ");
        }
    }
    curBuffer += sprintf(curBuffer, "}");
    printf("{%-60s => {", buffer1);
    // Reset and reuse buffer
    curBuffer = buffer1;
    for (int i = 0; i < numItemsRight; i++) {
        curBuffer += sprintf(curBuffer, "%s", data->headers[cols[numItemsLeft + i]]);
        if (i != numItemsRight - 1) {
            curBuffer += sprintf(curBuffer, ", ");
        }
    }
    curBuffer += sprintf(curBuffer, "}");
    printf("%-30s%s%.1lf\n", buffer1, "Confidence: ", confidence * 100);
}

/**
 * @brief Reads data from a CSV into a TableData struct. The CSV file is assumed to have a header. Each row signifies a
 * transaction and each column a product. An entry in this table is either a single character or empty, depending on
 * whether the product occured in the provided transacion.
 *
 * @param path Path to the CSV file.
 * @return TableData* Table where each row signifies a transaction and each column a product. An entry in this table is
 * either 1 or 0, depending on whether the product occured in the provided transacion.
 */
static TableData *readCSV(const char *path) {
    FILE *csvFile = fopen(path, "r");
    char *headerLine = NULL;
    size_t headerLength = 0;
    int numRead = getline(&headerLine, &headerLength, csvFile);
    if (numRead == -1) {
        warning("Invalid CSV file. Could not read header of file at path: %s\n", path);
        return NULL;
    }
    int numCols = 0;
    for (int i = 0; i < numRead; i++) {
        if (headerLine[i] == ',' || headerLine[i] == '\n') {
            numCols++;
        }
    }
    char **headers = safeMalloc(numCols * sizeof(char *));

    int currentCol = 1;
    headers[0] = headerLine;
    for (int i = 0; i < numRead; i++) {
        if (headerLine[i] == ',' || headerLine[i] == '\n') {
            headerLine[i] = '\0';
            if (i != numRead - 1) {
                headers[currentCol++] = &headerLine[i + 1];
            }
        }
    }

    long int dataStartPos = ftell(csvFile);
    int numRows = 0;
    while (!feof(csvFile)) {
        if (fgetc(csvFile) == '\n') {
            numRows++;
        }
    }

    int **data = allocIntMatrix(numCols, numRows);

    fseek(csvFile, dataStartPos, SEEK_SET);

    for (int r = 0; r < numRows; r++) {
        for (int c = 0; c < numCols; c++) {
            char ch = fgetc(csvFile);
            if (ch == ',' || ch == '\n') {
                data[r][c] = 0;
            } else {
                data[r][c] = 1;
                fgetc(csvFile);
            }
        }
    }

    fclose(csvFile);
    TableData *csv = malloc(sizeof(TableData));
    csv->headers = headers;
    csv->data = data;
    csv->numRows = numRows;
    csv->numCols = numCols;
    return csv;
}

/**
 * @brief Calculates the support for a given set. Note that it only counts the number of rows/
 *
 * @param data Table where each row signifies a transaction and each column a product. An entry in this table is either
 * 1 or 0, depending on whether the product occured in the provided transacion.
 * @param set The set to calculate the support of.
 * @param setSize The number of elements in the set.
 * @return int Number of transactions the complete set occurs in.
 */
static int calcSupport(const TableData *data, const int *set, int setSize) {
    int support = 0;
    for (int y = 0; y < data->numRows; y++) {
        int supported = 1;
        for (int x = 0; x < setSize; x++) {
            if (data->data[y][set[x]] == 0) {
                supported = 0;
                break;
            }
        }
        support += supported;
    }
    return support;
}

/**
 * @brief Performs a prune step on the given level sets. It does this by calculating the support of every item set and
 * leaving only those that satisfy the given minimum support.
 *
 * @param levelSet The level set to prune.
 * @param data Table where each row signifies a transaction and each column a product. An entry in this table is either
 * 1 or 0, depending on whether the product occured in the provided transacion.
 * @param minSupportRows The minimum number of rows an item set must occur in for it to be added to the level set.
 */
static void prune(LevelSets *levelSet, const TableData *data, int minSupportRows) {
    int setIdx = 0;
    for (int c = 0; c < levelSet->numSets; c++) {
        // SetSize = k
        int support = calcSupport(data, levelSet->sets[c], levelSet->setSize);
        if (support >= minSupportRows) {
            mapPut(setToString(levelSet->sets[c], levelSet->setSize), support);
            // Overwrite columns
            levelSet->sets[setIdx++] = levelSet->sets[c];
        }
    }
    levelSet->numSets = setIdx;
    printf("Size of large itemsets l(%d) %d\n", levelSet->setSize, setIdx);
}

#ifdef USE_ANTI_MONOTONICITY_SUPPORT
int subsetsExist(const int *set, int n) {
    int subsetNum = (1 << n) - 1;   // -1 to skip the set itself
    static int subset[MAX_LEVELS];  // setSize is small enough to allocate on the stack
    while (subsetNum-- > 0) {
        int subsetMask = subsetNum;
        int numLeft = 0;

        for (int i = 0; i < n && subsetMask; i++) {
            if (subsetMask && subsetMask & 1) {
                subset[numLeft++] = set[i];
            }
            subsetMask >>= 1;
        }
        if (numLeft == 0) {
            break;
        }
        if (mapGet(setToString(subset, numLeft)) == -1) {
            return 0;
        }
    }
    return 1;
}
#endif

/**
 * @brief Performs a self join and prune step. Calculates the level sets at level k.
 *
 * @param levelSetK_1 Level set at level k-1
 * @param k number of the new level. Equal to the index of the new level + 1
 * @param data Table where each row signifies a transaction and each column a product. An entry in this table is either
 * 1 or 0, depending on whether the product occured in the provided transacion.
 * @param minSupportRows The minimum number of rows an item set must occur in for it to be added to the level set.
 * @return LevelSets* Thew newly generated level sets at level k. NULL if no frequent item sets were found.
 */
static LevelSets *selfJoin(const LevelSets *levelSetK_1, int k, const TableData *data, int minSupportRows) {
    int n = levelSetK_1->numSets;
    int numNewSets = (n * (n + 1)) / 2;
    int **setsAtLevelK_1 = levelSetK_1->sets;
    int setIdx = 0;
    int **setsAtLevelK = allocIntMatrix(k, numNewSets);
    for (int fs = 0; fs < levelSetK_1->numSets - 1; fs++) {
        for (int ss = fs + 1; ss < levelSetK_1->numSets; ss++) {
            for (int k_1 = 0; k_1 < k - 2; k_1++) {
                if (setsAtLevelK_1[fs][k_1] != setsAtLevelK_1[ss][k_1]) {
                    // First k-2 elements do not match, so continue with iteration of the outer for loop
                    // One of the few valid use cases of the goto statement :o
                    // I feel like a dinosaur though
                    goto continue_outer_loop;
                }
            }
            // Generate new set
            for (int i = 0; i < k - 1; i++) {
                setsAtLevelK[setIdx][i] = setsAtLevelK_1[fs][i];
            }
            setsAtLevelK[setIdx][k - 1] = setsAtLevelK_1[ss][k - 2];
#ifdef USE_ANTI_MONOTONICITY_SUPPORT
            if (!subsetsExist(setsAtLevelK[setIdx], k)) {
                continue;
            }
#endif

            // Immediately prune any invalid generated sets
            // If the generated set is invalid, the setIdx is not incremented and it will be overriden in the next
            // iteration
            int support = calcSupport(data, setsAtLevelK[setIdx], k);
            if (support >= minSupportRows) {
                mapPut(setToString(setsAtLevelK[setIdx], k), support);
                setIdx++;
            }
        }
    continue_outer_loop:;
    }

    if (setIdx == 0) {
        free(setsAtLevelK);
        return NULL;
    }
    LevelSets *levelSet = safeMalloc(sizeof(LevelSets));
    levelSet->sets = setsAtLevelK;
    levelSet->numSets = setIdx;
    levelSet->setSize = k;
    printf("Size of large itemsets l(%d) %d\n", k, setIdx);
    return levelSet;
}

/**
 * @brief Generates the level sets. Note that the level sets work with column indices instead of column names for
 * performance reasons.
 *
 * @param finalLevel The number of level sets generated will be written to this pointer.
 * @param data Table where each row signifies a transaction and each column a product. An entry in this table is either
 * 1 or 0, depending on whether the product occured in the provided transacion.
 * @param minSupportRows The minimum number of rows an item set must occur in for it to be added to the level set.
 * @return LevelSets** Provides for each of the finalLevel levels a number of sets that satisfy the minimum support
 * criterion. At a given level k, the size of each set is k elements.
 */
static LevelSets **createFrequentItemSets(int *finalLevel, const TableData *data, int minSupportRows) {
    int level = 0;
    int **setsAtLevel1 = allocIntMatrix(level + 1, data->numCols);
    for (int i = 0; i < data->numCols; i++) {
        setsAtLevel1[i][level] = i;
    }
    LevelSets *set1 = safeMalloc(sizeof(LevelSets));
    set1->sets = setsAtLevel1;
    set1->numSets = data->numCols;
    set1->setSize = level + 1;

    prune(set1, data, minSupportRows);

    LevelSets **sets = safeMalloc(MAX_LEVELS * sizeof(LevelSets));
    sets[level] = set1;
    for (int i = 0; i < MAX_LEVELS; i++) {
        level++;
        // The prune step is merged with the selfJoin starting at k=2
        LevelSets *setK = selfJoin(sets[level - 1], level + 1, data, minSupportRows);
        if (!setK) {
            // No item sets generated
            break;
        }
        sets[level] = setK;
    }
    *finalLevel = level;
    return sets;
}

/**
 * @brief Calculates the confidence of an association rule.
 *
 * @param set The set containing the full association rules. The first numLeft items are the antecedent, the rest belong
 * to the consequent.
 * @param numLeft Number of items in the antecedent.
 * @param setSupport The support of the entire set. Provided here so that it doesn't need to be recalculated all the
 * time.
 * @return float Confidence. For the rule {A, B} => {C} this will return support(A, B, C) / support(A, B)
 */
inline static float confidence(const int *set, int numLeft, int setSupport) {
    return setSupport / (float)mapGet(setToString(set, numLeft));
}

#ifdef USE_ANTI_MONOTONICITY_CONFIDENCE
void pruneSubsets(const int *set, int n) {
    int subsetNum = (1 << n);
    static int subset[MAX_LEVELS];  // setSize is small enough to allocate on the stack

    while (subsetNum-- > 0) {
        int subsetMask = subsetNum;
        int numLeft = 0;
        for (int i = 0; i < n && subsetMask; i++) {
            if (subsetMask && subsetMask & 1) {
                subset[numLeft++] = i;
            }
            subsetMask >>= 1;
        }
        mapPut(setToString(subset, numLeft), -1);
    }
}
#endif

/**
 * @brief Checks for all subsets of a given set whether the corresponding association rule satisfies the minimum
 * confidence. If so, it will print the rule.
 *
 * @param subsetMask  Mask used to determine which elements from the set end up in the antecedent.
 * @param set The set to generate the subsets of.
 * @param setSize The size of the set.
 * @param data Table where each row signifies a transaction and each column a product. An entry in this table is either
 * 1 or 0, depending on whether the product occured in the provided transacion.
 * @param minConfidence The minimum confidence that an association rule must have for it to be sent to the output.
 * @param setSupport The support of the provided set.
 */
static void checkSubsets(int subsetMask, const int *set, int setSize, const TableData *data, float minConfidence,
                         int setSupport) {
#ifdef USE_ANTI_MONOTONICITY_CONFIDENCE
    if (mapGet(setToString(set, setSize)) == -1) {
        return;
    }
#endif
    // Big brain; the number of 1s in the subsetNum indicates the number of items in the antecedent. As such, we can
    // calculate the start position of the consequent.
    int numRight = __builtin_popcount(subsetMask);
    if (numRight >= setSize || numRight == 0) {
        return;
    }
    int numLeft = 0;
    static int cols[MAX_LEVELS];  // setSize is small enough to allocate on the stack
    for (int i = 0; i < setSize; i++) {
        if (subsetMask && subsetMask & 1) {
            cols[numLeft++] = set[i];
        } else {
            cols[numRight++] = set[i];
        }
        subsetMask >>= 1;
    }

    float conf = confidence(cols, numLeft, setSupport);
    if (conf >= minConfidence) {
        printAssociationRule(numLeft, setSize - numLeft, cols, data, conf);
    } else {
#ifdef USE_ANTI_MONOTONICITY_CONFIDENCE
        // Prune any subsets by the anti-monotonicity rule of the confidence
        // TODO: only prune sets of size - 1?
        pruneSubsets(cols, numLeft);
#endif
    }
}

/**
 * @brief Generates association rules in the form `{A,B,C} => {D} Confidence: 0.6` from the provided level sets and
 * data. The hash table from <search.h> is assumed to contain all supports of the to-be-calculated subsets of the level
 * sets.
 *
 * @param sets The level sets. Provides for each of the n levels a number of sets that satisfy the minimum support
 * criterion. At a given level k, the size of each set is k elements.
 * @param n Number of level sets.
 * @param data Table where each row signifies a transaction and each column a product. An entry in this table is either
 * 1 or 0, depending on whether the product occured in the provided transacion.
 * @param minConfidence The minimum confidence that an association rule must have for it to be sent to the output.
 */
static void generateAssociationRules(LevelSets **sets, int n, const TableData *data, float minConfidence) {
    for (int i = n - 1; i > 0; i--) {
        LevelSets *set = sets[i];
        int setSize = set->setSize;
        int numSets = set->numSets;
        int initialSubsetNum = (1 << setSize) - 1;  // -1 to prevent the set itself
        for (int i = 0; i < numSets; i++) {
            int subsetNum = initialSubsetNum;
            int setSupport = mapGet(setToString(set->sets[i], setSize));
#ifdef USE_ANTI_MONOTONICITY_CONFIDENCE
            if (mapGet(setToString(set->sets[i], setSize)) == -1) {
                continue;
            }
#endif
            while (subsetNum-- > 0) {
                checkSubsets(subsetNum, set->sets[i], setSize, data, minConfidence, setSupport);
            }
        }
    }
}

/**
 * @brief Performs the apriori algorithms on the data located at the csv Path and prints all the corresponding
 * association rules.
 *
 * @param csvPath Table where each row signifies a transaction and each column a product. An entry in this table is
 * either 1 or 0, depending on whether the product occured in the provided transacion.
 * @param minSupport The minimum support a frequent item set must have.
 * @param minConfidence The minimum confidence an association rule must have to be printed.
 */
void apriori(TableData *data, float minSupport, float minConfidence) {
    if (!data) {
        warning("No data preset in the provided data variable.");
        return;
    }
    if (data->numRows == 0 || data->numCols == 0) {
        warning("No data preset in the provided data variable.");
        return;
    }
    hcreate(HASH_TABLE_SIZE);

    int numLevels;
    int minSupportRows = data->numRows * minSupport;
    LevelSets **sets = createFrequentItemSets(&numLevels, data, minSupportRows);
    printf("\n");
    generateAssociationRules(sets, numLevels, data, minConfidence);

    // Clean up
    for (int i = 0; i < numLevels; i++) {
        freeLevelSet(sets[i]);
    }
    free(sets);
    hdestroy();
}

/**
 * @brief Performs the apriori algorithms on the data located at the csv Path and prints all the corresponding
 * association rules.
 *
 * @param csvPath Path to a csv file. The CSV file is assumed to have a header. Each row signifies a
 * transaction and each column a product. An entry in this table is either a single character or empty, depending on
 * whether the product occured in the provided transacion.
 * @param minSupport The minimum support a frequent item set must have.
 * @param minConfidence The minimum confidence an association rule must have to be printed.
 */
void aprioriCSV(const char *csvPath, float minSupport, float minConfidence) {
    TableData *data = readCSV(csvPath);
    apriori(data, minSupport, minConfidence);
    freeCSV(data);
}
