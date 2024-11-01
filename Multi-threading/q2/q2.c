#include <stdio.h>
#include <stdbool.h>
#include<time.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>


int capacity_A,capacity_B,capacity_C,num_groups;


struct person
{
    char c;
    int t;
    int patience;
    int goals;
};
struct person ** persons;


int main()
{
    scanf("%d%d%d%d",&capacity_A,&capacity_B,&capacity_C,&num_groups);
    persons=(struct person**)malloc(num_groups*sizeof(struct person*));
    for(int i=0;i<num_groups;++i)
    {
        int k;
        scanf("%d"&k);
        persons[i]=(struct person*)malloc(k*sizeof(struct person));
        for(int j=0;j<k;++j)
        {
           scanf("%c",persons[i][j].c);
           
        }
    }

}