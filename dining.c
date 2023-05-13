#include "dining.h"

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>

typedef struct dining {
  int capacity;
  sem_t student_semaphore; // This is used for keeping the dining hall under the capacity limit
  pthread_mutex_t mutex; // This is for blocking any cleaner or student that arrives after the current cleaner
  pthread_mutex_t mutex2; // This is used to allow students already in the dining hall to finish and leave once a cleaner arrives
  pthread_cond_t cond_empty; // This signals when dining hall has no more students to allow cleaners to enter
} dining_t;

dining_t *dining_init(int capacity) {
  dining_t *dining = malloc(sizeof(dining_t));
  dining->capacity = capacity;
  sem_init(&dining->student_semaphore, 0, capacity);
  pthread_mutex_init(&dining->mutex, NULL);
  pthread_mutex_init(&dining->mutex2, NULL);
  pthread_cond_init(&dining->cond_empty, NULL);
  return dining;
}

void dining_destroy(dining_t **dining) {
  sem_destroy(&(*dining)->student_semaphore);
  pthread_mutex_destroy(&(*dining)->mutex);
  pthread_mutex_destroy(&(*dining)->mutex2);
  pthread_cond_destroy(&(*dining)->cond_empty);
  free(*dining);
  *dining = NULL;
}

void dining_student_enter(dining_t *dining) {
  pthread_mutex_lock(&dining->mutex); // Barrier - Blocks if arriving after current clener
  pthread_mutex_unlock(&dining->mutex); // Unlock mutex so other students can keep entering once no cleaner is present
  sem_wait(&dining->student_semaphore); // Block if capacity reached
}

void dining_student_leave(dining_t *dining) {
  sem_post(&dining->student_semaphore); // Update semaphore when student leaves
  pthread_mutex_lock(&dining->mutex2); // Critical region -- ensure everything handled before student leaves
  int val;
  sem_getvalue(&dining->student_semaphore, &val); // Store current semaphore count in val
  if (val == dining->capacity) {
    pthread_cond_signal(&dining->cond_empty); // Signal once the dining hall is empty so any waiting cleaners can enter
    // This completes the extra credit portion since cleaners will enter as soon as they can
    // and therefore won't be waiting indefinitely if more students keep arriving after the cleaner does
  }
  pthread_mutex_unlock(&dining->mutex2);
}

void dining_cleaning_enter(dining_t *dining) {
  pthread_mutex_lock(&dining->mutex); // Lock mutex when cleaner enters to keep students or other cleaners from entering if they arrive after the current cleaner
  pthread_mutex_lock(&dining->mutex2); // Lock mutex2 for potential wait condition
  int val;
  sem_getvalue(&dining->student_semaphore, &val); // Get current semaphore count
  // No need for while loop since only one thread can be waiting here at a time
  // so there's no chance of a spurious wakeup here
  if (val < dining->capacity) {
    // Release mutex2 so other students who arrived before the cleaner can leave, and
    // then wait and reacquire mutex once the dining hall is empty
    pthread_cond_wait(&dining->cond_empty, &dining->mutex2);
  }
}

void dining_cleaning_leave(dining_t *dining) {
  pthread_mutex_unlock(&dining->mutex); // Release mutex once cleaner leaves so others can enter
  pthread_mutex_unlock(&dining->mutex2); // Release mutex2
}
