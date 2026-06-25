#ifndef GPGR_GL_FUNCS_HPP
# define GPGR_GL_FUNCS_HPP

#include "../Basic/basic.hpp"

#ifndef GL_SILENCE_DEPRECATION
# define GL_SILENCE_DEPRECATION
#endif

#ifdef __APPLE__
# include <OpenGL/gl3.h>
#elif defined(_WIN32)
# ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
# endif
# include <windows.h>
# ifndef GL_GLEXT_PROTOTYPES
#  define GL_GLEXT_PROTOTYPES
# endif
# include <GL/gl.h>
# include <GL/glext.h>
# include <GL/wglext.h>
#else
# ifndef GL_GLEXT_PROTOTYPES
#  define GL_GLEXT_PROTOTYPES
# endif
# include <GL/gl.h>
# include <GL/glext.h>
# include <GL/glx.h>
#endif

#if !defined(__APPLE__)
extern PFNGLGENVERTEXARRAYSPROC         glGenVertexArrays_ptr;
extern PFNGLBINDVERTEXARRAYPROC         glBindVertexArray_ptr;
extern PFNGLDELETEVERTEXARRAYSPROC      glDeleteVertexArrays_ptr;
extern PFNGLGENBUFFERSPROC              glGenBuffers_ptr;
extern PFNGLBINDBUFFERPROC              glBindBuffer_ptr;
extern PFNGLBUFFERDATAPROC              glBufferData_ptr;
extern PFNGLBUFFERSUBDATAPROC           glBufferSubData_ptr;
extern PFNGLDELETEBUFFERSPROC           glDeleteBuffers_ptr;
extern PFNGLVERTEXATTRIBIPOINTERPROC    glVertexAttribIPointer_ptr;
extern PFNGLVERTEXATTRIBPOINTERPROC     glVertexAttribPointer_ptr;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray_ptr;
extern PFNGLCREATESHADERPROC            glCreateShader_ptr;
extern PFNGLSHADERSOURCEPROC            glShaderSource_ptr;
extern PFNGLCOMPILESHADERPROC           glCompileShader_ptr;
extern PFNGLGETSHADERIVPROC             glGetShaderiv_ptr;
extern PFNGLGETSHADERINFOLOGPROC        glGetShaderInfoLog_ptr;
extern PFNGLDELETESHADERPROC            glDeleteShader_ptr;
extern PFNGLCREATEPROGRAMPROC           glCreateProgram_ptr;
extern PFNGLATTACHSHADERPROC            glAttachShader_ptr;
extern PFNGLLINKPROGRAMPROC             glLinkProgram_ptr;
extern PFNGLGETPROGRAMIVPROC            glGetProgramiv_ptr;
extern PFNGLGETPROGRAMINFOLOGPROC       glGetProgramInfoLog_ptr;
extern PFNGLUSEPROGRAMPROC              glUseProgram_ptr;
extern PFNGLDELETEPROGRAMPROC           glDeleteProgram_ptr;
extern PFNGLGETUNIFORMLOCATIONPROC      glGetUniformLocation_ptr;
extern PFNGLUNIFORMMATRIX4FVPROC        glUniformMatrix4fv_ptr;
extern PFNGLUNIFORM1IPROC               glUniform1i_ptr;
extern PFNGLUNIFORM3FVPROC              glUniform3fv_ptr;
extern PFNGLUNIFORM4FVPROC              glUniform4fv_ptr;
extern PFNGLUNIFORM4FPROC               glUniform4f_ptr;
extern PFNGLUNIFORM2FPROC               glUniform2f_ptr;
extern PFNGLUNIFORM1FPROC               glUniform1f_ptr;
extern PFNGLGENERATEMIPMAPPROC          glGenerateMipmap_ptr;
extern PFNGLACTIVETEXTUREPROC           glActiveTexture_ptr;

# define glGenVertexArrays         glGenVertexArrays_ptr
# define glBindVertexArray         glBindVertexArray_ptr
# define glDeleteVertexArrays      glDeleteVertexArrays_ptr
# define glGenBuffers              glGenBuffers_ptr
# define glBindBuffer              glBindBuffer_ptr
# define glBufferData              glBufferData_ptr
# define glBufferSubData           glBufferSubData_ptr
# define glDeleteBuffers           glDeleteBuffers_ptr
# define glVertexAttribIPointer    glVertexAttribIPointer_ptr
# define glVertexAttribPointer      glVertexAttribPointer_ptr
# define glEnableVertexAttribArray glEnableVertexAttribArray_ptr
# define glCreateShader            glCreateShader_ptr
# define glShaderSource            glShaderSource_ptr
# define glCompileShader           glCompileShader_ptr
# define glGetShaderiv             glGetShaderiv_ptr
# define glGetShaderInfoLog        glGetShaderInfoLog_ptr
# define glDeleteShader            glDeleteShader_ptr
# define glCreateProgram           glCreateProgram_ptr
# define glAttachShader            glAttachShader_ptr
# define glLinkProgram             glLinkProgram_ptr
# define glGetProgramiv            glGetProgramiv_ptr
# define glGetProgramInfoLog       glGetProgramInfoLog_ptr
# define glUseProgram              glUseProgram_ptr
# define glDeleteProgram           glDeleteProgram_ptr
# define glGetUniformLocation      glGetUniformLocation_ptr
# define glUniformMatrix4fv        glUniformMatrix4fv_ptr
# define glUniform1i               glUniform1i_ptr
# define glUniform3fv              glUniform3fv_ptr
# define glUniform4fv              glUniform4fv_ptr
# define glUniform4f               glUniform4f_ptr
# define glUniform2f               glUniform2f_ptr
# define glUniform1f               glUniform1f_ptr
# define glGenerateMipmap          glGenerateMipmap_ptr
# define glActiveTexture           glActiveTexture_ptr

ft_bool gpgr_load_gl_functions() noexcept;
#endif

#endif
