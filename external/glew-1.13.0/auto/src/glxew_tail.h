/* ------------------------------------------------------------------------ */

#ifdef GLEW_MX

typedef struct GLXEWContextStruct GLXEWContext;
GLEWAPI GLenum GLEWAPIENTRY glxewContextInit (GLXEWContext *ctx);
GLEWAPI GLboolean GLEWAPIENTRY glxewContextIsSupported (const GLXEWContext *ctx, const char *name);

#define glxewInit() glxewContextInit(glxewGetContext())
#define glxewIsSupported(x) glxewContextIsSupported(glxewGetContext(), x)

#define GLXEW_GET_VAR(x) (*(const GLboolean*)&(glxewGetContext()->x))
#define GLXEW_GET_FUN(x) x

#else /* GLEW_MX */

GLEWAPI GLenum GLEWAPIENTRY glxewInit ();
GLEWAPI GLboolean GLEWAPIENTRY glxewIsSupported (const char *name);

#define GLXEW_GET_VAR(x) (*(const GLboolean*)&x)
#define GLXEW_GET_FUN(x) x

#endif /* GLEW_MX */

GLEWAPI GLboolean GLEWAPIENTRY glxewGetExtension (const char *name);

#ifdef __cplusplus
}
#endif

#endif /* __glxew_h__ */
