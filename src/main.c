#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <common.h>
#include <file.h>
#include <parse.h>

void print_usage(char *argv[]) {
  printf("\nUsage: %s -n -f <database file>\n", argv[0]);
  printf("\t-n - create new database file\n");
  printf("\t-f - path to database file\n");
  return;
}

int main(int argc, char *argv[]) {

  char *filepath = NULL;
  char *addstring = NULL;
  bool newfile = false;
  int c;

  int dbfd = -1;
  struct dbheader_t *dbhdr = NULL;
  struct employee_t *employees = NULL;

  while ((c = getopt(argc, argv, "nf:a:")) != -1) {
    switch (c) {
    case 'n':
      newfile = true;
      break;
    case 'f':
      filepath = optarg;
      break;
    case 'a':
      addstring = optarg;
      break;
    case '?':
      printf("Unknown options -%c\n", c);
      break;
    default:
      return -1;
    }
  }
  if (filepath == NULL) {
    printf("Filepath is required\n");
    print_usage(argv);
    return -1;
  }

  if (newfile) {
    dbfd = create_db_file(filepath);
    if (dbfd == STATUS_ERROR) {
      printf("Unable to create database file\n");
      return -1;
    }

    if (create_db_header(&dbhdr) == STATUS_ERROR) {
      return -1;
      printf("Failed to create database header\n");
    }
  } else {
    dbfd = open_db_file(filepath);
    if (dbfd == STATUS_ERROR) {
      printf("Unable to open database file\n");
      return -1;
    }
    if (validate_db_header(dbfd, &dbhdr) == STATUS_ERROR) {
      printf("Failed to validate database header\n");
      return -1;
    }
  }

  if (read_employees(dbfd, dbhdr, &employees) != STATUS_SUCCESS) {
    printf("Failed to read employees\n");
    return 0;
  }

  if (addstring) {
    dbhdr->count++;
    employees = realloc(employees, dbhdr->count * sizeof(struct employee_t));
    add_employee(dbhdr, employees, addstring);
  }

  output_file(dbfd, dbhdr, employees);

  return 0;
}
