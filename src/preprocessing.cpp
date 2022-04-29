#include "preprocessing.h"

Mat getGrayscale(Bitmap& bitmap)
{
	// Init data
	int pixelSize = 3,
		w = bitmap.GetWidth(),
		h = bitmap.GetHeight();
	Rect area(0, 0, w, h);

	// Lock bits from bitmap area into system memory
	BitmapData bmpData;
	if (bitmap.LockBits(&area, ImageLockModeRead, PixelFormat24bppRGB, &bmpData) != Ok)
		return Mat(0);

	// Loop over image and convert to grayscale matrix
	Mat grayscale(h, vector<byte>(w));
	for (int i = 0; i < h; i++)
	{
		// Stride can be negative for bottom-up bitmaps
		byte* bytes = (byte*)bmpData.Scan0 + (i * bmpData.Stride);

		// Loop over scan line
		for (int j = 0; j < w; j++)
		{
			//Get each colour component
			byte r = bytes[pixelSize * j];
			byte g = bytes[pixelSize * j + 1];
			byte b = bytes[pixelSize * j + 2];

			// Conver to grayscale
			byte intensity = round(0.299 * r + 0.587 * g + 0.114 * b);

			// Assign grayscale matrix
			grayscale[i][j] = intensity;
		}
	}

	// Unlock bitmap from system memory
	bitmap.UnlockBits(&bmpData);

	return grayscale;
}

void threshold(Mat& I, byte thresh)
{
	for (auto& row : I)
		for (auto& px : row)
			if (px < thresh)
				px = 1;
			else
				px = 0;
}
