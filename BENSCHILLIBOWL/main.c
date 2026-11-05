#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#include "BENSCHILLIBOWL.h"

// Feel free to play with these numbers! This is a great way to
// test your implementation.
#define BENSCHILLIBOWL_SIZE 100
#define NUM_CUSTOMERS 90
#define NUM_COOKS 10
#define ORDERS_PER_CUSTOMER 3
#define EXPECTED_NUM_ORDERS NUM_CUSTOMERS * ORDERS_PER_CUSTOMER

// Global variable for the restaurant.
BENSCHILLIBOWL *bcb;

/**
 * Thread funtion that represents a customer. A customer should:
 *  - allocate space (memory) for an order.
 *  - select a menu item.
 *  - populate the order with their menu item and their customer ID.
 *  - add their order to the restaurant.
 */
void* BENSCHILLIBOWLCustomer(void* tid) {
    int customer_id = (int)(long) tid;
    
    // Each customer places multiple orders
    for (int i = 0; i < ORDERS_PER_CUSTOMER; i++) {
        // Allocate space for a new order
        Order *new_order = (Order*) malloc(sizeof(Order));
        if (new_order == NULL) {
            perror("Failed to allocate memory for order");
            exit(1);
        }
        
        // Populate the order
        new_order->menu_item = PickRandomMenuItem();
        new_order->customer_id = customer_id;
        new_order->next = NULL;
        
        // Add order to the restaurant
        AddOrder(bcb, new_order);
    }
    
    return NULL;
}

/**
 * Thread function that represents a cook in the restaurant. A cook should:
 *  - get an order from the restaurant.
 *  - if the order is valid, it should fulfill the order, and then
 *    free the space taken by the order.
 * The cook should take orders from the restaurants until it does not
 * receive an order.
 */
void* BENSCHILLIBOWLCook(void* tid) {
    int cook_id = (int)(long) tid;
    int orders_fulfilled = 0;
    
    while (1) {
        // Get an order from the restaurant
        Order *order = GetOrder(bcb);
        
        // If GetOrder returns NULL, there are no more orders expected
        if (order == NULL) {
            break;
        }
        
        // Fulfill the order and free its memory
        free(order);
        orders_fulfilled++;
    }
    
    printf("Cook #%d fulfilled %d orders\n", cook_id, orders_fulfilled);
    return NULL;
}

/**
 * Runs when the program begins executing. This program should:
 *  - open the restaurant
 *  - create customers and cooks
 *  - wait for all customers and cooks to be done
 *  - close the restaurant.
 */
int main() {
    // Seed the random number generator
    srand(time(NULL));
    
    // Open the restaurant
    bcb = OpenRestaurant(BENSCHILLIBOWL_SIZE, EXPECTED_NUM_ORDERS);
    
    // Arrays to hold the thread IDs
    pthread_t customers[NUM_CUSTOMERS];
    pthread_t cooks[NUM_COOKS];
    
    // Create Cook threads
    for (int i = 0; i < NUM_COOKS; i++) {
        pthread_create(&cooks[i], NULL, BENSCHILLIBOWLCook, (void*)(long)(i + 1));
    }
    
    // Create Customer threads
    for (int i = 0; i < NUM_CUSTOMERS; i++) {
        pthread_create(&customers[i], NULL, BENSCHILLIBOWLCustomer, (void*)(long)(i + 1));
    }
    
    // Wait for all Customers to finish
    for (int i = 0; i < NUM_CUSTOMERS; i++) {
        pthread_join(customers[i], NULL);
    }
    
    // Wait for all Cooks to finish
    for (int i = 0; i < NUM_COOKS; i++) {
        pthread_join(cooks[i], NULL);
    }
    
    // Close the restaurant
    CloseRestaurant(bcb);
    
    return 0;
}