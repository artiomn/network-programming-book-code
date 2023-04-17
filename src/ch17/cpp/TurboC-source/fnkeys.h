/***************************************************************************
  Copyright 1990, 2002 Ronald S. Burkey.

  This file is part of TurboC.

  TurboC is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  TurboC is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with TurboC; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

  Contact Ron Burkey at info@sandroid.org.
 
  Filename:	fnkeys.h
  Purpose:	Header containing MS-DOS "scan codes" for an IBM PC keyboard.
  Mods:		03/02/02 RSB	Added to TurboC project.

  Symbolic constants for reading special function keys from IBM PC keyboard.
  Example:  using getch() to fetch an ALT-A will return a 0 on the first
  call and an A_A on the second.  (A_, N_, S_, and C_ stand for "Alt-",
  "normal", "Shift-", and "Control-".)  The only other special keys are
  Ctrl-A (C_A), Ctrl-B (C_B), etc., which are returned in a single getch()
  as a number from 1..26.
***************************************************************************/

#ifndef _INCLUDED_FNKEYS_H
#define _INCLUDED_FNKEYS_H

enum N_NAMES
{
  C_A = 0X01, C_B, C_C, C_D, C_E, C_F, C_G, C_H, C_I, C_J, C_K, C_L, C_M,
  C_N, C_O,
  C_P, C_Q, C_R, C_S, C_T, C_U, C_V, C_W, C_X, C_Y, C_Z, N_ESC /* =0X1B */ ,
  A_Q = 0X10, A_W, A_E, A_R, A_T, A_Y, A_U, A_I, A_O, A_P /* =0X19 */ ,
  A_A = 0X1E, A_S, A_D, A_F, A_G, A_H, A_J, A_K, A_L /* =0X26 */ ,
  A_Z = 0X2C, A_X, A_C, A_V, A_B, A_N, A_M /* =0X32 */ ,
  N_F1 =
    0X3B, N_F2, N_F3, N_F4, N_F5, N_F6, N_F7, N_F8, N_F9, N_F10 /* =0X44 */ ,
  N_HOME = 0X47, N_UP, N_PGUP, N_PADMINUS, N_LEFT /* =0X4B */ ,
  N_RIGHT = 0X4D, N_PADPLUS, N_END, N_DOWN, N_PGDN, N_INS, N_DEL,
  S_F1, S_F2, S_F3, S_F4, S_F5, S_F6, S_F7, S_F8, S_F9, S_F10,
  C_F1, C_F2, C_F3, C_F4, C_F5, C_F6, C_F7, C_F8, C_F9, C_F10,
  A_F1, A_F2, A_F3, A_F4, A_F5, A_F6, A_F7, A_F8, A_F9, A_F10 /* =0X71 */ ,
  C_LEFT = 0X73, C_RIGHT, C_END, C_PGDN, C_HOME,
  A_1, A_2, A_3, A_4, A_5, A_6, A_7, A_8, A_9, A_0, A_MINUS, A_EQUALS,
  C_PGUP			/* =0X84 */
};

#endif /* _INCLUDED_FNKEYS_H */
