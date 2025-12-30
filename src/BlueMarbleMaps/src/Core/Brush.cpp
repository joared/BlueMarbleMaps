#include "BlueMarbleMaps/Core/Brush.h"

namespace BlueMarble
{
	Brush::Brush(Color color, Brush::Properties properties)
		: m_colors()
		, m_properties(properties)
	{
		m_colors.push_back(color);
	}

	Brush::Brush(std::vector<Color> colors, Brush::Properties properties)
		: m_colors(colors)
		, m_properties(properties)
	{
	}

	void Brush::addColor(const Color& color)
	{
		m_colors.push_back(color);
	}

	void Brush::setColors(const std::vector<Color>& colors)
	{
		m_colors = colors;
	}

	const std::vector<Color>& Brush::getColors() const
	{
		return m_colors;
	}

	void Brush::setProperty(const Brush::Properties& properties)
	{
		m_properties = properties;
	}

	const Brush::Properties& Brush::getProperties() const
	{
		return m_properties;
	}

	void Brush::setColor(const Color &color)
	{
		m_colors = { color };
	}

}
