#pragma once
#ifndef __glew_h__
#include <GL/glew.h>
#endif

inline static
bool IsExtensionSupported(const char *name)
{
  GLint n=0; 
  glGetIntegerv(GL_NUM_EXTENSIONS, &n); 
  for (GLint i=0; i<n; i++) 
  { 
    const char* extension = 
     (const char*)glGetStringi(GL_EXTENSIONS, i);
    if (!strcmp(name, extension)) {
      return true;
    }
  }
  return false;
}