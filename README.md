<br />
<p align="center">
  <h1 align="center">C-Apriori</h1>

  <p align="center">
    A fast C implementation of the Apriori algorithm
  </p>
</p>

## Build & Run

- Build using:

```shell
make
```

To analyse the `myDataFile.csv` file, run:

```shell
./apriori myDataFile.csv
```

Example output:

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
