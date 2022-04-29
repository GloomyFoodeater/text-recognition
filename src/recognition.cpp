#include "recognition.h"

/* Creates new matrix from submatrix
* param src: Source matrix to get info from
* param r: Region to copy
* returns: Copied submatrix
*/
Mat copyMat(const Mat& src, Rect r)
{
	Mat dst(r.Height, vector<byte>(r.Width));
	for (int i = r.Y; i < r.Y + r.Height; i++)
		for (int j = r.X; j < r.X + r.Width; j++)
			dst[i - r.Y][j - r.X] = src[i][j];
	return dst;
}

/* Pixelwise comparison of 2 images
* Images can have different sizes:
* scaling is done in place when accessing 
* another pixel
* param X: 1st matrix
* param Y: 2nd matrix
* returns: Similiarity rate in [0, 1] section
*/
double match(const Mat& X, const Mat& Y)
{
	int h = X.size(), w = X[0].size(), matches = 0;
	double yScale = Y.size() / (double)h, xScale = Y[0].size() / (double)w;
	for (int i = 0; i < h; i++)
	{
		for (int j = 0; j < w; j++)
		{
			if (X[i][j] == Y[(int)(yScale * i)][(int)(xScale * j)])
				matches++;
		}
	}
	return (double)matches / w / h;
}

/* Classify image by given templates and labels
* param I: Matrix of image
* param labels: Labels of each template
* param templates: Templates to compare image with
* returns: Index of max match in labels or -1 if it was low
*/
int classify(const Mat& I,
	wstring labels,
	const vector<Mat>& templates)
{
	double max = 0;
	int maxIdx = -1;
	for (int i = 0; i < templates.size(); i++)
	{
		double similiarity = match(I, templates[i]);
		if (similiarity > max && similiarity > 0.6)
		{
			maxIdx = i;
			max = similiarity;
		}
	}
	return maxIdx;
}

wstring recognise(const Mat& I,
	const Bounds& bounds,
	wstring labels,
	const vector<Mat>& templates)
{
	wstring text;
	for (int i = 0; i < bounds.size(); i++)
	{
		for (int j = 0; j < bounds[i].size(); j++)
		{
			for (int k = 0; k < bounds[i][j].size(); k++)
			{
				Mat letterImage = copyMat(I, bounds[i][j][k]);
				int idx = classify(letterImage, labels, templates);
				if (idx >= 0)
					text += labels[idx];
			}
			text += L" ";
		}
		text += L"\n";
	}
	return text;
}
