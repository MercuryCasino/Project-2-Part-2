#include "BENSCHILLIBOWL.h"

#include <assert.h>
#include <stdlib.h>
#include <time.h>

bool IsEmpty(BENSCHILLIBOWL* bcb);
bool IsFull(BENSCHILLIBOWL* bcb);
void AddOrderToBack(Order **orders, Order *order);

MenuItem BENSCHILLIBOWLMenu[] = { 
    "BensChilli", 
    "BensHalfSmoke", 
    "BensHotDog", 
    "BensChilliCheeseFries", 
    "BensShake",
    "BensHotCakes",
    "BensCake",
    "BensHamburger",
    "BensVeggieBurger",
    "BensOnionRings",
};
int BENSCHILLIBOWLMenuLength = 10;

/* Select a random item from the Menu and return it */
MenuItem PickRandomMenuItem() {
    return BENSCHILLIBOWLMenu[rand() % BENSCHILLIBOWLMenuLength];
}

/* Allocate memory for the Restaurant, then create the mutex and condition variables needed to instantiate the Restaurant */
BENSCHILLIBOWL* OpenRestaurant(int max_size, int expected_num_orders) {
    BENSCHILLIBOWL *bcb = (BENSCHILLIBOWL*) malloc(sizeof(BENSCHILLIBOWL));
    
    // Initialize properties
    bcb->orders = NULL;
    bcb->current_size = 0;
    bcb->max_size = max_size;
    bcb->next_order_number = 1;
    bcb->orders_handled = 0;
    bcb->expected_num_orders = expected_num_orders;
    
    // Initialize Synchronization Objects
    pthread_mutex_init(&bcb->mutex, NULL);
    pthread_cond_init(&bcb->can_add_orders, NULL);
    pthread_cond_init(&bcb->can_get_orders, NULL);
    
    printf("Restaurant is open!\n");
    return bcb;
}

/* check that the number of orders received is equal to the number handled (ie.fullfilled). Remember to deallocate your resources */
void CloseRestaurant(BENSCHILLIBOWL* bcb) {
    if (bcb->orders_handled != bcb->expected_num_orders) {
        fprintf(stderr, "Restaurant closed before all orders were handled!\n");
    }
    
    // Destroy Synchronization Objects
    pthread_mutex_destroy(&bcb->mutex);
    pthread_cond_destroy(&bcb->can_add_orders);
    pthread_cond_destroy(&bcb->can_get_orders);
    
    free(bcb);
    printf("Restaurant is closed!\n");
}

/* add an order to the back of queue */
int AddOrder(BENSCHILLIBOWL* bcb, Order* order) {
    // Lock the mutex before modifying restaurant queues
    pthread_mutex_lock(&bcb->mutex);
    
    // Wait until the restaurant is not full
    while (IsFull(bcb)) {
        pthread_cond_wait(&bcb->can_add_orders, &bcb->mutex);
    }
    
    // Populate the order number and add to queue
    order->order_number = bcb->next_order_number++;
    AddOrderToBack(&bcb->orders, order);
    bcb->current_size++;
    
    // Signal the cooks that an order has arrived
    pthread_cond_signal(&bcb->can_get_orders);
    
    // Unlock the mutex
    pthread_mutex_unlock(&bcb->mutex);
    
    return order->order_number;
}

/* remove an order from the queue */
Order *GetOrder(BENSCHILLIBOWL* bcb) {
    // Lock the mutex before modifying restaurant queues
    pthread_mutex_lock(&bcb->mutex);
    
    // Wait until the restaurant is not empty
    while (IsEmpty(bcb)) {
        // Condition: If all expected orders are already handled, break Cooks out of their wait.
        if (bcb->orders_handled >= bcb->expected_num_orders) {
            pthread_cond_broadcast(&bcb->can_get_orders); 
            pthread_mutex_unlock(&bcb->mutex);
            return NULL;
        }
        pthread_cond_wait(&bcb->can_get_orders, &bcb->mutex);
    }
    
    // Get the order from the front of the queue
    Order *order = bcb->orders;
    bcb->orders = bcb->orders->next;
    
    bcb->current_size--;
    bcb->orders_handled++;
    
    // Signal the customers that there is room to add new orders
    pthread_cond_signal(&bcb->can_add_orders);
    
    // If we've hit our expected orders, broadcast to ensure all remaining cooks wake up and terminate
    if (bcb->orders_handled >= bcb->expected_num_orders) {
        pthread_cond_broadcast(&bcb->can_get_orders);
    }
    
    // Unlock the mutex
    pthread_mutex_unlock(&bcb->mutex);
    
    return order;
}

// Optional helper functions
bool IsEmpty(BENSCHILLIBOWL* bcb) {
    return bcb->current_size == 0;
}

bool IsFull(BENSCHILLIBOWL* bcb) {
    return bcb->current_size == bcb->max_size;
}

/* this methods adds order to rear of queue */
void AddOrderToBack(Order **orders, Order *order) {
    order->next = NULL;
    if (*orders == NULL) {
        *orders = order;
    } else {
        Order *curr = *orders;
        while (curr->next != NULL) {
            curr = curr->next;
        }
        curr->next = order;
    }
}