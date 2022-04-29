
// Check whether (x, y) is inside region of interest
#define isInBounds(roi, x, y) ((roi).X <= (x) && (x) < (roi).X + (roi).Width &&\
							   (roi).Y <= (y) && (y) < (roi).Y + (roi).Height)

// Convert RBG pixel color to grayscale intensity
#define RGB2GRAY(c) ((int)(0.299 * (c).GetR() + 0.587 * (c).GetGreen() + 0.114 * (c).GetBlue()))

constexpr auto binThreshold = 150;	// Threshold for simple binarization

TextImage::TextImage(Bitmap& img)
{
	binarize(&img, 150);

	// Copy image matrix
	mat_ = &img;
	// Set region of interest
	roi_.X = 0;
	roi_.Y = 0;
	roi_.Width = img.GetWidth();
	roi_.Height = img.GetHeight();
}

TextImage::TextImage(const TextImage& img, Rect roi)
{
	// Shallow copy of input image
	mat_ = img.mat_;

	// Set region of interest
	roi_ = roi;
}

void TextImage::setBoundRect(Rect rect)
{
	roi_ = rect;
}

Rect TextImage::getBoundRect()
{
	return roi_;
}

inline int TextImage::getPixel(int x, int y)
{
	if (isInBounds(roi_, x, y))
	{
		Color color;
		if (mat_->GetPixel(x, y, &color) == Ok)
			return RGB2GRAY(color) < binThreshold ? 1 : 0;
	}

	// Return background pixel if
	// (x, y) is outside of roi
	return 0;
}

vector<double> TextImage::getFeatureVector(size_t size)
{
	size_t w = size, h = size;
	vector<double> features(h * w);
	if (roi_.Width != w || roi_.Height != h)
	{
		Bitmap clone(w, h);
		Graphics g(&clone);
		g.TranslateTransform((w - roi_.Width) / 2, (h - roi_.Height) / 2);
		//g.ScaleTransform(w / (REAL)roi_.Width, h / (REAL)roi_.Height);
		g.Clear(Color(255, 255, 255));
		g.DrawImage(mat_, 0, 0, roi_.X, roi_.Y, roi_.Width, roi_.Height, Unit::UnitPixel);				CLSID id;
		//CLSIDFromString(pngCLSID, &id);
		//wstring fileName = L"test\\Test" + to_wstring(CT) + L".png";
		//clone.Save(fileName.c_str(), &id, NULL);
		TextImage textImg(clone);
		return textImg.getFeatureVector(size);
	}
	for (size_t y = 0; y < h; y++)
	{
		for (size_t x = 0; x < w; x++)
		{
			features[y * w + x] = getPixel(x, y);
		}
	}
	return features;
}

vector<int> TextImage::getProfile(Axis axis)
{
	// Init profile vector
	int size = axis == Axis::x ? roi_.Height : roi_.Width;
	vector<int> profileProjection(size, 0);

	// Loop over x-axis
	for (int y = roi_.Y; y < roi_.Y + roi_.Height; y++)
	{
		// Loop over y-axis
		for (int x = roi_.X; x < roi_.X + roi_.Width; x++)
		{
			// Get index of current element in vector
			int idx = axis == Axis::x ? y - roi_.Y : x - roi_.X;

			// Include foreground pixel
			profileProjection[idx] += getPixel(x, y);
		}
	}
	return profileProjection;
}

inline int getPixel(Bitmap& I, int x, int y)
{
	Color c;
	if (I.GetPixel(x, y, &c) == Ok)
		return RGB2GRAY(c) == 0 ? 1 : 0;
	else
		return 0;
}

inline void setPixel(Bitmap& I, int c, int x, int y)
{
	I.SetPixel(x, y, Color(255, 255 * c, 255 * c, 255 * c));
}

void binarize(Bitmap& I, int threshold)
{
	Color c;
	size_t m = I.GetHeight(), n = I.GetWidth();
	for (size_t i = 0; i < m; i++)
	{
		for (size_t j = 0; j < n; j++)
		{
			I.GetPixel(j, i, &c);
			if (RGB2GRAY(c) > threshold)
				setPixel(I, 0, j, i); // White color
			else
				setPixel(I, 1, j, i); // Black color
		}
	}
}
