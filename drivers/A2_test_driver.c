/*
 * CSC A48 - Assignment 2 - BSTs test driver
 * 
 * This code is provided to help you test and debug your
 * solution for assignment 2.
 * 
 * The tests below should be commented and un-commented as
 * needed, while you're implementing different parts of 
 * your assignment (just like A1 before).
 * 
 * As you know by now:
 * 
 * - This is just a SUBSET of the tests we will run on
 *   your solution. You're responsible for coming up with
 *   your own test cases and making sure your code works
 *   for any input we can throw at it.
 * - Passing all of the tests here does not mean your code
 *   is bug free, it just means it's solid and you can
 *   expect to get a solid mark in the assignment
 * - You should read the tests carefully and understand what
 *   they are testing for, and think about why that is
 *   important and what additional tests you should be doing.
 * - FOR SOME OF THE TESTS YOU HAVE TO CHECK THE OUTPUT
 *   AND MAKE SURE IT'S CORRECT (e.g. tree traversals
 *   can not be auto-checked using only your code, our
 *   auto-marker will test those by comparing your output
 *   with the solution we have). So make sure you CHECK
 *   CAREFULLY for those tests that your code does what
 *   it should
 *      (This may also be a way for us to make sure
 *       you understand the material well ;) )
 * 
 * (c) 2019 - F. Estrada & M. Ahmadzadeh
 */

#include "BSTs.c"

int main()
{
    BST_Node *root=NULL;
    BST_Node *new_note=NULL;
    BST_Node *t=NULL;
    int bar,pass;
    double freq,index;
    
    // COMMENT AND UN-COMMENT THE TESTS BELOW AS NEEDED!
    
    // WHILE YOU'RE RUNNING THE TESTS BELOW - you should do ON PAPER the same
    // operations we're asking your BST to perform, so you know what SHOULD
    // be happening, and can find problems when they occur.
    
    // Test #1 - Creating and initializing a new node for the BST
    bar=2;
    index=.25;
    freq=440.0;
    new_note=newBST_Node(freq, bar, index);
    pass=1;
    if (new_note==NULL) pass=0;
    else
      if (new_note->freq!=freq || new_note->bar !=bar || new_note->index != index || new_note->key != (10.0*bar)+index) pass=0;
    if (pass==1)
      printf("Passed test #1\n");
    else
    {
      printf("Failed test #1 - creating a new node for the BST did not work as expected\n");
      return 0;
    }

    // Test #2 - Test inserting a couple nodes into the BST
    root=BST_insert(root,new_note);
    pass=1;
    if (root!=new_note)	pass=0;
    bar=2;
    index=0.0;
    freq=220.0;
    new_note=newBST_Node(freq,bar,index);    	// <- Where should this go in the tree?
    root=BST_insert(root,new_note);
    if (root->left!=new_note) pass=0;
    if (root->left->key!=(10.0*bar)+index) pass=0;
    bar=2;
    index=0.5;
    freq=880.0;
    new_note=newBST_Node(freq,bar,index);	// <- Where should this go in the tree?
    root=BST_insert(root,new_note);
    if (root->right!=new_note) pass=0;
    if (root->right->key!=(10.0*bar)+index) pass=0;
    if (pass==0)
    {
      printf("Failed test #2 - BST insert didn't work properly\n");
      return 0;
    }
    else printf("Passed test #2\n");
    
    // Test #3 - Search for specific nodes
    new_note=newBST_Node(110.0,2,.15);
    root=BST_insert(root,new_note);
    new_note=newBST_Node(55.0,1,.75);
    root=BST_insert(root,new_note);
    new_note=newBST_Node(330.0,1,.95);
    root=BST_insert(root,new_note);
    t=BST_search(root,1,.95);
    pass=1;
    if (t==NULL) pass=0;
    if (t->freq!=330.0) pass=0;
    if (pass==0)
    {
      printf("Failed test #3 - BST search did not find the node we just added to the tree\n");
      return 0;
    }
    else printf("Passed test #3\n");
    
    
    // Test #4 - Delete, case a) (node without children)
    root=BST_delete(root,1,.95);
    t=BST_search(root,1,.95);
    pass=1;
    if (t!=NULL) pass=0;
    t=BST_search(root,1,.75);
    if (t==NULL) pass=0;
    t=BST_search(root,2,.15);
    if (t==NULL) pass=0;
    root=BST_delete(root,2,.15);
    t=BST_search(root,2,.15);
    if (t!=NULL) pass=0;
    if (pass==0)
    {
      printf("Failed test #4 - BST delete, case a) something went wrong\n");
      return 0;
    }
    else printf("Passed test #4\n");

    // Test #5 - Delete, case b) (node with one child)
    t=BST_search(root,2,0.0);
    pass=1;
    if (t==NULL)
    {
      printf("Test #5 - Node with a note at bar=2, index=0.0 is not in the BST, it should be there!\n");
      return 0;
    }
    root=BST_delete(root,2,0.0);
    pass=1;
    t=BST_search(root,2,0.0);
    if (t!=NULL) pass=0;
    if (root->left->bar!=1 || root->left->index!=.75) pass=0;
    if (pass==0)
    {
      printf("Failed test #5 - BST delete case b) did not do the right thing\n");
      return 0;
    }
    else printf("Passed test #5\n");
    
    // Test #6 - Delete, case c) (node with 2 children)
    new_note=newBST_Node(125.0,2,.45);
    root=BST_insert(root,new_note);
    new_note=newBST_Node(175.0,2,.75);
    root=BST_insert(root,new_note);
    new_note=newBST_Node(100.0,2,.35);
    root=BST_insert(root,new_note);
    root=BST_delete(root,2,.25);	// Deletes the note at root!
    pass=1;
    t=BST_search(root,2,.25);
    if (t!=NULL) pass=0;
    if (root->bar!=2 || root->index != .35) pass=0;
    if (pass==0)
    {
      printf("Failed test #6 - BST delete case c) did not do the right thing\n");
      return 0;
    }
    else printf("Passed test #6\n");
    
    // Test #7 - In-Order Traversal
    // THIS TEST NEEDS TO BE CHECKED BY YOU, so make sure you have a diagram ON PAPER
    // of what the tree should contain up to this point, given all the operations
    // above. 
    // You then have to check that the functions print out the right information
    // that means: The nodes in the tree in the correct order given how the traversal
    // works.
    printf("\n*** Checking in-Order traversal\n");
    BST_inOrder(root,0);
    
    // Test #8 - pre-Order Traversal
    printf("\n*** Checking pre-Order traversal\n");
    BST_preOrder(root,0);
    
    // Test #9 - post-Order Traversal
    printf("\n*** Checking post-Order traversal\n");
    BST_postOrder(root,0);
    
    // The tests above check the basic functionality of your BST. The crunchy functions
    // (shift frequency, make playlist, and harmonize) are better tested from the
    // interactive test loop (use any of the pre-defined test inputs). 
    // The file called 'BST_driver_input.txt' will load a sequence of notes that
    // go from low-pitch to high-pitch when turned into a list and played back, try
    // it out. When printed in order, you should see one note per bar, with increasing
    // frequencies.
    
    // Be sure to test out the functionality of your crunchy functions thoroughly
    // before submitting!
    
    return 0;
}
  
