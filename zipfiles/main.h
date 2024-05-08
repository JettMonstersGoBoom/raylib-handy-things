Texture2D zipLoadTexture(const char *fname);
Image zipLoadImage(const char *fname);
void *zipLoadFileData(const char *fname,int *size);
int _zipFindFile(const char *fname);
void zipsMount(const char *path,const char *ext);
void zipMount(const char *fname);

