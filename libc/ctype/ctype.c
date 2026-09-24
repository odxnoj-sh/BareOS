#include <libc.h>

int isdigit(int c) { return c >= '0' && c <= '9'; }
int isalpha(int c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }
int isalnum(int c) { return isdigit(c) || isalpha(c); }
int isspace(int c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v'; }
int islower(int c) { return c >= 'a' && c <= 'z'; }
int isupper(int c) { return c >= 'A' && c <= 'Z'; }
int isxdigit(int c) { return isdigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'); }
int isprint(int c) { return c >= 32 && c <= 126; }
int iscntrl(int c) { return c < 32 || c == 127; }
int ispunct(int c) { return isprint(c) && !isalnum(c) && !isspace(c); }
int isgraph(int c) { return isprint(c) && !isspace(c); }

int tolower(int c) { return isupper(c) ? c + 'a' - 'A' : c; }
int toupper(int c) { return islower(c) ? c + 'A' - 'a' : c; }