#include <ctype.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <limits.h>

#ifndef ANSI

#include <sys/types.h>
#include <sys/timeb.h>

#ifndef WIN32
#include <sys/time.h>
struct timeval tv;
#endif

#endif

#include "bce.h"

/* bioskey routine from faile by Adrien M. Regimbald */
/**************************************************
 * Faile version 1.4                              *
 * Author: Adrien Regimbald                       *
 * E-mail: adrien@ualberta.ca                     *
 * Web Page: http://www.ualberta.ca/~adrien/      *
 */

#ifdef ANSI

int bioskey() 
{
    return 0;
}

#else

#ifndef WIN32

int bioskey (void) {

  fd_set readfds;

  FD_ZERO (&readfds);
  FD_SET (fileno (stdin), &readfds);
  tv.tv_sec=0; tv.tv_usec=0;
  select (16, &readfds, 0, 0, &tv);

  return (FD_ISSET (fileno (stdin), &readfds));

}

#else

#undef frame
#include <windows.h>
#define frame 0

int bioskey (void) {

  static int init = 0, pipe;
  static HANDLE inh;
  DWORD dw;

  if (!init) {
    init = 1;
    inh = GetStdHandle (STD_INPUT_HANDLE);
    pipe = !GetConsoleMode (inh, &dw);
    if (!pipe) {
      SetConsoleMode(inh, dw & ~(ENABLE_MOUSE_INPUT|ENABLE_WINDOW_INPUT));
      FlushConsoleInputBuffer (inh);
    }
  }
  if (pipe) {
    if (!PeekNamedPipe (inh, NULL, 0, NULL, &dw, NULL)) {
      return 1;
    }
    return dw;
  } else {
    GetNumberOfConsoleInputEvents (inh, &dw);
    return dw <= 1 ? 0 : dw;
  }

  return (0);

}

#endif

#endif

void rinput (char str[], int n, FILE *stream) {

  /* My input function - reads in up to n-1 characters from stream, or until
     we encounter a \n or an EOF.  Appends a null character at the end of the
     string, and stores the string in str[] */

  int ch, i = 0;

  while ((ch = getc (stream)) != (int) '\n' && ch != EOF) {
    if (i < n-1) {
      str[i++] = ch;
    }
  }

  str [i] = '\0';

}

