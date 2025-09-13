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
		Brush()
		{
		}
		Brush(Color color, Properties properties = None);
		Brush(std::vector<Color> colors, Properties properties = None);

		void addColor(const Color& color);
		void setColors(const std::vector<Color>& colors);
		const std::vector<Color>& getColors() const;

		void setProperty(const Properties& properties);
		const Properties& getProperties() const;

		// Joar stuff
		static Brush transparent() { return Brush(); }
        const Color& getColor() const { return m_color; };
        void setColor(const Color& color) { m_color = color; };
        bool getAntiAlias() const { return m_antiAlias; };
        void setAntiAlias(bool antiAlias) { m_antiAlias = antiAlias; };


	private:
		std::vector<Color> m_colors;
		Properties m_properties;

		// Joar stuff
		bool m_antiAlias = false;
        Color m_color = Color::transparent();

	};
}


