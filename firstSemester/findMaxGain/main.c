#include <stdio.h>
#include <stdlib.h>


typedef struct {
    int *arr;
    size_t size;
    size_t count;
} Array;

typedef struct {
    long long priceDifference;
    size_t buyIndex;
    size_t sellIndex;
} StockAction;

typedef struct {
    StockAction maxProfit;
    StockAction maxLoss;
} MaxGainLoss;

void createArray(Array *array, const size_t size) {
    array->arr = (int *) malloc(size * sizeof(array->arr));
    array->size = size;
    array->count = 0;
}

void push(Array *array, const int value) {
    if (array->count == array->size) {
        array->size *= 2;
        array->arr = (int *) realloc(array->arr, array->size * sizeof(array->arr));
    }
    array->arr[array->count++] = value;
}

MaxGainLoss getMaxGainAndLoss(const Array *array, size_t startIndex, size_t endIndex);

void calculateStockQuery(const Array *array, size_t startIndex, size_t endIndex);

int main(void) {
    printf("Ceny, hledani:\n");

    Array array;
    //to not waste time on reallocating memory
    createArray(&array, 100);

    while (true) {
        //get the first char and then decide what to do
        char leadingChar;
        int conversionRes = scanf(" %c", &leadingChar);

        if (conversionRes == EOF) {
            break;
        }

        if (leadingChar == '+') {
            int stockPrice;
            if (scanf(" %d", &stockPrice) != 1 || stockPrice < 0) {
                printf("Nespravny vstup.\n");
                free(array.arr);
                return 1;
            }
            push(&array, stockPrice);
            continue;
        }


        if (leadingChar == '?') {
            size_t startIndex, endIndex;
            if (scanf(" %zu %zu", &startIndex, &endIndex) != 2 || startIndex >= array.count || endIndex >= array.count || startIndex > endIndex) {
                printf("Nespravny vstup.\n");
                free(array.arr);
                return 1;
            }

            calculateStockQuery(&array, startIndex, endIndex);
            continue;
        }

        printf("Nespravny vstup.\n");
        free(array.arr);
        return 1;
    }

    free(array.arr);
    return 0;
}

//gets the result and prints them
void calculateStockQuery(const Array *const array, const size_t startIndex, const size_t endIndex) {
    const MaxGainLoss res = getMaxGainAndLoss(array, startIndex, endIndex);

    if (res.maxProfit.priceDifference != 0) {
        printf("Nejvyssi zisk: %lld (%zu - %zu)\n", res.maxProfit.priceDifference, res.maxProfit.buyIndex,
               res.maxProfit.sellIndex);
    } else {
        printf("Nejvyssi zisk: N/A\n");
    }

    if (res.maxLoss.priceDifference != 0) {
        printf("Nejvyssi ztrata: %lld (%zu - %zu)\n", res.maxLoss.priceDifference, res.maxLoss.buyIndex,
               res.maxLoss.sellIndex);
    } else {
        printf("Nejvyssi ztrata: N/A\n");
    }
}


MaxGainLoss getMaxGainAndLoss(const Array *const array, const size_t startIndex, const size_t endIndex) {
    StockAction profit = {};
    StockAction loss = {};

    profit.priceDifference = 0;
    profit.buyIndex = startIndex;
    profit.sellIndex = startIndex + 1;

    loss.priceDifference = 0;
    loss.buyIndex = startIndex;
    loss.sellIndex = startIndex + 1;

    size_t buyIndexForGain = startIndex;
    size_t buyIndexForLoss = startIndex;

    //iterates over the array and remembers the best profit and loss
    for (size_t sellIndex = startIndex + 1; sellIndex <= endIndex; sellIndex++) {
        long long currentDifferenceForGain = array->arr[sellIndex] - array->arr[buyIndexForGain];
        long long currentDifferenceForLoss = array->arr[sellIndex] - array->arr[buyIndexForLoss];

        //if the difference is higher than the current profit, it sets the new profit (for loss the other way)
        if (currentDifferenceForGain > profit.priceDifference) {
            profit.priceDifference = currentDifferenceForGain;
            profit.buyIndex = buyIndexForGain;
            profit.sellIndex = sellIndex;
        } //can use else if, both can never be true at the same time
        else if (currentDifferenceForLoss < loss.priceDifference) {
            loss.priceDifference = currentDifferenceForLoss;
            loss.buyIndex = buyIndexForLoss;
            loss.sellIndex = sellIndex;
        }

        //when finds lower price than the current buy price, changes the buyIndex to the current one(for loss the other way)
        if (array->arr[sellIndex] < array->arr[buyIndexForGain]) {
            buyIndexForGain = sellIndex;
        }

        if (array->arr[sellIndex] > array->arr[buyIndexForLoss]) {
            buyIndexForLoss = sellIndex;
        }
    }
    loss.priceDifference *= -1;

    MaxGainLoss maxGainLoss = {};
    maxGainLoss.maxProfit = profit;
    maxGainLoss.maxLoss = loss;
    return maxGainLoss;
}
