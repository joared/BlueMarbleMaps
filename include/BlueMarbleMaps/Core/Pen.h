#pragma once

#include "Color.h"
namespace BlueMarble
{
	class Pen
	{
	public:
		enum Properties
		{
			None,
			Double,
			Tracked,
			Dotted
		};

		static Pen transparent() { return Pen(Color::transparent(), 0.0); }
		Pen()
		{}
		Pen(Color color, double thickness, Properties properties = None, double offset = 0);
		Pen(std::vector<Color> colors, double thickness, Properties properties = None, double offset = 0);

		void setThickness(double thickness);
		double getThickness() const;

		void setOffset(double offset);
		double getOffset() const;

		void addColor(Color color);
		void setColors(std::vector<Color> colors);
		std::vector<Color> getColors() const;

		void setProperties(Properties properties);
		Properties getProperties() const;

		// Joar stuff
		const Color& getColor() const { return m_color; };
        void setColor(const Color& color) { m_color = color; };
        const Color& getFromColor() const { return m_colorFrom; };
        void setFromColor(const Color& color) { m_colorFrom = color; };
        
        bool getAntiAlias() const { return m_antiAlias; };
        void setAntiAlias(bool antiAlias) { m_antiAlias = antiAlias; };
        double getWidth() const { return m_width; }
        void setWidth(double width) { m_width = width; }
        double getFromWidth() const { return m_fromWidth; }
        void setFromWidth(double width) { m_fromWidth = width; }

	private:
		double m_thickness;
		double m_offset;
		std::vector<Color> m_colors;
		Properties m_properties;

		// Joar stuff
		bool m_antiAlias;
        Color m_color;
        Color m_colorFrom;
        double m_width;
        double m_fromWidth;
	};
}
