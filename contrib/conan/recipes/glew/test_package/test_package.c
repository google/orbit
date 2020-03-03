#include <GL/glew.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int main ()
{
    assert(glewGetString(GLEW_VERSION));
    printf("Bincrafters GLEW %s\n", glewGetString(GLEW_VERSION));
    return EXIT_SUCCESS;
}
