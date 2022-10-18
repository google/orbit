/* Freetype GL - A C OpenGL Freetype engine
 *
 * Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
 * file `LICENSE` for more details.
 */
#ifndef __VEC234_H__
#define __VEC234_H__

#ifdef __cplusplus
extern "C" {
namespace ftgl {
#endif


/**
 * Tuple of 4 ints.
 *
 * Each field can be addressed using several aliases:
 *  - First component:  <b>x</b>, <b>r</b>, <b>red</b> or <b>vstart</b>
 *  - Second component: <b>y</b>, <b>g</b>, <b>green</b> or <b>vcount</b>
 *  - Third component:  <b>z</b>, <b>b</b>, <b>blue</b>, <b>width</b> or <b>istart</b>
 *  - Fourth component: <b>w</b>, <b>a</b>, <b>alpha</b>, <b>height</b> or <b>icount</b>
 *
 */
typedef union
{
     int data[4];    /**< All compoments at once     */
     struct {
        int x;      /**< Alias for first component  */
        int y;      /**< Alias for second component */
        int z;      /**< Alias for third component  */
        int w;      /**< Alias for fourht component */
    };
    struct {
        int x_;     /**< Alias for first component  */
        int y_;     /**< Alias for second component */
        int width;  /**< Alias for third component  */
        int height; /**< Alias for fourth component */
    };
    struct {
        int r;      /**< Alias for first component  */
        int g;      /**< Alias for second component */
        int b;      /**< Alias for third component  */
        int a;      /**< Alias for fourth component */
    };
    struct {
        int red;    /**< Alias for first component  */
        int green;  /**< Alias for second component */
        int blue;   /**< Alias for third component  */
        int alpha;  /**< Alias for fourth component */
    };
    struct {
        int vstart; /**< Alias for first component  */
        int vcount; /**< Alias for second component */
        int istart; /**< Alias for third component  */
        int icount; /**< Alias for fourth component */
    };
} ivec4;


/**
 * Tuple of 3 ints.
 *
 * Each field can be addressed using several aliases:
 *  - First component:  <b>x</b>, <b>r</b> or <b>red</b>
 *  - Second component: <b>y</b>, <b>g</b> or <b>green</b>
 *  - Third component:  <b>z</b>, <b>b</b> or <b>blue</b>
 *
 */
typedef union
{
    int data[3];    /**< All compoments at once     */
    struct {
        int x;      /**< Alias for first component  */
        int y;      /**< Alias for second component */
        int z;      /**< Alias for third component  */
    };
    struct {
        int r;      /**< Alias for first component  */
        int g;      /**< Alias for second component */
        int b;      /**< Alias for third component  */
    };
    struct {
        int red;    /**< Alias for first component  */
        int green;  /**< Alias for second component */
        int blue;   /**< Alias for third component  */
    };
} ivec3;


/**
 * Tuple of 2 ints.
 *
 * Each field can be addressed using several aliases:
 *  - First component: <b>x</b>, <b>s</b> or <b>start</b>
 *  - Second component: <b>y</b>, <b>t</b> or <b>end</b>
 *
 */
typedef union
{
    int data[2];    /**< All compoments at once     */
    struct {
        int x;      /**< Alias for first component  */
        int y;      /**< Alias for second component */
    };
    struct {
        int s;      /**< Alias for first component  */
        int t;      /**< Alias for second component */
    };
    struct {
        int start;  /**< Alias for first component  */
        int end;    /**< Alias for second component */
    };
} ivec2;


/**
 * Tuple of 4 floats.
 *
 * Each field can be addressed using several aliases:
 *  - First component:  <b>x</b>, <b>left</b>, <b>r</b> or <b>red</b>
 *  - Second component: <b>y</b>, <b>top</b>, <b>g</b> or <b>green</b>
 *  - Third component:  <b>z</b>, <b>width</b>, <b>b</b> or <b>blue</b>
 *  - Fourth component: <b>w</b>, <b>height</b>, <b>a</b> or <b>alpha</b>
 */
typedef union
{
    float data[4];    /**< All compoments at once    */
    struct {
        float x;      /**< Alias for first component */
        float y;      /**< Alias for second component */
        float z;      /**< Alias for third component  */
        float w;      /**< Alias for fourth component */
    };
    struct {
        float left;   /**< Alias for first component */
        float top;    /**< Alias for second component */
        float width;  /**< Alias for third component  */
        float height; /**< Alias for fourth component */
    };
    struct {
        float r;      /**< Alias for first component */
        float g;      /**< Alias for second component */
        float b;      /**< Alias for third component  */
        float a;      /**< Alias for fourth component */
    };
    struct {
        float red;    /**< Alias for first component */
        float green;  /**< Alias for second component */
        float blue;   /**< Alias for third component  */
        float alpha;  /**< Alias for fourth component */
    };
} vec4;


/**
 * Tuple of 3 floats
 *
 * Each field can be addressed using several aliases:
 *  - First component:  <b>x</b>, <b>r</b> or <b>red</b>
 *  - Second component: <b>y</b>, <b>g</b> or <b>green</b>
 *  - Third component:  <b>z</b>, <b>b</b> or <b>blue</b>
 */
typedef union
{
    float data[3];   /**< All compoments at once    */
    struct {
        float x;     /**< Alias for first component */
        float y;     /**< Alias fo second component */
        float z;     /**< Alias fo third component  */
    };
    struct {
        float r;     /**< Alias for first component */
        float g;     /**< Alias fo second component */
        float b;     /**< Alias fo third component  */
    };
    struct {
        float red;   /**< Alias for first component */
        float green; /**< Alias fo second component */
        float blue;  /**< Alias fo third component  */
    };
} vec3;


/**
 * Tuple of 2 floats
 *
 * Each field can be addressed using several aliases:
 *  - First component:  <b>x</b> or <b>s</b>
 *  - Second component: <b>y</b> or <b>t</b>
 */
typedef union
{
    float data[2]; /**< All components at once     */
    struct {
        float x;   /**< Alias for first component  */
        float y;   /**< Alias for second component */
    };
    struct {
        float s;   /**< Alias for first component  */
        float t;   /**< Alias for second component */
    };
} vec2;


#ifdef __cplusplus
}
}
#endif

#endif /* __VEC234_H__ */
