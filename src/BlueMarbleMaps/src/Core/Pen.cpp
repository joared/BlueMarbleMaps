#include "BlueMarbleMaps/Core/Pen.h"

namespace BlueMarble
{
	Pen::Pen(Color color, double thickness, Pen::Properties properties, double offset)
		: m_colors()
		, m_offset(offset)
		, m_thickness(thickness)
		, m_properties(properties)
	{
		m_colors.push_back(color);
	}
	Pen::Pen(std::vector<Color> colors, double thickness, Pen::Properties properties, double offset)
		: m_colors(colors)
		, m_offset(offset)
		, m_thickness(thickness)
		, m_properties(properties)
	{

	}

	void Pen::setThickness(double thickness)
	{
		m_thickness = thickness;
	}
	double Pen::getThickness() const
	{
		return m_thickness;
	}

	void Pen::setOffset(double offset)
	{
		m_offset = offset;
	}
	double Pen::getOffset() const
	{
		return m_offset;
	}

	void Pen::addColor(Color color)
	{
		m_colors.push_back(color);
	}
	void Pen::setColors(std::vector<Color> colors)
	{
		m_colors = colors;
	}
	std::vector<Color> Pen::getColors() const
	{
		return m_colors;
	}

	void Pen::setProperties(Pen::Properties properties)
	{
		m_properties = properties;
	}
	Pen::Properties Pen::getProperties() const
	{
		return m_properties;
	}
}
