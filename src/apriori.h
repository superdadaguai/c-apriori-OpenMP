#ifndef APRIORI_H
#define APRIORI_H

/**
 * @brief Struct that describes a table containing integer data.
 */
typedef struct TableData {
    int numRows;  // Excluding header
    int numCols;
    char **headers;
    int **data;
} TableData;

/**
 * @brief Performs the apriori algorithms on the data located at the csv Path and prints all the corresponding
 * association rules.
 *
 * @param csvPath Table where each row signifies a transaction and each column a product. An entry in this table is
 * either 1 or 0, depending on whether the product occured in the provided transacion.
 * @param minSupport The minimum support a frequent item set must have.
 * @param minConfidence The minimum confidence an association rule must have to be printed.
 */
void apriori(TableData *data, float minSupport, float minConfidence);

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
void aprioriCSV(const char *csvPath, float minSupport, float minConfidence);

#endif  // APRIORI_H
