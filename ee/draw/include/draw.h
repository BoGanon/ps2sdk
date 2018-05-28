/**
 * @file
 * Draw library main functions
 */

#ifndef __DRAW_H__
#define __DRAW_H__

#include <tamtypes.h>
#include <math3d.h>

#undef ftoi4
#define ftoi4(F) ((int)(((float)F)*16.0f))

#define DRAW_DISABLE 0
#define DRAW_ENABLE  1

/** Register lists */
#define DRAW_XYZ_REGLIST \
	((u64)GIF_REG_XYZ2) << 0 | \
	((u64)GIF_REG_XYZ2) << 4

#define DRAW_RGBAQ_REGLIST \
	((u64)GIF_REG_RGBAQ) <<  0 | \
	((u64)GIF_REG_XYZ2)  <<  4

#define DRAW_UV_REGLIST \
	((u64)GIF_REG_UV)   << 0 | \
	((u64)GIF_REG_XYZ2) << 4

#define DRAW_RGBAQ_UV_REGLIST \
	((u64)GIF_REG_RGBAQ)  << 0 | \
	((u64)GIF_REG_UV)     << 4 | \
	((u64)GIF_REG_XYZ2)   << 8

#define DRAW_STQ_REGLIST \
	((u64)GIF_REG_RGBAQ)  << 0 | \
	((u64)GIF_REG_ST)     << 4 | \
	((u64)GIF_REG_XYZ2)   << 8

/** Alpha Blending */
#define BLEND_COLOR_SOURCE	0
#define BLEND_COLOR_DEST	1
#define BLEND_COLOR_ZERO	2

#define BLEND_ALPHA_SOURCE	0
#define BLEND_ALPHA_DEST	1
#define BLEND_ALPHA_FIXED	2

/** Alpha Correction */
#define ALPHA_CORRECT_RGBA32	0
#define ALPHA_CORRECT_RGBA16	1

/** Texture Color Components */
#define TEXTURE_COMPONENTS_RGB	0
#define TEXTURE_COMPONENTS_RGBA	1

/** Texture Function */
#define TEXTURE_FUNCTION_MODULATE	0
#define TEXTURE_FUNCTION_DECAL		1
#define TEXTURE_FUNCTION_HIGHLIGHT	2
#define TEXTURE_FUNCTION_HIGHLIGHT2	3

/** CLUT Storage Mode */
#define CLUT_STORAGE_MODE1	0
#define CLUT_STORAGE_MODE2	1

/** CLUT Load Control */
#define CLUT_NO_LOAD		0
#define CLUT_LOAD		1
#define CLUT_LOAD_COPY_CBP0	2
#define CLUT_LOAD_COPY_CBP1	3
#define CLUT_COMPARE_CBP0	4
#define CLUT_COMPARE_CBP1	5

/** Scan masking */
#define SCAN_MASK_NORMAL	0
#define SCAN_MASK_ODD		2
#define SCAN_MASK_EVEN		3

/** Color Clamping */
#define COLOR_CLAMP_MASK	0
#define COLOR_CLAMP_ENABLE	1

/** Scan masking */
#define SCAN_MASK_NORMAL	0
#define SCAN_MASK_ODD		2
#define SCAN_MASK_EVEN		3

/** Color Clamping */
#define COLOR_CLAMP_MASK	0
#define COLOR_CLAMP_ENABLE	1

/** Types */
#define PRIM_POINT		0x00
#define PRIM_LINE		0x01
#define PRIM_LINE_STRIP		0x02
#define PRIM_TRIANGLE		0x03
#define PRIM_TRIANGLE_STRIP	0x04
#define PRIM_TRIANGLE_FAN	0x05
#define PRIM_SPRITE		0x06

/** Shading */
#define PRIM_SHADE_FLAT		0
#define PRIM_SHADE_GOURAUD	1

/** Texture Mapping Coordinates */
#define PRIM_MAP_ST		0
#define PRIM_MAP_UV		1

/** Fixed Color Value */
#define PRIM_UNFIXED		0
#define PRIM_FIXED		1

/** Primitive Override Control */
#define PRIM_OVERRIDE_ENABLE	0
#define PRIM_OVERRIDE_DISABLE	1

/** Level of Detail */
#define LOD_FORMULAIC		0
#define LOD_USE_K		1

/** Texture Scaling */
#define LOD_MAG_NEAREST			0
#define LOD_MAG_LINEAR			1
#define LOD_MIN_NEAREST			0
#define LOD_MIN_LINEAR			1
#define LOD_MIN_NEAR_MIPMAP_NEAR	2
#define LOD_MIN_NEAR_MIPMAP_LINE	3
#define LOD_MIN_LINE_MIPMAP_NEAR	4
#define LOD_MIN_LINE_MIPMAP_LINE	5

/** Mipmaps */
#define LOD_MIPMAP_REGISTER	0
#define LOD_MIPMAP_CALCULATE	1

/** Texture wrapping */
#define WRAP_REPEAT		0
#define WRAP_CLAMP		1
#define WRAP_REGION_CLAMP	2
#define WRAP_REGION_REPEAT	3

/** Texture Alpha Expansion */
#define ALPHA_EXPAND_NORMAL		0
#define ALPHA_EXPAND_TRANSPARENT	1

/** Alpha Testing */
#define ATEST_METHOD_ALLFAIL		0
#define ATEST_METHOD_ALLPASS		1
#define ATEST_METHOD_LESS		2
#define ATEST_METHOD_LESS_EQUAL		3
#define ATEST_METHOD_EQUAL		4
#define ATEST_METHOD_GREATER_EQUAL	5
#define ATEST_METHOD_GREATER		6
#define ATEST_METHOD_NOTEQUAL		7

#define ATEST_KEEP_ALL			0
#define ATEST_KEEP_ZBUFFER		1
#define ATEST_KEEP_FRAMEBUFFER		2
#define ATEST_KEEP_ALPHA		3

/** Destination Alpha Testing */
#define DTEST_METHOD_PASS_ZERO		0
#define DTEST_METHOD_PASS_ONE		1

/** Depth Test */
#define ZTEST_METHOD_ALLFAIL		0
#define ZTEST_METHOD_ALLPASS		1
#define ZTEST_METHOD_GREATER_EQUAL	2
#define ZTEST_METHOD_GREATER		3

#ifdef __cplusplus
extern "C" {
#endif

typedef union {
	u64 xyz;
	struct {
		u16 x;
		u16 y;
		u32 z;
	};
} __attribute__((packed,aligned(8))) xyz_t;

typedef union {
	u64 uv;
	struct {
		float s;
		float t;
	};
	struct {
		float u;
		float v;
	};
} __attribute__((packed,aligned(8))) texel_t;

typedef union {
	u64 rgbaq;
	struct {
		u8 r;
		u8 g;
		u8 b;
		u8 a;
		float q;
	};
} __attribute__((packed,aligned(8))) color_t;

// Using shorts complicates things for normal usage
typedef struct {
	float x;
	float y;
	unsigned int z;
} vertex_t;

typedef union {
	VECTOR strq;
	struct {
		float s;
		float t;
		float r;
		float q;
	};
} __attribute__((packed,aligned(16))) texel_f_t;

typedef union {
	VECTOR rgba;
	struct {
		float r;
		float g;
		float b;
		float a;
	};
} __attribute__((packed,aligned(16))) color_f_t;

typedef union {
	VECTOR xyzw;
	struct {
		float x;
		float y;
		float z;
		float w;
	};
} __attribute__((packed,aligned(16))) vertex_f_t;

typedef struct {
	vertex_t v0;
	color_t  color;
} point_t;

typedef struct {
	vertex_t v0;
	vertex_t v1;
	color_t  color;
} line_t;

typedef struct {
	vertex_t v0;
	vertex_t v1;
	vertex_t v2;
	color_t  color;
} triangle_t;

typedef struct {
	vertex_t v0;
	vertex_t v1;
	color_t  color;
} rect_t;

typedef struct {
	vertex_t v0;
	texel_t  t0;
	vertex_t v1;
	texel_t  t1;
	color_t  color;
} texrect_t;

// color = (c1-c2)*a>>7 + c3
typedef struct {
	char color1;
	char color2;
	char alpha;
	char color3;
	unsigned char fixed_alpha;
} blend_t;

typedef struct {
	unsigned char width;
	unsigned char height;
	unsigned char components;
	unsigned char function;
} texinfo_t;

typedef struct {
	unsigned int address;
	unsigned int width;
	unsigned int height;
	unsigned int psm;
	unsigned int mask;
} framebuffer_t;

typedef struct {
	unsigned int enable;
	unsigned int method;
	unsigned int address;
	unsigned int zsm;
	unsigned int mask;
} zbuffer_t;

typedef struct {
	unsigned int address;
	unsigned int width;
	unsigned int psm;
	texinfo_t info;
} texbuffer_t;

typedef struct {
	unsigned int address;
	unsigned int psm;
	unsigned int storage_mode;
	unsigned int start;
	unsigned int load_method;
} clutbuffer_t;

typedef signed char dithermx_t[16];

typedef struct {
	unsigned char type;
	unsigned char shading;
	unsigned char mapping;
	unsigned char fogging;
	unsigned char blending;
	unsigned char antialiasing;
	unsigned char mapping_type;
	unsigned char colorfix;
} prim_t;

typedef struct {
	unsigned char calculation;
	unsigned char max_level;
	unsigned char mag_filter;
	unsigned char min_filter;
	unsigned char mipmap_select;
	unsigned char l;
	float k;
} lod_t;

typedef struct {
	int address1;
	int address2;
	int address3;
	char width1;
	char width2;
	char width3;
} mipmap_t;

typedef struct {
	unsigned char horizontal;
	unsigned char vertical;
	int minu, maxu;
	int minv, maxv;
} texwrap_t;

typedef struct {
	unsigned char enable;
	unsigned char method;
	unsigned char compval;
	unsigned char keep;
} atest_t;

typedef struct {
	unsigned char enable;
	unsigned char pass;
} dtest_t;

typedef struct {
	unsigned char enable;
	unsigned char method;
} ztest_t;


/** Sets up the drawing environment based on the framebuffer and zbuffer settings */
qword_t *draw_setup_environment(qword_t *q, int context, framebuffer_t *frame, zbuffer_t *z);

/** Clear the screen based on the screen's origin, width, and height using the defined color. **/
qword_t *draw_clear(qword_t *q, int context, float x, float y, float width, float height, int r, int g, int b);

/** Signal that drawing is finished */
qword_t *draw_finish(qword_t *q);

/** Wait until finish event occurs */
void draw_wait_finish(void);

/** Creates a dma chain filled with image information */
qword_t *draw_texture_transfer(qword_t *q, void *src, int width, int height, int psm, int dest, int dest_width);

/** Flush the texture cache */
qword_t *draw_texture_flush(qword_t *q);

/** Enables alpha blending */
void draw_enable_blending();

/** Disables alpha blending */
void draw_disable_blending();

/** Alpha Blending Per-Pixel MSB Control */
qword_t *draw_pixel_alpha_control(qword_t *q, int enable);

/** Alpha Blending */
qword_t *draw_alpha_blending(qword_t *q, int context, blend_t *blend);

/** Alpha Correction */
qword_t *draw_alpha_correction(qword_t *q, int context, int alpha);

/** Draws a single point */
qword_t *draw_point(qword_t *q, int context, point_t *point);

/** Draws a single line */
qword_t *draw_line(qword_t *q, int context, line_t *line);

/** Draws a triangle using a line strip */
qword_t *draw_triangle_outline(qword_t *q, int context, triangle_t *triangle);

/** Draws a single triangle */
qword_t *draw_triangle_filled(qword_t *q, int context,triangle_t *triangle);

/** Draws a rectangle using line primitives */
qword_t *draw_rect_outline(qword_t *q, int context, rect_t *rect);

/** Draws a single sprite */
qword_t *draw_rect_filled(qword_t *q, int context, rect_t *rect);

/** Draws a single texture mapped sprite */
qword_t *draw_rect_textured(qword_t *q, int context, texrect_t *rect);

/** Draws multiple sprite primitives */
qword_t *draw_rect_filled_strips(qword_t *q, int context, rect_t *rect);

/** Draws multiple strips to render a large texture
 * width must be multiple of 32 - 0.9375
 */
qword_t *draw_rect_textured_strips(qword_t *q, int context, texrect_t *rect);

/** Draws filled round rectangle, buggy with partial coordinates */
qword_t *draw_round_rect_filled(qword_t *q, int context, rect_t *rect);

/** Draws a hollow round rectangle, buggy with partial coordinates */
qword_t *draw_round_rect_outline(qword_t *q, int context, rect_t *rect);

/** Draws an arc using line primitives */
qword_t *draw_arc_outline(qword_t *q, int context, point_t *center, float radius, float angle_start, float angle_end);

/** Draws multiple triangle fans */
qword_t *draw_arc_filled(qword_t *q, int context, point_t *center, float radius, float angle_start, float angle_end);

/** Begins a primitive, allowing for vertex data to be filled in the packet directly */
qword_t *draw_prim_start(qword_t *q, int context, prim_t *prim, color_t *color);

/** Ends a primitive by calculating the number of qwords used, the number of registers, the register list */
qword_t *draw_prim_end(qword_t *q,int nreg, u64 reglist);

/** Converts floating point color, replacing alpha with constant value, and calculates q */
int draw_convert_rgbq(color_t *output, int count, vertex_f_t *vertices, color_f_t *colours, unsigned char alpha);

/** Converts floating point color and calculates q value */
int draw_convert_rgbaq(color_t *output, int count, vertex_f_t *vertices, color_f_t *colours);

/** Calculates the st coordinates from the perspective coordinate q = 1/w */
int draw_convert_st(texel_t *output, int count, vertex_f_t *vertices, texel_f_t *coords);

/** Converts and translates floating point vertices to fixed point vertices */
int draw_convert_xyz(xyz_t *output, float x, float y, int z, int count, vertex_f_t *vertices);

/** Returns the power of 2 needed for texture width and height */
unsigned char draw_log2(unsigned int x);

/** Framebuffer Attributes */
qword_t *draw_framebuffer(qword_t *q, int context, framebuffer_t *frame);

/** ZBuffer Attributes */
qword_t *draw_zbuffer(qword_t *q, int context, zbuffer_t *zbuffer);

/** TextureBuffer Attributes */
qword_t *draw_texturebuffer(qword_t *q, int context, texbuffer_t *texbuffer, clutbuffer_t *clut);

/** CLUT Storage Mode 1 Information */
qword_t *draw_clutbuffer(qword_t *q, int context, int psm, clutbuffer_t *clut);

/** CLUT Storage Mode 2 Information */
qword_t *draw_clut_offset(qword_t *q, int cbw, int u, int v);

/** Dithering Switch */
qword_t *draw_dithering(qword_t *q, int enable);

/** Dithering Matrix */
qword_t *draw_dither_matrix(qword_t *q, char *dm);

/** Fog Color */
qword_t *draw_fog_color(qword_t *q, unsigned char r, unsigned char g, unsigned char b);

/** Scanline Masking (framebuffer) */
qword_t *draw_scan_masking(qword_t *q, int mask);

/** Color Masking/Clamping */
qword_t *draw_color_clamping(qword_t *q, int enable);

/** Scanline Masking (framebuffer) */
qword_t *draw_scan_masking(qword_t *q, int mask);

/** Color Masking/Clamping */
qword_t *draw_color_clamping(qword_t *q, int enable);

/** Primitive Coordinate System offset */
qword_t *draw_primitive_xyoffset(qword_t *q, int context, float x, float y);

/** Primitive Control */
qword_t *draw_primitive_override(qword_t *q, int mode);

/** Overridden Primitive Attributes */
qword_t *draw_primitive_override_setting(qword_t *q, int context, prim_t *prim);

/** Texture Sampling, Level-of-Detail, and Filtering */
qword_t *draw_texture_sampling(qword_t *q, int context, lod_t *lod);

/** Mipmap levels 1-3 */
qword_t *draw_mipmap1(qword_t *q, int context, mipmap_t *mipmap);

/** Mipmap levels 4-6 */
qword_t *draw_mipmap2(qword_t *q, int context, mipmap_t *mipmap);

/** Texture Clamping */
qword_t *draw_texture_wrapping(qword_t *q, int context, texwrap_t *wrap);

/** Alpha Expansion Values */
qword_t *draw_texture_expand_alpha(qword_t *q, unsigned char zero_value, int expand, unsigned char one_value);

/** Scissoring pixel test area */
qword_t *draw_scissor_area(qword_t *q, int context, int x0, int x1, int y0, int y1);

/** Pixel Testing */
qword_t *draw_pixel_test(qword_t *q, int context, atest_t *atest, dtest_t *dtest, ztest_t *ztest);

/** Disable pixel testing defaults */
qword_t *draw_disable_tests(qword_t *q, int context, zbuffer_t *z);

/** Enable pixel testing defaults */
qword_t *draw_enable_tests(qword_t *q, int context, zbuffer_t *z);

#ifdef __cplusplus
}
#endif

#endif /* __DRAW_H__ */
