#ifndef SHV_XPM_H
#define SHV_XPM_H

#include "shvglobal.h"

#include <map>
#include <string>

namespace shv {

//template<class T, size_t N>
//constexpr size_t array_size(T (&)[N]) { return N; }

class SHV_DECL_EXPORT Xpm
{
public:
	struct SHV_DECL_EXPORT Color
	{
		uint8_t r = 0;
		uint8_t g = 0;
		uint8_t b = 0;

		static Color Black;
		static Color White;
		static Color Red;
		static Color Green;
		static Color Blue;
		static Color Magenta;
		static Color Cyan;
		static Color Yellow;

		Color() {}

		bool operator==(const Color &o) const {return r == o.r && g == o.g && b == o.b;}
		//bool operator<(const Color &o) const {return r < o.r && g < o.g && b < o.b;}
		bool operator<(const Color &o) const {return toInt() < o.toInt();}

		int toInt() const {int i = (((int)r) << 16) + (((int)g) << 8) + b; return i;}

		static Color fromRGB(uint8_t r, uint8_t g, uint8_t b) {Color ret; ret.r = r; ret.g = g; ret.b = b; return ret;}
		static Color fromString(const std::string &def);
	};
public:
	Xpm();
	int width() const {return m_width;}
	int height() const {return m_height;}
	const Color& colorAt(int row, int col) const;
	bool isValid() const {return m_width > 0 && m_height > 0 && m_colors.size() > 0;}

	static Xpm fromWrappedData(const char *data[]) {Xpm ret; ret.wrapData(data); return ret;}
	void wrapData(const char *data[]);
private:
	using ColorKey = unsigned int;
	static ColorKey colorKeyAt(const char *pc, int key_len);
protected:
	int m_width = 0;
	int m_height = 0;
	std::size_t m_colorKeyLength = 0;
	std::map<ColorKey, Color> m_colors;
	const char **m_pixels = nullptr;
};

class SHV_DECL_EXPORT XpmAtlas : public Xpm
{
	using Super = Xpm;
public:
	XpmAtlas() : Super() {}

	template<class T, size_t N>
	static XpmAtlas fromWrappedData(T (&data)[N]) {return fromWrappedData(data, N);}
	static XpmAtlas fromWrappedData(const char *data[], int data_size) {XpmAtlas ret; ret.wrapData(data, data_size); return ret;}
	void wrapData(const char *data[], int data_size);

	int glyphCount() const {return (width() / glyphWidth()) * (height() / glyphHeight());}
	int glyphWidth() const {return m_glyphWidth;}
	int glyphHeight() const {return m_glyphHeight;}
protected:
	int m_glyphWidth = 8;
	int m_glyphHeight = 8;
};

class SHV_DECL_EXPORT XpmFont : public XpmAtlas
{
	using Super = XpmAtlas;
public:
	XpmFont() : Super() {}

	template<class T, size_t N>
	static XpmFont fromWrappedData(T (&data)[N]) {return fromWrappedData(data, N);}
	static XpmFont fromWrappedData(const char *data[], int data_size) {XpmFont ret; ret.wrapData(data, data_size); return ret;}
	void wrapData(const char *data[], int data_size);

	int firstGlyphAscii() const {return m_firstGlyphAscii;}
protected:
	int m_firstGlyphAscii = 0;
};

}

#endif // XPM_H
