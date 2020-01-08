#include"clr_utils.h"

Bitmap ^textureToBitmap(Texture t)
{
	// Intialize the bitmap
	Bitmap ^bm = gcnew Bitmap(t.width, t.height);
	// For each pixel
	for (unsigned int x = 0; x < t.width; x++)
	{
		for (unsigned int y = 0; y < t.height; y++)
		{
			// Determine the index and color
			unsigned int index = t.indices[y * t.width + x], argb, paletteIndex = t.paletteIndices[y * t.width + x];
			KHColor c;
			if (paletteIndex < t.palette.colors.size() && index < t.palette.colors[paletteIndex].size())
			{
				c = t.palette.colors[paletteIndex][index];
			}
			else
			{
				c.r = 0;
				c.g = 0;
				c.b = 0;
				c.a = 0;
			}
			argb = (c.a << 24) | (c.r << 16) | (c.g << 8) | c.b;
			// Set the pixel to the color
			bm->SetPixel(x, y, System::Drawing::Color::FromArgb(argb));
		}
	}
	return bm;
}
