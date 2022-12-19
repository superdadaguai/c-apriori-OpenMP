#ifndef APRIORI_H
#define APRIORI_H

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
void apriori(const char *csvPath, float minSupport, float minConfidence);

#endif  // APRIORI_H
