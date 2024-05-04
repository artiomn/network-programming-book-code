/* RSA.H - header file for RSA.C
*/

/* Copyright (C) RSA Laboratories, a division of RSA Data Security,
Inc., created 1991. All rights reserved.
*/

int RSAPublicEncrypt (unsigned char *, unsigned int *, unsigned char *, unsigned int, R_RSA_PUBLIC_KEY *, R_RANDOM_STRUCT *);
int RSAPrivateEncrypt(unsigned char *, unsigned int *, unsigned char *, unsigned int, R_RSA_PRIVATE_KEY *);
int RSAPublicDecrypt (unsigned char *, unsigned int *, unsigned char *, unsigned int, R_RSA_PUBLIC_KEY *);
int RSAPrivateDecrypt(unsigned char *, unsigned int *, unsigned char *, unsigned int, R_RSA_PRIVATE_KEY *, R_RANDOM_STRUCT *);

