#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <CUnit/Basic.h>
#include "beargit.h"
#include "util.h"

/* printf/fprintf calls in this tester will NOT go to file. */

#undef printf
#undef fprintf

/* The suite initialization function.
 * You'll probably want to delete any leftover files in .beargit from previous
 * tests, along with the .beargit directory itself.
 *
 * You'll most likely be able to share this across suites.
 */
int init_suite(void)
{
    // preps to run tests by deleting the .beargit directory if it exists
    fs_force_rm_beargit_dir();
    unlink("TEST_STDOUT");
    unlink("TEST_STDERR");
    return 0;
}

/* You can also delete leftover files after a test suite runs, but there's
 * no need to duplicate code between this and init_suite 
 */
int clean_suite(void)
{
    return 0;
}

/* Simple test of fread().
 * Reads the data previously written by testFPRINTF()
 * and checks whether the expected characters are present.
 * Must be run after testFPRINTF().
 */
void simple_sample_test(void)
{
    // This is a very basic test. Your tests should likely do more than this.
    // We suggest checking the outputs of printfs/fprintfs to both stdout
    // and stderr. To make this convenient for you, the tester replaces
    // printf and fprintf with copies that write data to a file for you
    // to access. To access all output written to stdout, you can read 
    // from the "TEST_STDOUT" file. To access all output written to stderr,
    // you can read from the "TEST_STDERR" file.
    int retval;
    retval = beargit_init();
    CU_ASSERT(0==retval);
    retval = beargit_add("asdf.txt");
    CU_ASSERT(0==retval);
}

struct commit {
  char msg[MSG_SIZE];
  struct commit* next;
};


void free_commit_list(struct commit** commit_list) {
  if (*commit_list) {
    free_commit_list(&((*commit_list)->next));
    free(*commit_list);
  }

  *commit_list = NULL;
}

void run_commit(struct commit** commit_list, const char* msg) {
    int retval = beargit_commit(msg);
    CU_ASSERT(0==retval);

    struct commit* new_commit = (struct commit*)malloc(sizeof(struct commit));
    new_commit->next = *commit_list;
    strcpy(new_commit->msg, msg);
    *commit_list = new_commit;
}

void simple_log_test(void)
{
    struct commit* commit_list = NULL;
    int retval;
    retval = beargit_init();
    CU_ASSERT(0==retval);
    FILE* asdf = fopen("asdf.txt", "w");
    fclose(asdf);
    retval = beargit_add("asdf.txt");
    CU_ASSERT(0==retval);
    run_commit(&commit_list, "THIS IS BEAR TERRITORY!1");
    run_commit(&commit_list, "THIS IS BEAR TERRITORY!2");
    run_commit(&commit_list, "THIS IS BEAR TERRITORY!3");

    retval = beargit_log(10);
    CU_ASSERT(0==retval);

    struct commit* cur_commit = commit_list;

    const int LINE_SIZE = 512;
    char line[LINE_SIZE];

    FILE* fstdout = fopen("TEST_STDOUT", "r");
    CU_ASSERT_PTR_NOT_NULL(fstdout);

    while (cur_commit != NULL) {
      char refline[LINE_SIZE];

      // First line is commit -- don't check the ID.
      CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
      CU_ASSERT(!strncmp(line,"commit", strlen("commit")));

      // Second line is msg
      sprintf(refline, "   %s\n", cur_commit->msg);
      CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
      CU_ASSERT_STRING_EQUAL(line, refline);

      // Third line is empty
      CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
      CU_ASSERT(!strcmp(line,"\n"));

      cur_commit = cur_commit->next;
    }

    CU_ASSERT_PTR_NULL(fgets(line, LINE_SIZE, fstdout));

    // It's the end of output
    CU_ASSERT(feof(fstdout));
    fclose(fstdout);

    free_commit_list(&commit_list);
}

/*This test initializes the .beargit directory, creates and adds asdf.txt, 
and then tests 3 different commit messages--all of which should have an 
error thrown (extra space, no exclamation point, not all caps).  We check
this by iterating through STDERR and ensuring that each line has the correct
ERROR thrown by beargit_commit. */
void simple_commit_test(void) {
  int retval;
  retval = beargit_init();
  CU_ASSERT(0==retval);
  FILE* asdf = fopen("asdf.txt", "w");
  fclose(asdf);
  retval = beargit_add("asdf.txt");
  CU_ASSERT(0==retval);

  retval = beargit_commit("THIS IS BEAR  TERRITORY!1"); //extra space
  CU_ASSERT(1==retval);

  retval = beargit_commit("THIS IS BEAR TERRITORY."); //no exclamation point
  CU_ASSERT(1==retval);

  retval = beargit_commit("This is Bear Territory!"); // Not all Caps
  CU_ASSERT(1==retval);
  
  const int LINE_SIZE = 512;
  char line[LINE_SIZE];

  FILE* fstderr = fopen("TEST_STDERR", "r");
  CU_ASSERT_PTR_NOT_NULL(fstderr);

  int num_of_tests = 3;
  while (num_of_tests > 0) {
    char refline[LINE_SIZE];

    // Check to ensure error message is printed.
    CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstderr));
    char error_msg[MSG_SIZE] = "ERROR:  Message must contain \"THIS IS BEAR TERRITORY!\"";
    CU_ASSERT(!strncmp(line, error_msg, strlen(error_msg)));

    num_of_tests -= 1;
  }

    CU_ASSERT_PTR_NULL(fgets(line, LINE_SIZE, fstderr));

    // It's the end of output
    CU_ASSERT(feof(fstderr));
    fclose(fstderr);
}

/* This test is working off of the already inizialized .beargit directory
with asdf.txt.  It commits twice (moving the head of the branch forward each time) 
and then it checks out to the first commit. This means we are now detached. When
it tries to commit again, the ERROR about not being on the head of the branch should 
be thrown. This is checked by ensuring the return value is 1 and then iterating through 
STDERR (past the first three commit ERRORS from simple_commit_test) and ensures that the
ERROR statement is printed */
void no_head_commit_test(void) {
  int retval;
  
  int a, b;
  a = beargit_commit("THIS IS BEAR TERRITORY!1"); 
  CU_ASSERT(0==a);
  b = beargit_commit("THIS IS BEAR TERRITORY!2");
  CU_ASSERT(0==b);

  char b_commit_id[COMMIT_ID_SIZE];
  read_string_from_file(".beargit/.prev", b_commit_id, COMMIT_ID_SIZE);

  char a_commit_file[FILENAME_SIZE];
  sprintf(a_commit_file, ".beargit/%s/.prev", b_commit_id);

  char a_commit_id[COMMIT_ID_SIZE];
  read_string_from_file(a_commit_file, a_commit_id, COMMIT_ID_SIZE);

  retval = beargit_checkout(a_commit_id, 0);
  CU_ASSERT(0==retval);

  retval = beargit_commit("THIS IS BEAR TERRITORY!2");
  CU_ASSERT(1==retval);
  
  const int LINE_SIZE = 512;
  char line[LINE_SIZE];

  FILE* fstderr = fopen("TEST_STDERR", "r");
  CU_ASSERT_PTR_NOT_NULL(fstderr);

  int num_of_tests = 4;
  while (num_of_tests > 0) {
    CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstderr));
    if (num_of_tests == 1) {
    // Check to ensure error message is printed.
      char error_msg[MSG_SIZE] = "ERROR:  Need to be on HEAD of a branch to commit.";
      CU_ASSERT(strncmp(line, error_msg, strlen(error_msg)) == 0);
    }
    num_of_tests -= 1;
  }

  CU_ASSERT_PTR_NULL(fgets(line, LINE_SIZE, fstderr));

  // It's the end of output
  CU_ASSERT(feof(fstderr));
  fclose(fstderr);
}


/* This test just tests some of the simple checkout features. 
* First, it inits, then it makes a commit (and stores the ID), branch, two more commits. 
* Subsequently, we try to checkout the old commit. This checks to 
* make sure that we can checkout commits. We then checkout another commit.
* We go back and make another commit and then try to checkout a branch.
* This checks to make sure that we can checkout branches as well. */
void basic_test_checkout(void) {
  int retval;
  retval = beargit_init();
  CU_ASSERT(0==retval);

  retval = beargit_commit("THIS IS BEAR TERRITORY!1"); 
  CU_ASSERT(0==retval);

  char com_id[COMMIT_ID_SIZE];
  read_string_from_file(".beargit/.prev", com_id, COMMIT_ID_SIZE);
  retval = beargit_checkout("branch1", 1);
  CU_ASSERT(0==retval);

  retval = beargit_commit("THIS IS BEAR TERRITORY!2");
  CU_ASSERT(0==retval);

  retval = beargit_commit("THIS IS BEAR TERRITORY!3");
  CU_ASSERT(0==retval);

  char ref_id[COMMIT_ID_SIZE];
  read_string_from_file(".beargit/.prev", ref_id, COMMIT_ID_SIZE);
  retval = beargit_checkout(com_id, 0);
  CU_ASSERT(0==retval);

  retval = beargit_checkout(ref_id, 0);
  CU_ASSERT(0==retval);

  retval = beargit_commit("THIS IS BEAR TERRITORY!5");
  CU_ASSERT(1==retval);

  retval = beargit_checkout("branch1", 0);
  CU_ASSERT(0==retval);
  retval = beargit_commit("THIS IS BEAR TERRITORY!5");
  CU_ASSERT(0==retval); 

  retval = beargit_commit("THIS IS BEAR TERRITORY!6");
  CU_ASSERT(0==retval);

}

/* This test tests the basics of the rest command.
* Simply, it makes a file and writes some text into it. 
* It then commits and then writes other text into the file. 
* It then resets said file so that the text should revert. 
* It then makes sure that the text in the file is the same
* as before the second edit was made. */
void basic_test_reset(void) {
  int retval;
  FILE* asdf = fopen("asdf1.txt", "w");
  fprintf(asdf, "%s\n", "original text");
  fclose(asdf);
  retval = beargit_add("asdf1.txt");
  CU_ASSERT(0==retval);

  retval = beargit_commit("THIS IS BEAR TERRITORY!adsf_commit");
  CU_ASSERT(0==retval);
  char id[COMMIT_ID_SIZE];
  read_string_from_file(".beargit/.prev", id, COMMIT_ID_SIZE);

  FILE* asdf1 = fopen ("asdf1.txt", "w");
  fprintf(asdf1, "%s\n", "should be gone");
  fclose(asdf1);
  retval = beargit_reset(id, "asdf1.txt");
  CU_ASSERT(0==retval); 
  char orig[FILENAME_SIZE];

  read_string_from_file("asdf1.txt", orig, FILENAME_SIZE);
  CU_ASSERT_STRING_EQUAL("original text\n", orig);
}

void basic_merge_test(void) {
  int retval;
  retval = beargit_init();
  CU_ASSERT(0==retval);
  FILE* asdf = fopen("asdf1.txt", "w");
  fprintf(asdf, "%s\n", "original text");
  fclose(asdf);
  retval = beargit_add("asdf1.txt");
  CU_ASSERT(0==retval);

  retval = beargit_commit("THIS IS BEAR TERRITORY!1"); 
  CU_ASSERT(0==retval);

  // Go to newbranch and add/change files
  retval = beargit_checkout("newbranch", 1);
  CU_ASSERT(0==retval);

  FILE* newfile = fopen("newfile.txt", "w");
  fclose(newfile);
  retval = beargit_add("newfile.txt");
  CU_ASSERT(0==retval);
  FILE* asdf1 = fopen("asdf1.txt", "w");
  fprintf(asdf1, "%s\n", "overwrite!");
  fclose(asdf1);
  retval = beargit_add("asdf1.txt");
  CU_ASSERT(0==retval);
  retval = beargit_commit("THIS IS BEAR TERRITORY!2"); 
  CU_ASSERT(0==retval);

  // Go back to master branch
  retval = beargit_checkout("master", 0);
  CU_ASSERT(0==retval);

  // Merge with non-existant branch to throw error
  retval = beargit_merge("newbranchERR");
  CU_ASSERT(1==retval);

  // Merge with actual branch (that has a new asdf1.txt and newfile.txt)
  retval = beargit_merge("newbranch");
  CU_ASSERT(0==retval);

  const int LINE_SIZE = 512;
  char line[LINE_SIZE];

  // Ensure that correct error was thrown when merging with non-existant branch
  FILE* fstderr = fopen("TEST_STDERR", "r");
  CU_ASSERT_PTR_NOT_NULL(fstderr);

  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstderr));
  // Check to ensure error message is printed.
  char error_msg[MSG_SIZE] = "ERROR:  No branch or commit newbranchERR exists.";
  CU_ASSERT(strncmp(line, error_msg, strlen(error_msg)) == 0);

  CU_ASSERT_PTR_NULL(fgets(line, LINE_SIZE, fstderr));

  // It's the end of output
  CU_ASSERT(feof(fstderr));
  fclose(fstderr);

  // Ensure that the correct output was printed when merged with actual branch
  FILE* fstdout = fopen("TEST_STDOUT", "r");
  CU_ASSERT_PTR_NOT_NULL(fstdout);

  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  char output[MSG_SIZE] = "newfile.txt added";
  CU_ASSERT(strncmp(line, output, strlen(output)) == 0);

  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  char output2[MSG_SIZE] = "asdf1.txt conflicted copy created";
  CU_ASSERT(strncmp(line, output2, strlen(output2)) == 0);

  CU_ASSERT_PTR_NULL(fgets(line, LINE_SIZE, fstderr));

  // It's the end of output
  CU_ASSERT(feof(fstdout));
  fclose(fstdout);
}



/* The main() function for setting up and running the tests.
 * Returns a CUE_SUCCESS on successful running, another
 * CUnit error code on failure.
 */
int cunittester()
{
   CU_pSuite pSuite = NULL;
   CU_pSuite pSuite2 = NULL;
   CU_pSuite pSuite3 = NULL;
   CU_pSuite pSuite4 = NULL;
   // CU_pSuite pSuite5 = NULL;

   /* initialize the CUnit test registry */
   if (CUE_SUCCESS != CU_initialize_registry())
      return CU_get_error();

   /* add a suite to the registry */
   pSuite = CU_add_suite("Suite_1", init_suite, clean_suite);
   if (NULL == pSuite) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* Add tests to the Suite #1 */
   if (NULL == CU_add_test(pSuite, "Simple Test #1", simple_sample_test))
   {
      CU_cleanup_registry();
      return CU_get_error();
   }

   pSuite2 = CU_add_suite("Suite_2", init_suite, clean_suite);
   if (NULL == pSuite2) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* Add tests to the Suite #2 */
   if (NULL == CU_add_test(pSuite2, "Log output test", simple_log_test))
   {
      CU_cleanup_registry();
      return CU_get_error();
   }


   pSuite3 = CU_add_suite("Suite_3", init_suite, clean_suite);
   if (NULL == pSuite3) {
     CU_cleanup_registry();
     return CU_get_error();
   }

   // See comments for details, but this suite is designed to elicit the error
   // messages from beargit_commit to ensure they're thrown at the appropriate
   // times (when the message isn't THIS IS BEAR TERRITORY!, or when a commit
   // is attempted after detachment) and with the message exactly as it appears in the spec.

  //*Add tests to the Suite #3*/
   if (NULL == CU_add_test(pSuite3, "Commit Message Test", simple_commit_test))
   {
     CU_cleanup_registry();
     return CU_get_error();
   }

   if (NULL == CU_add_test(pSuite3, "Detached Commit Test", no_head_commit_test))
   {
     CU_cleanup_registry();
     return CU_get_error();
   }

   pSuite4 = CU_add_suite("Suite_4", init_suite, clean_suite);
   if (NULL == pSuite4) {
     CU_cleanup_registry();
     return CU_get_error();
   }
  
   // Written by my partner, but I think the gist of these are to test the basic functions
   // of beargit_checkout and beargit_reset (which builds upon what the checkout test output).
   // The reset test begins with a file containing text, commits it, then overwrites what was 
   // in the file. This test checks that after the reset, the file in the directory contains
   // the original text (ie it was reset). The checkout test creates scenarios for all three 
   // types of checkout: with just the commit id, checking out to a branch, or creating a new 
   // branch and checking out to that. It also makes sure to throw an error if a commit is called
   // after being detached (much like the test in Suite 3)

   //*Add tests to the Suite #4*/
   if (NULL == CU_add_test(pSuite4, "Basic Checkout Test", basic_test_checkout)) {
     CU_cleanup_registry();
     return CU_get_error();
   }
  
   if (NULL == CU_add_test(pSuite4, "Basic Reset Test", basic_test_reset)) {
     CU_cleanup_registry();
     return CU_get_error();
   }

   // pSuite5 = CU_add_suite("Suite_5", init_suite, clean_suite);
   // if (NULL == pSuite5) {
   //   CU_cleanup_registry();
   //   return CU_get_error();
   // }

   // //*Add tests to the Suite #5*/
   // if (NULL == CU_add_test(pSuite5, "Basic Merge Test", basic_merge_test)) {
   //   CU_cleanup_registry();
   //   return CU_get_error();
   // }
  

   /* Run all tests using the CUnit Basic interface */
   CU_basic_set_mode(CU_BRM_VERBOSE);
   CU_basic_run_tests();
   CU_cleanup_registry();
   return CU_get_error();
}