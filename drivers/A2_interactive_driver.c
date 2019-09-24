/*
 * CSC A48 - Assignment 2, BSTs
 * 
 * Interactive driver
 * 
 * This code is provided only to help you test and 
 * debug your code. You should use it while implementing
 * your solution to try different inputs, see your code
 * is doing the right thing, and find errors.
 * 
 * However, it does not constitute a replacement for
 * YOUR OWN TESTING while writing your solution. You're
 * expected to thoroughly test each function and each
 * part of your code.
 * 
 * (c) F. Estrada, M. Ahmadzadeh, 2019
 */

#include<string.h>
#include"BSTs.c"

void get_barindex(int *bar, double *index)
{
    printf("Please enter the bar:\n");
    scanf("%d",bar);
    getchar();
    printf("Please enter the index:\n");
    scanf("%lf",index);
    getchar();
}

int main()
{
    int choice, bar, semitones,i;
    double freq, index, time_shift;
    char note1[5], note2[5];
    BST_Node *root=NULL;
    BST_Node *new_note=NULL;
    BST_Node *t=NULL;
    char name[1024];
    char line[1024];
    FILE *f;
    
    read_note_table();		// Set up tables of note names and frequencies.
      
    choice=0;
    while (choice!=10)
    {
      printf("Please select from among the following options:\n");
      printf("0 - Insert new note\n");
      printf("1 - Search for note\n");
      printf("2 - Delete note\n");
      printf("3 - Print notes in order\n");
      printf("4 - Print notes in pre-order\n");
      printf("5 - Print notes in post-order\n");
      printf("6 - Read song from file\n");
      printf("7 - Make and run playlist\n");
      printf("8 - Shift notes\n");
      printf("9 - Harmonize\n");
      printf("10 - Delete BST and exit\n");
      
      scanf("%d",&choice);
      getchar();

      if (choice==0)
      {
	get_barindex(&bar, &index);
	printf("Enter the note's frequency\n");
	scanf("%lf",&freq);
	getchar();
	new_note=newBST_Node(freq,bar,index);
	root=BST_insert(root,new_note);
      }
      
      if (choice==1)
      {
	get_barindex(&bar, &index);
	t=BST_search(root,bar,index);
	if (t!=NULL)
	{
	  printf("Found note at (bar:index)=%d:%f, with frequency=%f Hz\n",bar,index,t->freq);
	}
	else
	{
	  printf("No such note found in the tree\n");
	}	
      }
      
      if (choice==2)
      {
	get_barindex(&bar, &index);
	root=BST_delete(root,bar,index);
      }
      
      if (choice==3)
	BST_inOrder(root,0);
      
      if (choice==4)
	BST_preOrder(root,0);
      
      if (choice==5)
	BST_postOrder(root,0);
      
      if (choice==6)
      {
	printf("Name of the song file (in .txt format)\n");
	fgets(&name[0],1024,stdin);
	i=strlen(&name[0]);
	if (name[i-1]=='\n') name[i-1]='\0';
	f=fopen(&name[0],"r");
	if (f==NULL)
	{
	  printf("Unable to open song file for reading. Please check name and path\n");
	}
	else
	{
	  while(fgets(&line[0],50,f))
	  {
	    sscanf(line,"%d %lf",&bar, &index);
	    i=strlen(line);
	    if (line[i-1]=='\n') line[i-1]='\0';
	    if (line[i-4]=='\t') strcpy(&note1[0],&line[i-3]);
	    else strcpy(&note1[0],&line[i-4]);
	    freq=-1;
	    for (i=0; i<100; i++)	      
	      if (strcmp(&note_names[i][0],note1)==0) {freq=note_freq[i]; break;}
	    printf("Read note from file with (bar:index)=%d:%f, name=%s, freq=%f\n",bar,index,note1,freq);

	    if (freq>0)
	    {
	      new_note=newBST_Node(freq,bar,index);
	      root=BST_insert(root,new_note);
	    }
	  }
	  fclose(f);
	}
      }
      
      if (choice==7)
      {
	BST_makePlayList(root);
	play_notes(2);
      }
      
      if (choice==8)
      {
	printf("Enter the name for the notes you want shifted\n");
	fgets(&note1[0],5,stdin);
	i=strlen(&note1[0]);
	if (note1[i-1]=='\n') note1[i-1]='\0';	// fgets() leaves the carriage return in the string! remove it!
	printf("Enter the name for the target notes\n");
	fgets(&note2[0],5,stdin);
	i=strlen(&note2[0]);
	if (note2[i-1]=='\n') note2[i-1]='\0';
	BST_shiftFreq(root,note1,note2);	
      }
      
      if (choice==9)
      {
	printf("Enter the number of semitones for note shifting\n");
	scanf("%d",&semitones);
	getchar();
	printf("Enter the time shift for each new note\n");
	scanf("%lf",&time_shift);
	root=BST_harmonize(root,semitones,time_shift);
      }
      
    }	// Enf while (choice!=10)

    delete_BST(root);
    return 0;
}