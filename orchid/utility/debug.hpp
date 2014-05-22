
#ifndef __ORCHID_DEBUG_H__
#define __ORCHID_DEBUG_H__

#include <stdio.h>  
  
#define __ORCHID_DEBUG__
  
#ifdef __ORCHID_DEBUG__ 
#define ORCHID_DEBUG(format,...)  printf("File: " __FILE__", Line: %05d: " format"\n", __LINE__, ##__VA_ARGS__)
#else  
#define ORCHID_DEBUG(format,...)  
#endif

#define __ORCHID_ERROR__

#ifdef __ORCHID_ERROR__ 
#define ORCHID_ERROR(format,...)  fprintf(stderr,"File: " __FILE__", Line: %05d: " format"\n", __LINE__, ##__VA_ARGS__)
#else  
#define ORCHID_ERROR(format,...)  
#endif


#endif