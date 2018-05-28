#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>

#include <testsuite.h>

static const char *test_init(void *arg)
{
  char cwd[256];
  if (getcwd(cwd, 256)[0] == '\0')
    chdir("host:");

  if (strcmp(getcwd(cwd, 256),"host:"))
    return "failed to setup cwd";

  return NULL;
}

static const char *test_fgets(void *arg)
{
  char buf[64], *ret;
  FILE *fp = fopen((char *)arg, "rt");
  if (fp == NULL)
  {
    return "failed to open test file";
  }

  ret = fgets(buf, sizeof(buf), fp);
  fclose(fp);

  if (ret != buf || strcmp(buf, "hello world\n") != 0)
  {
    return "read wrong data from file";
  }

  return NULL;
}

static const char *test_fopen_fclose(void *arg)
{
  FILE *fp = fopen((char *)arg, "rt");
  if (fp == NULL)
  {
    return "failed to open test file";
  }

  fclose(fp);
  return NULL;
}

static const char *test_fgetc(void *arg)
{
  int p, ch;
  char *error;

  FILE *fp = fopen((char *)arg, "rt");
  if (fp == NULL)
  {
    return "failed to open test file";
  }

  /* read one byte */
  ch = fgetc(fp);
  if (ch != 'h')
  {
    error = "fgetc failed";
    goto failed;
  }

  /* read until EOF */
  for (p=0; p<512; p++)
  {
    ch = fgetc(fp);
    if (ch == EOF)
    {
      break;
    }
  }

  if (ch != EOF)
  {
    error = "EOF not returned when should";
    goto failed;
  }

  fclose(fp);
  return NULL;

failed:
  fclose(fp);
  return error;
}

static const char *test_fseek_ftell(void *arg)
{
  int off, ret;
  char buf[512], *error;

  FILE *fp = fopen((char *)arg, "rt");
  if (fp == NULL)
  {
    return "failed to open test file";
  }

  /* seek to the beginning of a file */
  ret = fseek(fp, 0, SEEK_SET);
  if (ret != 0)
  {
    error = "failed to seek to beginning of file";
    goto failed;
  }

  off = ftell(fp);
  if (off != 0)
  {
    error = "ftell returned wrong offset\n";
    goto failed;
  }

  ret = fread(buf, 2, 1, fp);
  if (ret != 1 || strncmp(buf, "he", 2) != 0)
  {
    error = "failed to read first two bytes";
    goto failed;
  }

  /* seek forward */
  ret = fseek(fp, +2, SEEK_CUR);
  off = ftell(fp);
  if (ret != 0 || off != 4)
  {
    error = "failed to seek forward / tell returned wrong offset";
    goto failed;
  }

  ret = fread(buf, 2, 1, fp);
  if (ret != 1 || strncmp(buf, "o ", 2) != 0)
  {
    error = "error reading from test file";
    goto failed;
  }

  /* seek backward */
  ret = fseek(fp, -2, SEEK_CUR);
  off = ftell(fp);
  if (ret != 0 || off != 4)
  {
    error = "error seeking backwards / tell returned wrong offset";
    goto failed;
  }

  ret = fread(buf, 2, 1, fp);
  if (ret != 1 || strncmp(buf, "o ", 2) != 0)
  {
    error = "error reading from test file";
    goto failed;
  }

  /* seek to end of file, "hello world" = 12 bytes */
  ret = fseek(fp, 0, SEEK_END);
  off = ftell(fp);
  if (ret != 0 || off != 12)
  {
    error = "error seeking end of file";
    goto failed;
  }

  fclose(fp);
  return NULL;

  failed:
  fclose(fp);
  return error;
}

static const char *test_fread(void *arg)
{
  int ret;
  char buf[512], *error;

  FILE *fp = fopen((char *)arg, "rt");
  if (fp == NULL)
  {
    return "cannot open test file";
  }

  /* test of reading one chunk */
  ret = fread(buf, 3, 1, fp);
  if (ret != 1 || strncmp(buf, "hel", 3) != 0)
  {
    error = "failed to read one block";
    goto failed;
  }

  /* three chunks */
  ret = fread(buf, 2, 3, fp);
  if (ret != 3 || strncmp(buf, "lo wor", 6) != 0)
  {
    error = "failed to read three blocks";
    goto failed;
  }

  /* until end of file */
  ret = fread(buf, 1, sizeof(buf), fp);
  if (ret < 2 || strncmp(buf, "ld", 2) != 0)
  {
    error = "failed to read until eof";
    goto failed;
  }

  /* past end of file */
  ret = fread(buf, 1, sizeof(buf), fp);
  if (ret != 0)
  {
    error = "read past end of file";
    goto failed;
  }

  fclose(fp);
  return NULL;

failed:
  fclose(fp);
  return error;
}

static const char *test_fwrite(void *arg)
{
  FILE *fp;
  int ret;

  char buf[12] = "hello world";
  char buf2[20];

  fp = fopen((char *)arg, "w+");
  if (fp == NULL)
    return "cannot open/create test file";

  ret = fwrite(buf, 2, 1, fp);
  if (ret != 1) {
    fclose(fp);
    return "failed to write 1 block of 2 bytes";
  }

  fseek(fp, 0, SEEK_SET);
  ret = fread (buf2, 2, 1, fp);
  if (ret != 1 || strncmp(buf2, "he", 2) != 0) {
    fclose(fp);
    printf("buf2 = %s ret = %d\n", buf2, ret);
    return "failed to write/read 2 bytes correctly";
  }

  ret = fwrite(buf, 1, strlen(buf), fp);
  if (ret != 11) {
    fclose(fp);
    return "failed to write 11 bytes";
  }

  fseek(fp, 0, SEEK_SET);
  ret = fread (buf2, 1, sizeof(buf2), fp);
  if (ret != 13 || strncmp(buf2, "hehello world", 13)) {
    fclose(fp);
    return "failed to write rest of file";
  }

  fclose(fp);

  return NULL;
}

/* These do not work via host: on ps2client.
   The hostfs protocol doesn't implement stat(). */
static const char *test_stat_file(void *arg)
{
  struct stat st;

  stat((const char *)arg, &st);
  printf("fn %s mode 0x%x, size %d\n", (char *)arg, st.st_mode, (int)st.st_size);
  if (S_ISDIR(st.st_mode))
  {
    return "expected file, not a directory";
  }

  return 0;
}

static const char *test_stat_dir(void *arg)
{
  struct stat st;

  stat((const char *)arg, &st);
  printf("fn %s mode 0x%x, size %d\n", (char *)arg, st.st_mode, (int)st.st_size);
  if (S_ISDIR(st.st_mode) == 0)
  {
    return "expected directory, not regular file";
  }

  return 0;
}

static const char *test_mkdir(void *arg)
{
  mode_t mode = 0666;

  if (mkdir(arg,mode) != 0)
  {
    return "failed to create directory";
  }

  return 0;
}

static const char *test_rmdir(void *arg)
{
  if (rmdir(arg) != 0)
  {
    return "failed to delete directory";
  }

  return 0;
}

static const char *test_opendir_closedir(void *arg)
{
  DIR *dd = opendir((char*)arg);

  if (dd == NULL)
    return "failed to open directory";

  if (closedir(dd) != 0)
    return "failed to close directory";

  return NULL;
}

static const char *test_readdir_rewinddir(void *arg)
{
  int i = 0;
  int j = 0;

  DIR *dd = opendir((char*)arg);
  struct dirent *de;

  if (dd == NULL)
    return "failed to open directory";

  while((de = readdir(dd)) != NULL) {
    if (de->d_name == NULL) {
      closedir(dd);
      return "bad dirent returned";
    }
    if (!strlen(de->d_name)) {
      closedir(dd);
      return "empty dirent returned";
    }
    i++;
  }

  rewinddir(dd);

  while((de = readdir(dd)) != NULL) {
    if (de->d_name == NULL) {
      closedir(dd);
      return "bad dirent returned after rewind";
    }
    if (!strlen(de->d_name)) {
      closedir(dd);
      return "empty dirent returned after rewind";
    }
    j++;
  }

  closedir(dd);

  if ((i < 1) || (j < 1))
    return "no directory entries read";

  if (i != j)
    return "directory entries don't match/rewind failed";

  return NULL;
}

int libc_add_tests(test_suite *p)
{
  const char *textfile,*textfile2;
  const char *dir, *dir2;

#ifdef _EE
  textfile = "host:testfiles/dummy";
  textfile2 = "host:testfiles/dummy2";
  dir = "host:testfiles";
  dir2 = "host:dummydir";
#else
  textfile = "testfiles/dummy";
  textfile2 = "testfiles/dummy2";
  dir = "textfiles/";
  dir2 = "dummydir";
#endif

  add_test(p, "init", test_init, NULL);
  add_test(p, "fopen, fclose", test_fopen_fclose, (void *)textfile);
  add_test(p, "fgets", test_fgets, (void *)textfile);
  add_test(p, "fread", test_fread, (void *)textfile);
  add_test(p, "fwrite", test_fwrite, (void*)textfile2);
  add_test(p, "fgetc", test_fgetc, (void *)textfile);
  add_test(p, "fseek, ftell", test_fseek_ftell, (void *)textfile);
  add_test(p, "stat", test_stat_file, (void *)textfile);
  add_test(p, "stat", test_stat_dir, (void *)dir);
  add_test(p, "mkdir", test_mkdir, (void *)dir2);
  add_test(p, "rmdir", test_rmdir, (void *)dir2);
  add_test(p, "opendir, closedir", test_opendir_closedir, (void*)dir);
  add_test(p, "readdir, rewinddir", test_readdir_rewinddir, (void*)dir);

  return 0;
}

