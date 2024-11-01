#include <stdio.h>
#include <stdbool.h>
#include<time.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>

int num_students;
int num_labs;
int num_courses;
clock_t start;       //for time
clock_t present;

// labs struct

struct lab             // lab struct 
{
    char name[10];
    int num_ta;
    int lab_limit;
    int *ta_limit;     
    bool *available;
    bool all_limits_done;
    pthread_mutex_t lock;         //lock for a lab 
};

struct course
{
    char name[10];
    int courseid;
    float interest;
    bool ta_assigned;
    int num_labs;
    int* lab_ids;
    int max_students;
    int ta_id;
    int lab_id;
    int num_students;
    int slots;
    pthread_mutex_t lock;
    pthread_cond_t c;                // for waiting students for atttending course tut
    pthread_cond_t v;                // for waiting to end tut
}; 

struct student
{
    float students_calibre;
    int studentid;
    int prefernece_1;
    int prefernece_2;
    int prefernece_3;
    int time_registered;
    int curr_pref;
    pthread_mutex_t lock;
    bool active;
};

struct course *courses ;
struct student *students;
struct lab *labs_data;

void *course_init(void *course_deet);
void *student_init(void *student_deets);
bool probability(float prob);
