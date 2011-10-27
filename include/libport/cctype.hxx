/*
 * Copyright (C) 2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#if defined WIN32

extern "C"
{
  inline
  int isascii(int c)
  {
    return c > 0 && c < 128;
  }

  inline
  int isblank(int c)
  {
    return c == '\t' || c == ' ';
  }
}

#endif
