#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#include "dining.h"
#include "utils.h"

// Tests for cleaners arriving between students leaving and ensures that all students leave before a cleaner enters
int main(void) {
  dining_t* d = dining_init(3);

  student_t student1 = make_student(1, d);
  student_t student2 = make_student(2, d);
  student_t student3 = make_student(3, d);
  cleaning_t cleaning = make_cleaning(1, d);

  // student 1 comes in, can enter
  student_enter(&student1);
  // student 2 enters
  student_enter(&student2);
  // student 3 enters
  student_enter(&student3);

  // cleaning cannot enter because of students 1, 2, and 3; this should block until the dining hall is empty
  pthread_create(&cleaning.thread, NULL, cleaning_enter, &cleaning);
  msleep(100);

  // student 1 leaves
  student_leave(&student1);
  // student 2 leaves
  student_leave(&student2);
  // student 3 leaves
  student_leave(&student3);

  // cleaner should now enter since the dining hall is empty and not anytime between students leaving
  // who are already in the dining hall
  pthread_join(cleaning.thread, NULL);
  msleep(100);
  cleaning_leave(&cleaning); // cleaning finishes

  dining_destroy(&d);
}
