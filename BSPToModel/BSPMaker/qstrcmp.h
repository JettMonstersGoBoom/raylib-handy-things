#ifndef _qSTRCMP_H_
#define _qSTRCMP_H_

int qstricmp(const char *a, const char *b) {
  int ca, cb;
  do {
     ca = (unsigned char) *a++;
     cb = (unsigned char) *b++;
     ca = tolower(toupper(ca));
     cb = tolower(toupper(cb));
   } while (ca == cb && ca != '\0');
   return ca - cb;
}
#else 
int qstricmp(const char *a, const char *b);
#endif

