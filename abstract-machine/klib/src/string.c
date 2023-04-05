#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  /*
   *  strlen()  calculates the length of the string pointed to by s, 
   *  excluding the terminating null byte ('\0').
   */
  size_t length = 0;
  while(*s++ != 0) length++;
  return length;
}

char *strcpy(char *dst, const char *src) {
  /*
   *  The strcpy() function copies the string pointed to by src,
   *  including the terminating null byte ('\0'), 
   *  to the buffer pointed to by dest.
   */
  char *tmp = dst;
  while((*dst++ = *src++) != 0) ;
  return tmp;
}

char *strncpy(char *dst, const char *src, size_t n) {
  panic("Not implemented");
}

char *strcat(char *dst, const char *src) {
  /*
   *  The strcat() function appends the src string to the dest string,
   *  overwriting the terminating null byte ('\0') at the end of dest, 
   *  and then adds a terminating null byte.
   */
  size_t i = strlen(dst);
  while((dst[i] = *src++) != 0) i++;
  return dst;
}

int strcmp(const char *s1, const char *s2) {
  /*
   *  strcmp() compares the two strings s1 and s2,
   *  the comparison is done using unsigned characters.
   *  strcmp() returns:
   *    0, if the s1 and s2 are equal;
   *    a negative value if s1 is less than s2;
   *    a positive value if s1 is greater than s2.
   * 
   */
  while(*s1 && *s1 == *s2)
    s1++, s2++;
  return (unsigned char)*s1 - (unsigned char)*s2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  panic("Not implemented");
}

void *memset(void *s, int c, size_t n) {
  /*
   *  The memset() function fills the first n bytes of the memory 
   *  area pointed to by s with the constant byte c.
   */
  char *cdst = (char *) s;
  int i;
  for(i = 0; i < n; i++){
    cdst[i] = c;
  }
  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  panic("Not implemented");
}

void *memcpy(void *s1, const void *s2, size_t n) {
  unsigned char *us1=(unsigned char *)s1;
  const unsigned char *us2=(const unsigned char *)s2;
  for(;n>0;++us1,++us2,--n)
      *us1=*us2;
  return s1;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  /*
   *  The memcmp() function compares the first n bytes 
   *  (each interpreted as unsigned char) 
   *  of the memory areas s1 and s2.
   */
  const char *p1 = s1, *p2 = s2;
  while (n-- > 0) {
    if (*p1 != *p2) {
      return *p1 - *p2;
    }
    p1++;
    p2++;
  }
  return 0;
}

#endif
