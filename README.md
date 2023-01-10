<br />
<p align="center">
  <h1 align="center">C-Apriori</h1>

  <p align="center">
    A fast C implementation of the Apriori algorithm
  </p>
</p>

## About the project

This repository contains a reasonably fast implementation for the apriori algorithm.

## Getting Started

To get a local copy up and running follow these simple steps.

### Prerequisites

You need to the following to be able to compile and run the project

* [Make](https://www.gnu.org/software/make/)

### Setup

To set up the program, run the following commands:
```sh
    git clone git@github.com:BugelNiels/c-apriori.git
    make
```

### Running

You can run the project as follows with a given data file `myDataFile.csv` as follows:

```sh
./apriori myDataFile.csv
```

Running it without any additional arguments will assume a minimum support of 0.005 and a minimum confidence of 0.6.
Alternatively, you can provide the minimum support and confidence via tha commandline arguments:

```sh
./apriori myDataFile.csv 0.005 0.6
```

# Input data file

The CSV file represents contains the information of the products and the transactions. Every row represents a transaction. A cell in the csv has the value `t` if the product specified in the column occured in said transaction. A simply csv file might look like this:

```csv
A,B,C,D,E
t,,t,t,
,t,t,,t
t,t,t,,t
,t,,,t
```

Running the apriori algorithm on this CSV with a minimum support of `0.5` and a minimum confidence of `0.75`:

```sh
./apriori simple.csv 0.5 0.75
```

Produces the following output containing the association rules:

```
Size of large itemsets l(1) 4
Size of large itemsets l(2) 4
Size of large itemsets l(3) 1

{C, E}                                                        => {B}                            Confidence: 100.0
{B, C}                                                        => {E}                            Confidence: 100.0
{A}                                                           => {C}                            Confidence: 100.0
{E}                                                           => {B}                            Confidence: 100.0
{B}                                                           => {E}                            Confidence: 100.0

Execution took 0.000306 sec.
```
