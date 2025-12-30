#include "BlueMarbleMaps/Core/Pen.h"

namespace BlueMarble
{
	Pen::Pen()
		: Pen(Color::black(), 1.0)
	{

	}

    Pen::Pen(Color color, double thickness, Pen::Properties properties, double offset)
        : m_colors()
        , m_offset(offset)
        , m_thickness(thickness)
        , m_properties(properties)
		, m_antiAlias(false)
    {
		m_colors.push_back(color);
	}
	Pen::Pen(std::vector<Color> colors, double thickness, Pen::Properties properties, double offset)
		: m_colors(colors)
		, m_offset(offset)
		, m_thickness(thickness)
		, m_properties(properties)
		, m_antiAlias(false)
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

	 void Pen::setColor(const Color& color)
	 {
		m_colors = { color };
	 }

     const Color& Pen::getColor() const
     {
         if (m_colors.empty())
		 {
			return Color::black();
		 }

		 return m_colors[0];
     }

     const std::vector<Color>& Pen::getColors() const
     {
         return m_colors;
     }

	void Pen::setAntiAlias(bool antiAlias)
	{
		m_antiAlias = antiAlias;
	}
	
	bool Pen::getAntiAlias() const
	{
		return m_antiAlias;
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
