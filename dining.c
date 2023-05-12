#include "dining.h"

#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>

typedef struct dining {
  // TODO: Add your variables here
  int capacity;
  sem_t student_semaphore; // Used for keeping the dining hall under the capacity limit
  pthread_mutex_t mutex; // For creating critical regions, and also blocks when a cleaner is present and another student or cleaner arrives
  pthread_cond_t cond_empty; // Signals when dining hall has no more students
} dining_t;

dining_t *dining_init(int capacity) {
  // TODO: Initialize necessary variables
  dining_t *dining = malloc(sizeof(dining_t));
  dining->capacity = capacity;
  sem_init(&dining->student_semaphore, 0, capacity);
  pthread_mutex_init(&dining->mutex, NULL);
  pthread_cond_init(&dining->cond_empty, NULL);
  return dining;
}

void dining_destroy(dining_t **dining) {
  // TODO: Free dynamically allocated memory
  sem_destroy(&(*dining)->student_semaphore);
  pthread_mutex_destroy(&(*dining)->mutex);
  pthread_cond_destroy(&(*dining)->cond_empty);
  free(*dining);
  *dining = NULL;
}

void dining_student_enter(dining_t *dining) {
  sem_wait(&dining->student_semaphore); // Block if capacity reached
  pthread_mutex_lock(&dining->mutex); // Barrier - Wait on cleaner to leave
  pthread_mutex_unlock(&dining->mutex); // Unlock mutex so other students can keep entering if no cleaner is present
}

void dining_student_leave(dining_t *dining) {
  sem_post(&dining->student_semaphore); // Update semaphore when student leaves
  pthread_mutex_lock(&dining->mutex); // Critical region -- ensure everything ahndling before student leaves and the lock reacquired
  int val;
  sem_getvalue(&dining->student_semaphore, &val); // Store current semaphore count in val
  if (val == dining->capacity) {
    pthread_cond_signal(&dining->cond_empty); // Signal once the dining hall is empty so any waiting cleaners can enter
    // This completes the extra credit portion since cleaners will enter as soon as they can
    // and therefore won't be waiting indefinitely if more students keep arriving after the cleaner does
  }
  pthread_mutex_unlock(&dining->mutex);
}

void dining_cleaning_enter(dining_t *dining) {
  pthread_mutex_lock(&dining->mutex); // Lock mutex when cleaner enters to keep students or other cleaners from entering
  int val;
  sem_getvalue(&dining->student_semaphore, &val); // Get current semaphore count
  if (val < dining->capacity) { // No need for while loop since only one thread can be waiting here at a time
                                // so there's no chance of a spurious wakeup here
    // Release the mutex so other students can enter if they arrived before the cleaer
    // then wait and reacquire mutex once the dining hall is empty
    pthread_cond_wait(&dining->cond_empty, &dining->mutex);
  }
}

void dining_cleaning_leave(dining_t *dining) {
  pthread_mutex_unlock(&dining->mutex); // Release mutex once cleaner leaves so others can enter
}
