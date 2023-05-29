#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
/* Width field, default width is 8 */
#define BIT_WIDE_HEX 8

int printf(const char *fmt, ...) {
  char buffer[2048];/* to small buffer will cause stackoverflow */
  va_list ap;
  int ret;

  va_start(ap, fmt);
  ret = vsprintf(buffer, fmt, ap);
  va_end(ap);

  putstr(buffer);

  return ret;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  /*
   *  vsprintf() is equivalent to the function sprintf(),
   *  except that it is called with a va_list instead of a variable number of arguments.
   *  it doesn't call the va_end macro, because it invokes the va_arg macro, 
   *  the value of ap is undefined after the call.
   */
  char buffer[32];
  char *txt, cha;
  int num, len, len_p;
  uint32_t addr_p;
  int i, j, len_format_flag = 0;
  for (i = 0, j = 0; fmt[i] != '\0'; ++i){
    if (fmt[i] != '%' && len_format_flag == 0){
      out[j++]=fmt[i];
      continue;
    }
    switch (fmt[++i]){
      case 's':
        len_format_flag = 0;
        txt = va_arg(ap, char*);
        for (int k = 0; txt[k] !='\0'; ++k)
          out[j++] = txt[k];
        break;
      
      case 'd':
        len_format_flag = 0;
        num = va_arg(ap, int);
        if(num == 0){
          out[j++] = '0';
          break;
        }
        for (len = 0; num ; num /= 10, ++len)
          buffer[len] = num % 10 + '0'; /* inverted sequence */
        for (int k = len - 1; k >= 0; --k)
          out[j++] = buffer[k];
        break;
      
      case 'p':
        len_format_flag = 0;
        addr_p = va_arg(ap, uint32_t);
        for (len_p = 0; addr_p ; addr_p /= 16, ++len_p)
          buffer[len_p] = addr_p % 16 + '0'; /* inverted sequence */
        for (int k = 0; k < BIT_WIDE_HEX - len_p; ++k)
          out[j++] = '0';/* add 0 if the length is not enough */
        for (int k = len_p - 1; k >= 0; --k)
          out[j++] = buffer[k];
        break;

      case '0' ... '9':  /* for format like %02d */
        len_format_flag = 1;
        break;

      case 'c':
        cha = (char)va_arg(ap, int);
        out[j++] = cha;
        len_format_flag = 0;
        break;

      default:
        break;
        // assert(0);
    }  
  }
  out[j] = '\0';
  return j;
}

int sprintf(char *out, const char *fmt, ...) {
  va_list ap;
  /*
   *  The va_start() macro initializes ap for subsequent use by va_arg() and va_end(), 
   *  and must be called first.
   */
  va_start(ap, fmt);
  int num = vsprintf(out ,fmt, ap);
  va_end(ap);/* after the call va_end(ap) the variable ap is undefined. */
  return num;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  /* write at most n bytes to out */
  va_list ap;
  va_start(ap, fmt);
  int len = vsnprintf(out, n, fmt, ap);
  va_end(ap);
  return len;
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  int written = 0; // Number of characters written to the out
  // Iterate through the fmt string
  while (*fmt != '\0')
  {
      // Check if there is enough space in the out
      if (written >= n - 1)
        break;
      if (*fmt == '%')
      {
        fmt++; // Move past '%'
        // Handle fmt specifiers
        switch (*fmt)
        {
          case 's':
          {
            const char* str = va_arg(ap, const char*);
            // Copy the string to the out
            while (*str != '\0' && written < n - 1)
            {
              out[written] = *str;
              str++;
              written++;
            }
            break;
          }
          case 'd':
          {
            int num = va_arg(ap, int);
            int numLength = 0;
            // Calculate the number of digits in the integer
            if (num == 0)
              numLength = 1;
            else
            {
              int temp = num;
              while (temp != 0)
              {
                temp /= 10;
                numLength++;
              }
            }
            // Check if there is enough space in the out
            if (written + numLength >= n - 1)
              break;
            // Convert the integer to a string
            char numStr[12]; // Assuming a maximum of 12 digits for the integer
            int i = numLength - 1;
            if (num == 0)
              numStr[i] = '0';
            else
            {
              while (num != 0)
              {
                numStr[i] = '0' + (num % 10);
                num /= 10;
                i--;
              }
            }
            // Copy the integer string to the out
            for (int j = 0; j < numLength; j++)
            {
              out[written] = numStr[j];
              written++;
            }
            break;
          }
          // Add support for more fmt specifiers as needed
          default:
            // Unsupported fmt specifier, ignore it
            break;
          }
        }
      else
      {
        // Copy regular characters to the buffer
        out[written] = *fmt;
        written++;
      }
      fmt++;
  }
  // Add null terminator to the buffer
  if (written < n)
    out[written] = '\0';
  else if (n > 0)
    out[n - 1] = '\0';
  return written;
}

#endif
