#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

struct set{
    struct set* next;
    int length;
    int valid;
    long int data;
};

void add_line(struct set** whole_set, int set, long int data){
    struct set* insert = (struct set*)malloc(sizeof(struct set));
    
    insert->data = data;
    insert->length = whole_set[set]->length;
    insert->next = NULL;
    insert->valid = 1;
    //case where hasn't been touched yet
    if(whole_set[set]->valid == 0){
        whole_set[set]->data = data;
        whole_set[set]->valid = 1;
	    free(insert);
        return;
    }
    struct set* current = whole_set[set];
    while(current->next!=NULL){
        current = current->next;
    }
   
    current->next = insert;
}

int power(int n){
    return 1 << n;
}

int ln(int n){
    int i = 0;
    while(n>1){
        n = n>>1;
        i+=1;   
    }return i;
}

int ispowerof2(int number){
    for(int i=0;i<100;i++){
        if(power(i) == number){
            return 1;
        }
    }
    return 0;
}

int room_in_set(struct set**whole_set, int set){
    struct set* temp = whole_set[set];
    if(whole_set[set]->valid == 0){
        return 1;
    }
    int length = temp->length;
    int counter = 1;
    while(temp->next!=NULL){  
        
        counter+=1;
        temp = temp->next;
    }
    if(length>counter){
        return 1;
    }else{
        return 0;
    }
}

void replace_FIFO(struct set** whole_set, int set, long int data){
    //case that there is only one thing in the set
    if(whole_set[set]->next == NULL){
        whole_set[set]->data = data;
    }else{
	    struct set* temp = whole_set[set];
        whole_set[set] = whole_set[set]->next;
	    free(temp);
        add_line(whole_set, set, data);
    }
}


void read(struct set** whole_set, int block_size, int set_size, long int data, int* ptr){
    
    //removing block offset

    long int setmask = (1 << set_size) -1;
    data = data >> block_size;
    
    long int set = data & setmask;
    long int tag = data >> set_size;
  
    for(struct set* current =  whole_set[set];current!=NULL;current = current->next){
        if(current->data == tag){
           
            //Cache HIT Return
            ptr[0] +=1;
            
            
            return;
        }
    }
        // cache_miss + memory_read
    ptr[1] += 1;
    ptr[2] += 1;
 
    if(room_in_set(whole_set, set)){
        
        
        add_line(whole_set, set, tag);
    }else{
        
        replace_FIFO(whole_set, set, tag);
    }
    
    if(ptr[4] ==1){
        data += 1;
        set = data & setmask;
        tag = data >> set_size;
        //Checks if the next block is already in a set
        for(struct set* current =  whole_set[set];current!=NULL;current = current->next){
            if(current->data == tag){
            
                return;
            }
        }
        //Incrementing memory reads and setting y in cache for prefetching
        ptr[2] += 1;
        if(room_in_set(whole_set, set)){
            add_line(whole_set, set, tag);
        }else{
            replace_FIFO(whole_set, set, tag);
        }
    }
    
}

void write(struct set** whole_set, int block_size, int set_size, long int data, int* ptr){

    long int setmask = (1 << set_size) - 1;
    data = data >> block_size;
    
    long int set = data & setmask;
    long int tag = data >> set_size;
 
    
    for(struct set* current =  whole_set[set];current!=NULL;current = current->next){
        if(current->data == tag){
            //Cache HIT Return
            ptr[0] +=1;
	        ptr[3] +=1;
         
            return;
        }
    }
    //Cache MISS + memory read + memory_write
    ptr[1] += 1;
    ptr[2] += 1;
    ptr[3] += 1;
    
    //ADD TO LIST
    if(room_in_set(whole_set, set)){
        
        
        add_line(whole_set, set, tag);
    }else{
       
        replace_FIFO(whole_set, set, tag);
    }
    
    
    //IF miss then prefetch
    if(ptr[4] ==1){
        data += 1;
        set = data & setmask;
        tag = data >> set_size;
        //Checks if the next block is already in a set
        for(struct set* current =  whole_set[set];current!=NULL;current = current->next){
            if(current->data == tag){
                
                return;
            }
        }
        //Incrementing memory reads and setting y in cache
        ptr[2] += 1;
        if(room_in_set(whole_set, set)){
            add_line(whole_set, set, tag);
        }else{
            replace_FIFO(whole_set, set, tag);
        }
    }
    
    
}

void printList(struct set** whole_set, int set){
    for(int i=0;i<set;i++){
    
        for(struct set* current = whole_set[i]; current != NULL; current = current->next){
            printf("SET %d, data %lx , ", i, current->data);
        }
        printf("   :   ");
    }
    printf("\n\n");
}


int main(int argc, char *argv[]){
    
    
    //READING INPUT
    int cache_size, associativity, block_size, associativity_n, set, lines_per_set, block_offset_size, set_offset_size;
    cache_size = atoi(argv[1]);
    block_size = atoi(argv[4]);
    block_offset_size = ln(block_size);

    //Reading INPUTS
    if(!strcmp(argv[2], "direct")){
        associativity = 0;
    }else if(!strcmp(argv[2], "assoc")){
        associativity = 1;
    }else{
        associativity = 2;
        associativity_n = atoi(&argv[2][6]);
    }

    if(!strcmp(argv[3], "fifo")){
        //replacement_policy = 0;
    }else if(!strcmp(argv[3], "lru")){
        //replacement_policy = 1;
    }
    //Defining Variables

    if(associativity == 0){
        set = cache_size/block_size;
        lines_per_set = 1;
    }else if(associativity == 1){
        set = 1;
        lines_per_set = cache_size/block_size;
    }else{
        set = cache_size/(block_size * associativity_n);
        lines_per_set = associativity_n;
    }
    
    
    set_offset_size = ln(set);
    if(ispowerof2(cache_size) == 0){
        printf("error\n");
        return 0;
    }
    if(ispowerof2(block_size) == 0){
        printf("error\n");
        return 0;
    }
    
    //Creating the set
    struct set** whole_set = malloc(sizeof(struct set*) * set);
    for(int i=0;i<set;i++){
        whole_set[i] = malloc(sizeof(struct set));
        whole_set[i]->length = lines_per_set;
        whole_set[i]->next = NULL;
        whole_set[i]->data = -1;
        whole_set[i]->valid = 0;
    }

    //Reading TRACE
    //To discard large hex, %*x, to read hex and store as long %lx

    int* ptr = malloc(sizeof(int) * 5);
    for(int i =0;i<5;i++){
        ptr[i] = 0;
    }

    FILE *trace = fopen(argv[5], "r");
    char type[6];
    long int data;

    while(1){
    
        fscanf(trace, "%*x");  
        fscanf(trace, "%s", type);
        if(strcmp(type, "#eof") == 0){
            break;
        }
        fscanf(trace, "%s", type);
        
        fscanf(trace, "%lx", &data);
        
        if(strcmp(type, "R") == 0){
            read(whole_set, block_offset_size, set_offset_size, data, ptr);
        }else if(strcmp(type, "W") == 0){
            write(whole_set, block_offset_size, set_offset_size, data, ptr);
        }
       
        
    }
    fclose(trace);
    printf("Prefetch %d\n", ptr[4]);
    printf("Memory reads: %d\n", ptr[2]);
    printf("Memory writes: %d\n", ptr[3]);
    printf("Cache hits: %d\n", ptr[0]);
    printf("Cache misses: %d\n", ptr[1]);
    


    for(int i=0; i<set;i++){
        struct set* temp = whole_set[i];
        while(whole_set[i] != NULL){
            temp = whole_set[i];
            whole_set[i] = whole_set[i]->next;
            free(temp);
        }
    }
    free(whole_set);
    
    //Now for the prefetching
    
    ptr[0] = 0;
    ptr[1] = 0;
    ptr[2] = 0;
    ptr[3] = 0;
    ptr[4] = 1;


    struct set** whole_set2 = malloc(sizeof(struct set*) * set);
    for(int i=0;i<set;i++){
        whole_set2[i] = malloc(sizeof(struct set));
        whole_set2[i]->length = lines_per_set;
        whole_set2[i]->next = NULL;
        whole_set2[i]->data = -1;
        whole_set2[i]->valid = 0;
    }
    
    FILE *trace2 = fopen(argv[5], "r");
    while(1){
        fscanf(trace2, "%*x");  
        fscanf(trace2, "%s", type);
        
        if(strcmp(type, "#eof") == 0){
            break;
        }
        fscanf(trace2, "%s", type);
        
        fscanf(trace2, "%lx", &data);
        
        if(strcmp(type, "R") == 0){
            read(whole_set2, block_offset_size, set_offset_size, data, ptr);
        }else if(strcmp(type, "W") == 0){
            write(whole_set2, block_offset_size, set_offset_size,  data, ptr);
        }
        
    }
    printf("Prefetch %d\n", ptr[4]);
    printf("Memory reads: %d\n", ptr[2]);
    printf("Memory writes: %d\n", ptr[3]);
    printf("Cache hits: %d\n", ptr[0]);
    printf("Cache misses: %d\n", ptr[1]);
    fclose(trace2);
    //printList(whole_set2, set);
   


    //FReeing Memory
    for(int i=0; i<set;i++){
        struct set* temp = whole_set2[i];
        while(whole_set2[i] != NULL){
            temp = whole_set2[i];
            whole_set2[i] = whole_set2[i]->next;
            free(temp);
        }
    }
    free(whole_set2);
    free(ptr);
    
}
