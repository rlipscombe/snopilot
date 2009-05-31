/* dib_transforms.h
*/

#ifndef DIB_TRANSFORMS_H
#define DIB_TRANSFORMS_H 1

#include <math.h>

namespace dib
{
	struct IdentityTransform
	{
		RGBQUAD operator() (RGBQUAD src) const
		{
			return src;
		}
	};

	struct RedMaskTransform
	{
		RGBQUAD operator() (RGBQUAD src) const
		{
			RGBQUAD result;

			result.rgbReserved = 0x00;
			result.rgbRed = src.rgbRed;
			result.rgbGreen = 0x00;
			result.rgbBlue = 0x00;

			return result;
		}
	};

	struct GreenMaskTransform
	{
		RGBQUAD operator() (RGBQUAD src) const
		{
			RGBQUAD result;

			result.rgbReserved = 0x00;
			result.rgbRed = 0x00;
			result.rgbGreen = src.rgbGreen;
			result.rgbBlue = 0x00;

			return result;
		}
	};

	struct BlueMaskTransform
	{
		RGBQUAD operator() (RGBQUAD src) const
		{
			RGBQUAD result;

			result.rgbReserved = 0x00;
			result.rgbRed = 0x00;
			result.rgbGreen = 0x00;
			result.rgbBlue = src.rgbBlue;

			return result;
		}
	};

	struct AlphaMaskTransform
	{
		RGBQUAD operator() (RGBQUAD src) const
		{
			RGBQUAD result;

			result.rgbReserved = 0x00;
			result.rgbRed = src.rgbReserved;
			result.rgbGreen = src.rgbReserved;
			result.rgbBlue = src.rgbReserved;

			return result;
		}
	};

	class GrayscaleTransform
	{
		const double m_red_fact;
		const double m_green_fact;
		const double m_blue_fact;

		const double m_gamma;

	public:
		GrayscaleTransform(double red_fact = 0.299, double green_fact = 0.587, double blue_fact = 0.114, double gamma = 0.5)
			: m_red_fact(red_fact), m_green_fact(green_fact), m_blue_fact(blue_fact), m_gamma(gamma)
		{
		}

		RGBQUAD operator() (RGBQUAD src) const
		{
			RGBQUAD result;

			double dGray = 0;

			// This appears to work, find out what other values do.	    
			dGray += src.rgbRed * m_red_fact;
			dGray += src.rgbGreen * m_green_fact;
			dGray += src.rgbBlue * m_blue_fact;

			BYTE value = (BYTE)(pow(dGray / 255.0, m_gamma) * 255.0);
			result.rgbBlue = value;
			result.rgbGreen = value;
			result.rgbRed = value;
			result.rgbReserved = src.rgbReserved;

			return result;
		}
	};

	class SaturationTransform
	{
		const double m_gamma;

	public:
		SaturationTransform(double gamma = 1.4)
			: m_gamma(gamma)
		{
		}

		RGBQUAD operator() (RGBQUAD src) const
		{
			RGBQUAD result;

			result.rgbBlue = ApplyGamma(src.rgbBlue);
			result.rgbGreen = ApplyGamma(src.rgbGreen);
			result.rgbRed = ApplyGamma(src.rgbRed);
			result.rgbReserved = src.rgbReserved;

			return result;
		}

		BYTE ApplyGamma(BYTE src) const
		{
			return (BYTE)(pow(src / 255.0, m_gamma) * 255.0);
		}
	};

	class ColourShiftTransform
	{
	protected:
		// Shift the channel down a little, making sure that we don't wrap.
		BYTE Downshift(BYTE b, BYTE down_by) const
		{
			if (b > down_by)
				return b - down_by;

			return 0;
		}

		// Shift the channel up a little, making sure that we saturate, rather than wrap.
		BYTE Upshift(BYTE b, BYTE up_by) const
		{
			if (b < (255 - up_by))
				return b + up_by;

			return 255;
		}
	};

	class RedShiftTransform : public ColourShiftTransform
	{
	public:
		RGBQUAD operator() (RGBQUAD src) const
		{
			RGBQUAD result;

			result.rgbBlue = Downshift(src.rgbBlue, 40);
			result.rgbGreen = Downshift(src.rgbGreen, 40);
			result.rgbRed = Upshift(src.rgbRed, 40);
			result.rgbReserved = src.rgbReserved;

			return result;
		}
	};

	class GreenShiftTransform : public ColourShiftTransform
	{
	public:
		RGBQUAD operator() (RGBQUAD src) const
		{
			RGBQUAD result;

			result.rgbBlue = Downshift(src.rgbBlue, 40);
			result.rgbGreen = Upshift(src.rgbGreen, 40);
			result.rgbRed = Downshift(src.rgbRed, 40);
			result.rgbReserved = src.rgbReserved;

			return result;
		}
	};

	class BlueShiftTransform : public ColourShiftTransform
	{
	public:
		RGBQUAD operator() (RGBQUAD src) const
		{
			RGBQUAD result;

			result.rgbBlue = Upshift(src.rgbBlue, 40);
			result.rgbGreen = Downshift(src.rgbGreen, 40);
			result.rgbRed = Downshift(src.rgbRed, 40);
			result.rgbReserved = src.rgbReserved;

			return result;
		}
	};
}; // namespace dib

#endif /* DIB_TRANSFORMS_H */

