#ifndef XPM_H
#define XPM_H

#include <map>
#include <string>

class Xpm
{
public:
	struct Color
	{
		uint8_t r = 0;
		uint8_t g = 0;
		uint8_t b = 0;

		static Color Black;

		Color(uint8_t r = 0, uint8_t g = 0, uint8_t b = 0) : r(r), g(g), b(b) {}

		bool operator==(const Color &o) const {return r == o.r && g == o.g && b == o.b;}

		static Color fromString(const std::string &def);
	};
public:
	Xpm();
	int width() const {return m_width;}
	int height() const {return m_height;}
	const Color& colorAt(int row, int col) const;
	bool isValid() const {return m_width > 0 && m_height > 0 && m_colors.size() > 0;}

	void wrapData(const char *data[]);
private:
	using ColorKey = unsigned int;
	ColorKey colorKeyAt(const char *pc) const;
private:
	int m_width = 0;
	int m_height = 0;
	std::size_t m_colorKeyLength = 0;
	std::map<ColorKey, Color> m_colors;
	const char **m_pixels = nullptr;
};

#endif // XPM_H
