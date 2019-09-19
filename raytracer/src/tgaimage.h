#ifndef __IMAGE_H__
#define __IMAGE_H__

#include <fstream>

#pragma pack(push,1)
struct TGA_Header {
	char idlength;
	char colormaptype;
	char datatypecode;
	short colormaporigin;
	short colormaplength;
	char colormapdepth;
	short x_origin;
	short y_origin;
	short width;
	short height;
	char  bitsperpixel;
	char  imagedescriptor;
};
#pragma pack(pop)

struct TGAColor {
	unsigned char bgra[4];
	unsigned char bytespp;

	TGAColor() : bgra(), bytespp(1) {
		for (int i = 0; i < 4; i++) bgra[i] = 0;
	}

	TGAColor(unsigned char R, unsigned char G, unsigned char B, unsigned char A = 255) : bgra(), bytespp(4) {
		bgra[0] = B;
		bgra[1] = G;
		bgra[2] = R;
		bgra[3] = A;
	}

	TGAColor(unsigned char v) : bgra(), bytespp(1) {
		for (int i = 0; i < 4; i++) bgra[i] = 0;
		bgra[0] = v;
	}


	TGAColor(const unsigned char* p, unsigned char bpp) : bgra(), bytespp(bpp) {
		for (int i = 0; i < (int)bpp; i++) {
			bgra[i] = p[i];
		}
		for (int i = bpp; i < 4; i++) {
			bgra[i] = 0;
		}
	}

	unsigned char& operator[](const int i) { return bgra[i]; }

	TGAColor operator *(float intensity) const {
		TGAColor res = *this;
		intensity = (intensity > 1.f ? 1.f : (intensity < 0.f ? 0.f : intensity));
		for (int i = 0; i < 4; i++) res.bgra[i] = bgra[i] * intensity;
		return res;
	}

	static void lerp(const TGAColor& lhs, const TGAColor& rhs, float lerpValue, TGAColor * resultOut) {

		lerpValue = (lerpValue > 1.f ? 1.f : (lerpValue < 0.f ? 0.f : lerpValue));
		for (int i = 0; i < 4; i++) (*resultOut)[i] = lhs.bgra[i] * lerpValue + (1 - lerpValue) * rhs.bgra[i];
	}
};

namespace Colors
{
	const TGAColor black = TGAColor(0, 0, 0, 255);
	const TGAColor blue = TGAColor(0, 0, 255, 255);
	const TGAColor skyBlue = TGAColor(135, 206, 235, 255);
	const TGAColor clear = TGAColor(0, 0, 0, 0);
	const TGAColor cyan = TGAColor(0, 255, 255, 255);
	const TGAColor gray = TGAColor(122, 122, 122, 255);
	const TGAColor green = TGAColor(0, 255, 0, 255);
	const TGAColor magenta = TGAColor(255, 0, 255, 255);
	const TGAColor red = TGAColor(255, 0, 0, 255);
	const TGAColor white = TGAColor(255, 255, 255, 255);
	const TGAColor yellow = TGAColor(255, 235, 4, 255);
}

class TGAImage {
protected:
	unsigned char* data;
	int width;
	int height;
	int bytespp;

	bool   load_rle_data(std::ifstream& in);
	bool unload_rle_data(std::ofstream& out);
public:
	enum Format {
		GRAYSCALE = 1, RGB = 3, RGBA = 4
	};

	TGAImage();
	TGAImage(int w, int h, int bpp);
	TGAImage(const TGAImage& img);
	bool read_tga_file(const char* filename);
	bool write_tga_file(const char* filename, bool rle = true);
	bool flip_horizontally();
	bool flip_vertically();
	bool scale(int w, int h);
	TGAColor get(int x, int y);
	bool set(int x, int y, TGAColor& c);
	bool set(int x, int y, const TGAColor& c);
	~TGAImage();
	TGAImage& operator =(const TGAImage& img);
	int get_width();
	int get_height();
	int get_bytespp();
	unsigned char* buffer();
	void clear();
};

#endif //__IMAGE_H__
