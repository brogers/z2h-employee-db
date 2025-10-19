#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "common.h"
#include "parse.h"

void list_employees(struct dbheader_t *dbhdr, struct employee_t *employees) {
  (void)dbhdr;
  (void)employees;
  int i = 0;
  for (; i < dbhdr->count; i++) {
    struct employee_t employee = employees[i];
    printf("Employee %d\n\tName: %s\n\tAddress: %s\n\tHours: %d\n", i,
           employee.name, employee.address, employee.hours);
  }
  return;
}

int add_employee(struct dbheader_t *dbhdr, struct employee_t **employees,
                 char *addstring) {

  if (NULL == dbhdr) return STATUS_ERROR;
  if (NULL == employees) return STATUS_ERROR;
  if (NULL == *employees) return STATUS_ERROR;
  if (NULL == addstring) return STATUS_ERROR;

  char *name = strtok(addstring, ",");
  if (NULL == name) return STATUS_ERROR;

  char *addr = strtok(NULL, ",");
  if (NULL == addr) return STATUS_ERROR;

  char *hours = strtok(NULL, ",");
  if (NULL == hours) return STATUS_ERROR;

  struct employee_t *e = *employees;
  e = realloc(e, sizeof(struct employee_t) * dbhdr->count + 1);
  if (e == NULL) {
    return STATUS_ERROR;
  }

  dbhdr->count++;

  strncpy(e[dbhdr->count - 1].name, name, sizeof(e[dbhdr->count - 1].name) - 1);
  strncpy(e[dbhdr->count - 1].address, addr,
          sizeof(e[dbhdr->count - 1].address) - 1);

  e[dbhdr->count - 1].hours = atoi(hours);

  *employees = e;

  return STATUS_SUCCESS;
}

int read_employees(int fd, struct dbheader_t *dbhdr,
                   struct employee_t **employeesOut) {
  if (fd < 0) {
    printf("Got a bad FD from the user\n");
    return STATUS_ERROR;
  }

  int count = ntohl(dbhdr->count);

  struct employee_t *employees = calloc(count, sizeof(struct employee_t));

  if (employees == NULL) {
    printf("Malloc failed\n");
    return STATUS_ERROR;
  }

  read(fd, employees, count * sizeof(struct employee_t));

  int i = 0;
  for (; i < count; i++) {
    employees[i].hours = ntohl(employees[i].hours);
  }

  *employeesOut = employees;

  return STATUS_SUCCESS;
}

int output_file(int fd, struct dbheader_t *dbhdr,
                struct employee_t *employees) {
  if (fd < 0) {
    printf("Got a bad FD from the user\n");
    return STATUS_ERROR;
  }

  int realcount = dbhdr->count;

  dbhdr->magic = htonl(dbhdr->magic);
  dbhdr->filesize = htonl(sizeof(struct dbheader_t) +
                          (sizeof(struct employee_t) * realcount));
  dbhdr->count = htons(dbhdr->count);
  dbhdr->version = htons(dbhdr->version);

  lseek(fd, 0, SEEK_SET);

  write(fd, dbhdr, sizeof(struct dbheader_t));

  int i = 0;
  for (; i < realcount; i++) {
    employees[i].hours = htonl(employees[i].hours);
    write(fd, &employees[i], sizeof(struct employee_t));
  }

  return STATUS_SUCCESS;
}

int validate_db_header(int fd, struct dbheader_t **headerOut) {
  if (fd < 0) {
    printf("Got a bad FD from the user\n");
    return STATUS_ERROR;
  }

  struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
  if (header == NULL) {
    printf("Malloc failed to create db header\n");
    return STATUS_ERROR;
  }

  if (read(fd, header, sizeof(struct dbheader_t)) !=
      sizeof(struct dbheader_t)) {
    perror("read");
    free(header);
    return STATUS_ERROR;
  }

  header->version = ntohs(header->version);
  header->count = ntohs(header->count);
  header->magic = ntohl(header->magic);
  header->filesize = ntohl(header->filesize);

  if (header->version != 1) {
    printf("Improper header version\n");
    free(header);
    return -1;
  }

  if (header->magic != HEADER_MAGIC) {
    printf("Improper header magic\n");
    free(header);
    return -1;
  }

  struct stat dbstat = {0};
  fstat(fd, &dbstat);
  if (header->filesize != dbstat.st_size) {
    printf("Corrupted database\n");
    free(header);
    return -1;
  }

  *headerOut = header;

  return STATUS_SUCCESS;
}

int create_db_header(struct dbheader_t **headerOut) {
  struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
  if (headerOut == NULL) {
    printf("Received an invalid headerOut\n");
    return STATUS_ERROR;
  }

  if (header == NULL) {
    printf("Malloc failed to create db header\n");
    return STATUS_ERROR;
  }

  header->version = 0x1;
  header->count = 0;
  header->magic = HEADER_MAGIC;
  header->filesize = sizeof(struct dbheader_t);

  *headerOut = header;

  return STATUS_SUCCESS;
}
