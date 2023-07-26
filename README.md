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

```shell
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

For `myDataFile.csv`, the following output is produced:

```shell
No minimum support and confidence provided; using defaults 0.005 and 0.6

Size of large itemsets l(1) 120
Size of large itemsets l(2) 628
Size of large itemsets l(3) 279
Size of large itemsets l(4) 14

{citrus_fruit, root_vegetables, whole_milk}                   => {other_vegetables}             Confidence: 63.3
{fruit_vegetable_juice, other_vegetables, yogurt}             => {whole_milk}                   Confidence: 61.7
{pip_fruit, root_vegetables, whole_milk}                      => {other_vegetables}             Confidence: 61.4
{other_vegetables, pip_fruit, root_vegetables}                => {whole_milk}                   Confidence: 67.5
{other_vegetables, pip_fruit, yogurt}                         => {whole_milk}                   Confidence: 62.5
{root_vegetables, tropical_fruit, yogurt}                     => {other_vegetables}             Confidence: 61.2
{other_vegetables, root_vegetables, whipped_sour_cream}       => {whole_milk}                   Confidence: 60.7
{other_vegetables, root_vegetables, yogurt}                   => {whole_milk}                   Confidence: 60.6
{other_vegetables, tropical_fruit, yogurt}                    => {whole_milk}                   Confidence: 62.0
{root_vegetables, tropical_fruit, yogurt}                     => {whole_milk}                   Confidence: 70.0
{bottled_water, butter}                                       => {whole_milk}                   Confidence: 60.2
{butter, domestic_eggs}                                       => {whole_milk}                   Confidence: 62.1
{butter, root_vegetables}                                     => {whole_milk}                   Confidence: 63.8
{butter, tropical_fruit}                                      => {whole_milk}                   Confidence: 62.2
{butter, whipped_sour_cream}                                  => {whole_milk}                   Confidence: 66.0
{butter, yogurt}                                              => {whole_milk}                   Confidence: 63.9
{curd, tropical_fruit}                                        => {whole_milk}                   Confidence: 63.4
{domestic_eggs, margarine}                                    => {whole_milk}                   Confidence: 62.2
{domestic_eggs, pip_fruit}                                    => {whole_milk}                   Confidence: 62.4
{domestic_eggs, tropical_fruit}                               => {whole_milk}                   Confidence: 60.7
{onions, root_vegetables}                                     => {other_vegetables}             Confidence: 60.2
{pip_fruit, whipped_sour_cream}                               => {other_vegetables}             Confidence: 60.4
{pip_fruit, whipped_sour_cream}                               => {whole_milk}                   Confidence: 64.8

Execution took 0.558534 sec.
```
