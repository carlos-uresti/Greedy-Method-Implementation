
//Carlos Uresti ID 1001838308
// 3318 Lab 2
//lab successfully sorts and prints matching outcomes for first set of data in course website
//compiled using, gcc lab2.c then ./a.out < testdata.txt

#include <stdio.h>
#include <stdlib.h>


//this struct will track the wash time and dry time for each basket
//as well as the start and end for each basket
//it will also assign each basket an index number as well
//as an in pointer to track if the basket has been used 
struct Basket
{
    int washer_start;
    int washer_end;
    int dryer_start;
    int dryer_end;
    int finish;
    int wash;
    int dry;
    int wait_time;
    int index;
    int *used;
    
};



int main()
{

 
    int cmp();//sorts baskets by index value
    int cmp1();//sorts baskets by wash time
    int cmp2();//sorts baskets by dry tim
    int binary[1] = { 1 };//used to mark when a basket had been used
    int i, j, k; //counters
    int m;//array size value

    //number of baskets as well number of element of struct array 
    scanf("%d", &m);
    
    
    //baskets arrays to hold baskest and its sorted variations
    struct Basket baskets[m];
    struct Basket sorted_by_wash_time[m];
    struct Basket sorted_by_dry_time[m];
    struct Basket optimal[m];

 
    
    //scan input from command line
    for (j = 0; j < m; j++)
    {
        scanf("%d %d", &baskets[j].wash, &baskets[j].dry);
        baskets[j].index = j;
    }
   
    //sort both arrays in ascending order by wash time using qsort
    qsort(baskets, m, sizeof(struct Basket), cmp1);

    //after sorting initial basket by wash time, assign to new array
    for (j = 0; j < m; j++)
    {   
        sorted_by_wash_time[j] = baskets[j];
    
    }

    //sort both arrays in ascending order by dry time using qsort
    qsort(baskets, m, sizeof(struct Basket), cmp2);
    
    //after sorting by wash time, assing to new array
    for (j = 0; j < m; j++)
    {
        sorted_by_dry_time[j] = baskets[j];
        

    }

    //resort initial basket by index value
    qsort(baskets, m, sizeof(struct Basket), cmp);

    j = 0;//loop counter
    k = m;//counter for end of optimal array
    int p = 0;//counter for starrt of optimal array
    int counter = 0;//will track when to check elements at very end of array

    while (j < k)
    {   
       
        //compare first two items of arrays sorted by wash and dry time, element that satisfies condition goes at end of array
        if (sorted_by_wash_time[j].wash > sorted_by_dry_time[j].dry)
        {
           //if the values compared are not the same basket enter here
            if (sorted_by_wash_time[j].index != sorted_by_dry_time[j].index)
            {
                //mark every compared element and place in correct spot of final array
                //increase all counterse
                sorted_by_wash_time[j].used =  binary;
                sorted_by_dry_time[j].used = binary;
                optimal[k - 1] = sorted_by_dry_time[j];
                optimal[p] = sorted_by_wash_time[j];
                k--;
                j++;
                p++;
                counter +=2;
               
            }


        }//compare first two items of arrays sorted by wash and dry time, element that satisfies condition goes at start of array
        else if(sorted_by_wash_time[j].wash <= sorted_by_dry_time[j].dry)
        {
            //mark every compared element and place in correct spot of final array
                //increase all counterse
            if (sorted_by_wash_time[j].index != sorted_by_dry_time[j].index)
            {
                
                sorted_by_wash_time[j].used = binary;
                sorted_by_dry_time[j].used = binary;
                optimal[p] = sorted_by_wash_time[j];
                optimal[k - 1] = sorted_by_dry_time[j];
                p++;
                j++;
                k--;
                counter+=2;
                
            }
        }//if the values compared are not the same basket enter here
        if (sorted_by_wash_time[j].index == sorted_by_dry_time[j].index)
        {   
            
            //when you find a matching index, assign to either end and continue
            sorted_by_wash_time[j].used = binary;
            sorted_by_dry_time[j].used = binary;
            optimal[k-1] = sorted_by_wash_time[j];
            k--;
            j++;
            counter++;

        }//test that last item in arrays used for comparison has been used
        if (counter == m)
        {
            
            if ((sorted_by_wash_time[m-1].used == binary) && (sorted_by_dry_time[m - 1].used == binary) )
            {

              
                if (sorted_by_wash_time[m - 1].wash < sorted_by_dry_time[m - 1].dry)
                {
                    optimal[p - 1] = sorted_by_wash_time[m - 1];
                }
                if (sorted_by_wash_time[m - 1].wash > sorted_by_dry_time[m - 1].dry)
                {
                    optimal[p - 1] = sorted_by_dry_time[m - 1];
                }
                
            }
        }
     
      
    }

    //once optimal array is sorted, calculate washer start and dryer start time
    j = 0;
    optimal[j].washer_start = j;
    optimal[j].dryer_start= optimal[j].wash - optimal[j].washer_start; 

    //print the contents of each basket
    printf("index: %d wash time: %d dry time: %d washer start: %d dryer start: %d\n", optimal[j].index, optimal[j].wash, optimal[j].dry, optimal[j].washer_start, optimal[j].dryer_start);
    for (j = 1; j < m; j++)
    {
        optimal[j].washer_start = optimal[j - 1].wash + optimal[j - 1].washer_start;
        optimal[j].dryer_start = optimal[j - 1].dryer_start + optimal[j - 1].dry;
        printf("index: %d wash time: %d dry time: %d washer start: %d dryer start: %d\n", optimal[j].index, optimal[j].wash, optimal[j].dry, optimal[j].washer_start, optimal[j].dryer_start);
    
    }///calculate the makespan by adding total wash time plus last dryer time
    ////or by adding total dryer time
    //whichever of the two is larger becomes makespan
    int makespan = 0;
    for (j = 0; j < m; j++)
    {
        makespan += optimal[j].wash;
    }
    makespan += optimal[m - 1].dry;

    int makespan1 = 0;
    for (j = 0; j < m; j++)
    {
        makespan1 += optimal[j].dry;
    }
    int makespan2 = 0;
    if (makespan > makespan1) 
    {
        makespan2 = makespan;
    }
    else 
    {
        makespan2 = makespan1;
    }


    printf("makespan is: %d\n", makespan2);
                                          
}

//this function will sort baskets by original index
int cmp(void const* a, void const* b)
{
    struct Basket* BasketA = (struct Basket*)a;
    struct Basket* BasketB = (struct Basket*)b;


    return(BasketA->index - BasketB->index);
}

//will sort basked structs by ascending wash time
int cmp1(void const* a, void const* b)
{
    struct Basket *BasketA = ( struct Basket*)a;
    struct Basket *BasketB = ( struct Basket*)b;

    
    return(BasketA->wash - BasketB->wash);
}

//will sort basked structs by ascending dry time
int cmp2(void const* a, void const* b)
{
    struct Basket* BasketA = (struct Basket*)a;
    struct Basket* BasketB = (struct Basket*)b;


    return(BasketA->dry - BasketB->dry);
}

