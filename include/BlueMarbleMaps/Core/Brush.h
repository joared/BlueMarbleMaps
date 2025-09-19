#pragma once
#include "Color.h"

namespace BlueMarble
{
	class Brush
	{

	public:
		enum Properties
		{
			None,
			Material,
			Lighting,
			LOD
		};
		Brush(Color color, Properties properties = None);
		Brush(std::vector<Color> colors, Properties properties = None);

		void addColor(const Color& color);
		void setColors(const std::vector<Color>& colors);
		const std::vector<Color>& getColors() const;

		void setProperty(const Properties& properties);
		const Properties& getProperties() const;

	private:
		std::vector<Color> m_colors;
		Properties m_properties;
	};
}


