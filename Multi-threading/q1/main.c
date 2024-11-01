#include "defs.h"

int main()
{
    start = clock();

    scanf("%d%d%d", &num_students, &num_labs, &num_courses);
    pthread_t course_thread[num_courses];
    pthread_t student_thread[num_students];
    courses = (struct course *)malloc(sizeof(struct course) * num_courses);
    for (int i = 0; i < num_courses; ++i)
    {
        scanf("%s", courses[i].name);
        scanf("%f", &courses[i].interest);
        scanf("%d", &courses[i].max_students);
        scanf("%d", &courses[i].num_labs);
        courses[i].lab_ids = (int *)malloc(sizeof(int) * courses[i].num_labs);
        for (int j = 0; j < courses[i].num_labs; ++j)
        {
            scanf("%d", &courses[i].lab_ids[j]);
        }
        courses[i].ta_assigned = false;
        courses[i].ta_id = -1;
        courses[i].lab_id = -1;
        courses[i].num_students = 0;
        pthread_mutex_init(&courses[i].lock, NULL);
        pthread_cond_init(&courses[i].c, NULL);
        pthread_cond_init(&courses[i].v, NULL);
        courses[i].courseid = i;
    }
    students = (struct student *)malloc(sizeof(struct student) * num_students);
    for (int i = 0; i < num_students; ++i)
    {
        scanf("%f", &students[i].students_calibre);
        scanf("%d", &students[i].prefernece_1);
        scanf("%d", &students[i].prefernece_2);
        scanf("%d", &students[i].prefernece_3);
        scanf("%d", &students[i].time_registered);
        students[i].curr_pref = -1;
        pthread_mutex_init(&students[i].lock, NULL);
        students[i].studentid = i;
        students[i].active = 0;
    }
    labs_data = (struct lab *)malloc(sizeof(struct lab) * num_labs);
    for (int i = 0; i < num_labs; ++i)
    {
        scanf("%s", labs_data[i].name);
        scanf("%d", &labs_data[i].num_ta);
        scanf("%d", &labs_data[i].lab_limit);
        labs_data[i].ta_limit = (int *)malloc(sizeof(int) * labs_data[i].num_ta);
        labs_data[i].available = (bool *)malloc(sizeof(bool) * labs_data[i].num_ta);
        labs_data[i].all_limits_done = 0;
        for (int j = 0; j < labs_data[i].num_ta; ++j)
        {
            labs_data[i].ta_limit[j] = 0;
            labs_data[i].available[j] = 1;
        }
        pthread_mutex_init(&labs_data[i].lock, NULL);
    }

    for (int i = 0; i < num_students; ++i)
        pthread_create(&student_thread[i], NULL, student_init, (void *)&students[i]);

    for (int i = 0; i < num_courses; ++i)
        pthread_create(&course_thread[i], NULL, course_init, (void *)&courses[i]);

    for (int i = 0; i < num_students; ++i)
        pthread_join(student_thread[i],NULL);
    
    for (int i = 0; i < num_courses; ++i)
        pthread_join(student_thread[i],NULL);
    
}

void *course_init(void *course_deet)            //course threads
{
    struct course *course_details = (struct course *)course_deet;
    int exit_counter;         // to check if all tas of all labs are done
l1:
    exit_counter = 0;
    pthread_mutex_lock(&course_details->lock);

    if (course_details->ta_assigned == 0)
    {

        for (int i = 0; i < course_details->num_labs && course_details->ta_assigned == false; ++i) // finding a lab
        {
            int k = 0;
            
            pthread_mutex_lock(&labs_data[course_details->lab_ids[i]].lock);   //locking lab
            if (labs_data[course_details->lab_ids[i]].all_limits_done == true)
            {
                exit_counter++;
                pthread_mutex_unlock(&labs_data[course_details->lab_ids[i]].lock);
                continue;
            }

            for (int j = 0; j < labs_data[course_details->lab_ids[i]].num_ta; ++j)   //slecting ta from lab
            {

                if (labs_data[course_details->lab_ids[i]].available[j] == true)
                {
                    course_details->ta_id = j;
                    course_details->lab_id = i;
                    labs_data[course_details->lab_ids[i]].available[j] = false;
                    labs_data[course_details->lab_ids[i]].ta_limit[j] += 1;

                    printf("TA %d from lab %s has been allocated to course %s : taship number=%d\n", course_details->ta_id, labs_data[course_details->lab_id].name, course_details->name, labs_data[course_details->lab_ids[i]].ta_limit[j]);
                    course_details->ta_assigned = true;
                    break;
                }
                if (labs_data[course_details->lab_ids[i]].ta_limit[j] == labs_data[course_details->lab_ids[i]].lab_limit)
                    ++k;
            }
            if (k == labs_data[course_details->lab_id].num_ta)
            {
                printf("Lab %s no longer has students available for TAship and has been eliminated\n", labs_data[course_details->lab_id].name);
                labs_data[course_details->lab_id].all_limits_done = true;
            }
            pthread_mutex_unlock(&labs_data[course_details->lab_ids[i]].lock);
        }
        if (exit_counter == course_details->num_labs)
        {
            course_details->num_students = -1;
            pthread_cond_broadcast(&course_details->c);
            pthread_mutex_unlock(&course_details->lock);
            sleep(5);
            pthread_mutex_destroy(&course_details->lock);
            printf("The course %s doesn’t have any eligible students for TA ship available and is removed from the course offerings.\n",course_details->name);
            pthread_exit(NULL);
        }
    }
    course_details->slots = (rand() % course_details->max_students) + 1;  //random allocation of slots
    printf("Course %s has been allocated %d seats.\n", course_details->name, course_details->slots);
    pthread_cond_broadcast(&course_details->c);
    pthread_mutex_unlock(&course_details->lock);
    while (1)// checking if conditions for starting are met
    {
        if (course_details->num_students == course_details->slots)
        {
            printf("Tutorial has started for course %s with %d seats filled out of %d\n", course_details->name, course_details->slots, course_details->slots);
            break;
        }
        int x = 0;
        for (int i = 0; i < num_students; ++i)
        {
            if (students[i].curr_pref == course_details->courseid && students[i].active)
                ++x;
        }
        if (x == num_students)
        {
            printf("Tutorial has started for course %s with %d seats filled out of %d\n", course_details->name, course_details->num_students, course_details->slots);
            break;
        }
    }
    pthread_mutex_lock(&course_details->lock);
    sleep(10);
    course_details->ta_assigned = false;
    pthread_cond_broadcast(&course_details->v);
    pthread_mutex_unlock(&course_details->lock);
    printf("TA %d from lab %s has completed the tutorial for course %s\n", course_details->ta_id, labs_data[course_details->lab_id].name, course_details->name);
    
    course_details->num_students = 0;

    if (labs_data[course_details->lab_id].ta_limit[course_details->ta_id] == labs_data[course_details->lab_id].lab_limit)
        labs_data[course_details->lab_id].available[course_details->ta_id] = false;
    else
        labs_data[course_details->lab_id].available[course_details->ta_id] = true;
    goto l1;
}

void *student_init(void *student_deets)   //student threads
{

    struct student *student_details = (struct student *)student_deets;
    student_details->curr_pref = student_details->prefernece_1;
    sleep(student_details->time_registered);
    student_details->active = 1;
    // time_t curr_time = clock();

    // while ((curr_time - start) / CLOCKS_PER_SEC < student_details->time_registered)
    // {
    //     sleep(1);
    // }
    printf("Student %d has filled in preferences for course registration\n", student_details->studentid);
    while (1)
    {
        if (courses[student_details->curr_pref].num_students == -1)      // change preference is course is taken down from portal
        {
            if (student_details->curr_pref == student_details->prefernece_1)
            {
                student_details->curr_pref = student_details->prefernece_2;
                printf("Student %d has changed preference from %s(priority 1) to %s(priority 2)\n", student_details->studentid, courses[student_details->prefernece_1].name, courses[student_details->prefernece_2].name);

            }
            else if (student_details->curr_pref == student_details->prefernece_2)
            {
                student_details->curr_pref = student_details->prefernece_3;
                printf("Student %d has changed preference from %s(priority 2) to %s(priority 3)\n", student_details->studentid, courses[student_details->prefernece_2].name, courses[student_details->prefernece_3].name);
            }
            else if (student_details->curr_pref == student_details->prefernece_3)
            {
                printf("Student %d has exited the simulation without taking any course\n",student_details->studentid);
                pthread_exit(NULL);
            }
        }

        pthread_mutex_lock(&courses[student_details->curr_pref].lock);       //lock a course
        if (courses[student_details->curr_pref].num_students < courses[student_details->curr_pref].slots &&courses[student_details->curr_pref].ta_assigned)
        {
            courses[student_details->curr_pref].num_students++;
            printf("Student %d has been allocated seat in %s\n", student_details->studentid, courses[student_details->curr_pref].name);
            pthread_cond_wait(&courses[student_details->curr_pref].v, &courses[student_details->curr_pref].lock);
            pthread_mutex_unlock(&courses[student_details->curr_pref].lock);
            float prob = (float)courses[student_details->curr_pref].interest * (float)student_details->students_calibre;
            if (probability(prob) == 1)
            {
                printf("Student %d has selected course %s permanently\n", student_details->studentid, courses[student_details->curr_pref].name);
                pthread_exit(NULL);
            }
            else
            {
                printf("Student %d has withdrawn from course %s \n", student_details->studentid, courses[student_details->curr_pref].name);
                if (student_details->curr_pref == student_details->prefernece_1)
                {
                    student_details->curr_pref = student_details->prefernece_2;
                    printf("Student %d has changed preference from %s(priority 1) to %s(priority 2)\n", student_details->studentid, courses[student_details->prefernece_1].name, courses[student_details->prefernece_2].name);
                }
                else if (student_details->curr_pref == student_details->prefernece_2)
                {
                    student_details->curr_pref = student_details->prefernece_3;
                    printf("Student %d has changed preference from %s(priority 2) to %s(priority 3)\n", student_details->studentid, courses[student_details->prefernece_2].name, courses[student_details->prefernece_3].name);
                }
                else if (student_details->curr_pref == student_details->prefernece_3)
                {
                    printf("Student %d has exited the simulation without taking any course\n",student_details->studentid);
                    pthread_exit(NULL);
                }
            }
        }
        else
        {
            while(courses[student_details->curr_pref].ta_assigned==false&&courses[student_details->curr_pref].num_students>0)
            pthread_cond_wait(&courses[student_details->curr_pref].c, &courses[student_details->curr_pref].lock);
            pthread_mutex_unlock(&courses[student_details->curr_pref].lock);
            continue;
        }
    }
}

bool probability(float prob)
{
    int x = prob * 100;
    if (rand() % 100 + 1 <= prob)
        return 1;
    else
        return 0;
}