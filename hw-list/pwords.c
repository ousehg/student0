/*
 * Word count application with one thread per input file.
 *
 * You may modify this file in any way you like, and are expected to modify it.
 * Your solution must read each input file from a separate thread. We encourage
 * you to make as few changes as necessary.
 */

/*
 * Copyright © 2021 University of California, Berkeley
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <pthread.h>

#include "word_count.h"
#include "word_helpers.h"

#define NUM_THREADS 4

typedef struct {
  int threadid;
  char* file_name;
  word_count_list_t* wclist;
} thread_args_t;

void* thread_per_file(void* threadargs) {
  thread_args_t* args = (thread_args_t*)threadargs;
  printf("Thread #%d started, counting words of %s\n", args->threadid, args->file_name);
  FILE* f = fopen(args->file_name, "r");
  count_words(args->wclist, f);
  fclose(f);
  pthread_exit(NULL);
}

/*
 * main - handle command line, spawning one thread per file.
 */
int main(int argc, char* argv[]) {
  int rc;
  int nthreads = NUM_THREADS;

  /* Create the empty data structure. */
  word_count_list_t word_counts;
  init_words(&word_counts);

  pthread_t threads[argc];
  thread_args_t args[argc];

  if (argc <= 1) {
    /* Process stdin in a single thread. */
    count_words(&word_counts, stdin);
  } else {
    nthreads = argc - 1;
    int t;

    for (t = 0; t < nthreads; t++) {
      args[t].threadid = t;
      args[t].file_name = argv[t + 1];
      args[t].wclist = &word_counts;
      rc = pthread_create(&threads[t], NULL, thread_per_file, (void*)&args[t]);
      if (rc) {
        printf("ERROR; return code from pthread_create() is %d\n", rc);
        exit(-1);
      }
    }

    for (t = 0; t < nthreads; t++) {
      rc = pthread_join(threads[t], NULL);
      if (rc) {
        fprintf(stderr, "Error joining thread %d: %d\n", t, rc);
        exit(-1);
      }
      printf("Thread #%d finished\n", t);
    }
  }

  /* Output final result of all threads' work. */
  wordcount_sort(&word_counts, less_count);
  fprint_words(&word_counts, stdout);
  return 0;
}
