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

	private:
		double m_thickness;
		double m_offset;
		std::vector<Color> m_colors;
		Properties m_properties;
	};
}
