
#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>
#include <sys/stat.h>
#include <time.h>



#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                
#define MAX_COMMAND_SIZE 255    // The maximum command-line size
#define MAX_NUM_ARGUMENTS 5     // Mav shell only supports five arguments
#define MAX_CHAR_FILENAME 32    //max characters for file names

#define NUM_BLOCKS 4226 //based on requirements
#define BLOCK_SIZE 8192 //based on requirements
#define TOTAL_MEM NUM_BLOCKS*BLOCK_SIZE //total memory of system


uint8_t blocks[NUM_BLOCKS][BLOCK_SIZE]; //based on requiremnts
uint32_t totalMemFree = TOTAL_MEM;  //starting memory - all free

//to aid in creating/saving filesystem image
char currDir;
FILE *curr; 
FILE *ofptr;//open file pointer
FILE *fsptr;
char nameTracker[40];

//File system directory structure
struct _DirEntry
{
  char name[MAX_CHAR_FILENAME];//file name
  uint32_t inode;              //inode #
  uint8_t valid;               //whether in use or not

};
//File system inode structure
struct _Inode
{
  uint32_t data_blocks[1250];//actual data location

  struct tm * timeinfo; //time created
  char saveTime[40];
  time_t createTime;
  
  uint32_t fileSize;
  uint8_t valid; // whether in use or not

  //Attribute value key:
  //0 can write + not hidden
  //1 can write + hidden
  //2 read only + not hidden
  //3 read only + hidden

  int attribute;

};

int* free_inode_map;
int* free_block_map;


int block_index = 0;
//1.10 Directory stored in first (block 0) disk block
struct _DirEntry *dir = (struct _DirEntry* )&blocks[0][0];
struct _Inode *inode = (struct _Inode* )&blocks[3][0];

/*
Function intializes the structures
and marks all blocks as valid initially
*/
static void initializer()
{

  uint32_t memory = totalMemFree;


  //based on requirements 1.10 through 1.13

  //1.12 use block 1 for the inode map
  free_inode_map = (int*)&blocks[1][0];
  
  //1.13 use block 2 for the free block map
  free_block_map = (int*)&blocks[2][0];

  int i;
  //1.11 requirement is that blocks 3-131 be used for i nodes 
  //saves blocks 0, 1 and 2/ starts initializing Inodes at spot 3  
  for(i = 3; i < 131; i++)// 
  {
    free_inode_map[i] = 1;//free
  }
  for(i = 132; i < 4225; i++)//the remainder of blocks are used for data 
  {
    free_block_map[i] = 1;//free
  }
  //initializing all directories as valid - can be used
  for(i = 0; i < 128; i++){
    dir[i].valid = 1; 
  }
  //1.14 this is now a single level hierarchy with no subdirectories
}

/***************************************list**********************************************/
/*
Function lists all files in system 
no paramerters needed
*/
void listCommand()
{
  int i;
  int found = 0;
  for(i = 0; i < 128; i++){
    if((dir[i].valid == 0) && (dir[i].inode != -1) 
    && (inode[i].attribute != 1) && (inode[i].attribute != 3)) //hidden files
    {   
      printf("FileSize: %d bytes %s FileName: %s\n", 
      inode[i].fileSize, inode[i].saveTime, dir[i].name);
      found = 1;
    }
  }
  if(found == 0){
    printf("No files found\n");
  }

}

/******************************************get*******************************************/
/*
Function gets file from system and makes a copy eith rewrites
or makes a new copy

Parameters:
char* token1: name of the file we will get
char* token2: name of output file
*/
static void getCommand(char* token1, char* token2)
{

  int    status;// Hold the status of all return values.
  struct stat buf;
  status =  stat(token1, &buf ); 

  int found  = 0;//file name match not found
  int i = 0;

  while(i < 128)
  {//TODO change 128 to var
    if(strcmp(token1, dir[i].name) == 0)//filename in dir
    {
      found = 1;
      //printf("'%s' file name found \n", token1);
      break;
    }
    i++;
  }

  if(found == 0)
  {
    printf("get error: file not found\n");
  }

  //github code
  if(found != 0) 
  {
    FILE *ofp;
    ofp = fopen(token2, "w");

    if( ofp == NULL )
    {
      printf("Could not open output file: %s\n", token2);
      perror("Opening output file returned");
      exit(0);
    }

    int index = i; //index of match at dir
    int inode_no = dir[i].inode;  //inode # of file to get
    int copy_size  = inode[inode_no].fileSize; //file size being copied
    int data_pos = inode[inode_no].data_blocks[inode_no]; //where actual daa is stored
    
    
    int offset = 0;
    //from github
    while( copy_size > 0 )
    { 

      int num_bytes;
      if( copy_size < BLOCK_SIZE )
      {
        num_bytes = copy_size;
      }
      else 
      {
        num_bytes = BLOCK_SIZE;
      }
      fwrite(blocks[data_pos], num_bytes, 1, ofp );
      copy_size -= BLOCK_SIZE;
      offset    += BLOCK_SIZE;
      block_index ++;
      fseek( ofp, offset, SEEK_SET );
    }

    // Close the output file, we're done. 
    fclose( ofp );

  }
  

}
/**************************************del***********************************************/
/*
Function deletes a file
Parameters:
char* token1 = name of file to be deleted
*/
static void delCommand(char* token1)
{
  int found  = 0;//file name match not found
  int i = 0;

  while(i < 128)
  {//TODO change 128 to var
  
    if(strcmp(token1, dir[i].name) == 0)//filename in dir
    {
      found = 1;
      break;
    }
    i++;
  }

  if(found == 0)
  {
    printf("del error: file not found\n");
  }
  if(inode[i].attribute == 2 || inode[i].attribute == 3){//read only attributes
    printf("This file is marked read-only.\n");
  }
  else{

    totalMemFree = totalMemFree + inode[i].fileSize;
    memset(inode[i].data_blocks, 0, inode[i].fileSize);//go to the i node being deleted, where the pointer for data_blocks begins
    dir[i].valid = 1;                                     //set NULL to all bytes that make up the fileSize, if NULL does not work, try 0
    strcpy(dir[i].name, "");                              
    dir[i].inode = -1; 
    free_block_map[i] = 1;//will allow something else to write in freed space
    free_inode_map[i] = 1;//will allow something else to write in freed space
    
    inode[i].data_blocks[i] = 0; //reset address
    inode[i].fileSize = -1;
    
  }
  
  

}

/****************************************df*********************************************/
/*
Function lists the amount of space left in file
*/
static void dfCommand()
{
  printf("%d bytes free\n",totalMemFree);
  
}

/*************************************attrib*****************************************/
/*
Function adds or removes hidden and read attributes from file
Parameters:
char* token1 = attribute we want to add or remove
char* token2 = file name
*/
static void attribCommand(char* token1, char* token2)
{
  //token 2 is file name //token 1 is attribute name
  int found  = 0;//file name match not found
  int i = 0;

  while(i < 128)
  {//TODO change 128 to var
    if(strcmp(token2, dir[i].name) == 0)//filename in dir
    {
      found = 1;
      break;
    }
    i++;
  }

  if(found == 0)
  {
    printf("attrib error: file not found\n");
  }

  //check if 2 attrubutes present at once
  if(strcmp(token1, "+h") == 0)
  {
    if((inode[i].attribute == 2) || (inode[i].attribute == 3))//read only + not hidden or hidden
    {
      inode[i].attribute = 3; //read only + hidden
    }
    else
    { 
      inode[i].attribute = 1; //can write + hidden
    }
  }
  if(strcmp(token1, "-h") == 0)
  {
    if((inode[i].attribute == 2) || (inode[i].attribute == 3))//read only + not hidden or hidden
    {
      inode[i].attribute = 2; //read only + not hidden
    }
    else
    { 
      inode[i].attribute = 0; //can write + not hidden
    }
  }
  if(strcmp(token1, "+r") == 0)
  {

    if((inode[i].attribute == 1) || (inode[i].attribute == 3))
    //write/read only + hidden
    {
      inode[i].attribute = 3; //read only + hidden
    }
    else
    { 
      inode[i].attribute = 2; //read only + not hidden
    }
  }
  if(strcmp(token1, "-r") == 0)
  {
    if((inode[i].attribute == 1) || (inode[i].attribute == 3))
    //write/read only + hidden
    {
      inode[i].attribute = 1; //read/write + hidden
    }
    else
    { 
      inode[i].attribute = 0; //read/write+ not hidden
    }
  }

}

/***************************createfs**************************************************/
/*
Creates a file
Parameter:
char* token1 = file name
*/
static void createfsCommand(char* token1)
{
  //df value rests here
  totalMemFree = TOTAL_MEM;
  strcpy(nameTracker, token1);
  //printf("%s\n", nameTracker);
  curr = fopen(token1, "w");
  

}


/***************************************savefs******************************************/
/*
Function saves file
*/
static void savefsCommand()
{
  
  int i =0;
   while(i < 128)
  {//TODO change 128 to var
    if(strcmp(nameTracker, dir[i].name) == 0)//filename in dir
    {
      
      printf("'%s' file name found \n", nameTracker);
      break;
    }
    i++;
  }
  time(&inode[i].createTime);//save time
  strcpy(inode[dir[i].inode].saveTime, ctime(&inode[i].createTime));
 

  FILE *sfsptr;//save file pointer
  
  //open file where data  will be saved, open in write mode
  sfsptr = fopen(nameTracker, "w");
  
  //copy blocks to path specified by user
  fwrite(blocks, sizeof(uint8_t), NUM_BLOCKS*BLOCK_SIZE, sfsptr);

}


/**************************************close********************************************/
/*
Closes a file - takes in file pointer as input
*/
static void closeCommand(FILE *ofptr)
{
  fclose(ofptr);
}

/***********************************put************************************************/
/*
Function puts a file into system
param:
char* token1 = file name
*/
static void putCommand(char* token1)
{
  //TODO check if there is space in the disk

  int    status; // Hold the status of all return values.
  struct stat buf; // stat struct to hold returns from stat call

  //token1 is file name supplied by the user
  status =  stat(token1, &buf ); 

  //check if in valid state and max file name is less than 32 chars
  if( (status != -1) && strlen(token1) < MAX_CHAR_FILENAME)
  {

    if(dir[block_index].valid == 1)
    {
      
      //--change directory struct to show new file info--
      dir[block_index].valid = 0; //not free anymore
      strcpy(dir[block_index].name , token1); //assign name
      //figure out what to set inode to
      dir[block_index].inode = block_index;
      //printf("In directory: inode # saved as: %d\n",dir[block_index].inode);

      //--change freeblock map to show new file info--
      free_block_map[block_index] = 0;//not free

      //--change freeinode map to show new file info--
      free_inode_map[block_index] = 0;//not free


      //--change inode struct to show new file info--
      //inode[block_index].blocks[block_index] = block_index+132;//data starts
      
      
      inode[block_index].fileSize =  buf . st_size;
      inode[block_index].valid = 0;//not free
      inode[block_index].attribute = 0; //can write + not hidden
      

      totalMemFree = totalMemFree - inode[block_index].fileSize;
      if(block_index == 0){
        inode[block_index].data_blocks[block_index] = block_index+132;
      }
      else{
        inode[block_index].data_blocks[block_index] = 132 + inode[block_index -1].fileSize;
      }
      
      //printf("inode: data starts at: %d\n", inode[block_index].data_blocks[block_index]);

      //start saving file data at 132 after inodes

      //save the file in blocks
      // Open the input file read-only 
      FILE *ifp = fopen (token1, "r" ); 
      //printf("Reading %d bytes from %s\n", (int) buf . st_size, token1);

      int copy_size   = buf . st_size;
      int offset      = 0;    
      while( copy_size > 0 )
      {
        fseek( ifp, offset, SEEK_SET );
        //+132 starting index of data
        int bytes  = fread(blocks[block_index+132], BLOCK_SIZE, 1, ifp );
        if( bytes == 0 && !feof( ifp ) )
        {
          printf("An error occured reading from the input file.\n");
          exit(0);
        }
        clearerr( ifp );
        copy_size -= BLOCK_SIZE;
        offset    += BLOCK_SIZE;

        // Increment the index into the block array 
        block_index ++;

      }
      fclose( ifp );
    }
    
  }
  
  
}
/*********************************quit*******************************************/
//funtion to quit
static int quitCommand()
{
  return 0;
}

/*********************************main*******************************************/
int main()
{
  //initialize all structs
  initializer();
  //github code
  char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );
   
  while( 1 )
  {
    // Print out the mfs prompt
    printf ("mfs> ");

    while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );

  
    
    if((strncmp(cmd_str, "exit", strlen("exit")) == 0) || 
    strncmp(cmd_str, "quit", strlen("quit")) == 0){
      break;
    }
    
    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];
    
    int   token_count = 0;                                 
                                                          
    // Pointer to point to the token
    // parsed by strsep
    char *argument_ptr;                                                   
    char *working_str  = strdup( cmd_str );

              

    // we are going to move the working_str pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *working_root = working_str;
      
    // Tokenize the input strings with whitespace used as the delimiter
    while ( ( (argument_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) && 
              (token_count<MAX_NUM_ARGUMENTS))
    {
      
      token[token_count] = strndup( argument_ptr, MAX_COMMAND_SIZE );
      if( strlen( token[token_count] ) == 0 )
      {
        token[token_count] = NULL;
      }
        token_count++;
    }

    
    if(token[0] != NULL)
    {
      //open command
      if(strcmp(token[0], "open") == 0 && token[1] != NULL)
      {   
          //curr = ofptr;
          currDir = *token[1];
          //open file where you will be extracting data from
          curr = fopen(token[1], "rw");
          //save the file image to blocks
          fread(blocks, sizeof(uint8_t), NUM_BLOCKS*BLOCK_SIZE, curr);
      }
      if(strcmp(token[0], "open") == 0  && token[1] == NULL )
      {
        printf("open: File not found\n");
      }
      //put command
      if((strcmp(token[0], "put") == 0) && (token[1] != NULL))
      {
        putCommand(token[1]);
      }
      //list command
      if(strcmp(token[0], "list") == 0)
      {
        listCommand();
      }
      if(strcmp(token[0], "df") == 0)
      {
        dfCommand();
      }
      //get command
      if(strcmp(token[0], "get") == 0)
      {
        if(token[2] != NULL)
        {
          getCommand(token[1], token[2]);
        } 
        else
        {
          getCommand(token[1], token[1]);
        }
      }
      //del command
      if(strcmp(token[0], "del") == 0 && (token[1] != NULL))
      {
        delCommand(token[1]);
      }
      //attrib command
      if(strcmp(token[0], "attrib") == 0 && (token[1] != NULL) && 
      (token[2]) != NULL){
        attribCommand(token[1], token[2]);
      }
      //createfs command
      if(strcmp(token[0], "createfs") == 0 && (token[1] != NULL ))
      {   
        currDir = *token[1];
        createfsCommand(token[1]);             
      }
      if(token[1] == NULL && strcmp(token[0], "createfs") == 0 )
      {
        printf("createfs: File not found\n");
      }
      //savefs command
      if(strcmp(token[0], "savefs") == 0)
      {
        //has to manually retrieve name of directory
        savefsCommand();
      }
      //close command
      if(strcmp(token[0], "close") == 0 && token[1] != NULL)
      {
        closeCommand(curr);
      }
      //quit command
      if(strcmp(token[0], "quit"))
      {  
        quitCommand();
      }
    }
    free( working_root );
   
  }
  return 0;
}
