#include "segmentation.h"

// Axises of image
typedef bool Axis;
#define Ox true
#define Oy false


/* Get projection profile of image region
* Ox - horizontal profile(sums along rows),
* Oy - vertical profile(sums along columns)
* 
* param I: Matrix of image
* param r: Region of image
* param axis: Direction of summation
* returns: Profile projection
*/
vector<int> getProfile(const Mat& I, Rect r, Axis axis)
{
	// Init profile vector
	int size = axis == Ox ? r.Height : r.Width;
	vector<int> pp(size);

	int m = r.Y + r.Height, n = r.X + r.Width;
	// Loop along y-axis
	for (int i = r.Y; i < m; i++)
	{
		// Loop along x-axis
		for (int j = r.X; j < n; j++)
		{
			// Get index of current element in vector
			int idx = axis == Ox ? i - r.Y : j - r.X;

			// Update sum
			pp[idx] += I[i][j];
		}
	}
	return pp;
}

/* Segmentation along given axis
* param I: Matrix of image
* param r: Image bounds
* param axis: Axis
* returns: Bounds of found images
*/
vector<Rect> splitBound(const Mat& I, Rect r, Axis axis)
{
	Rect bound;
	vector<Rect> bounds;

	// Get profile projection
	auto pp = getProfile(I, r, !axis);

	// Init coordinates of text segment and size of profile
	int start = -1, end, n = pp.size();

	// Loop over profile projection
	for (int i = 0; i < n; i++)
	{
		if (pp[i] && start == -1)
			start = i; // Save start coordinate of text segment
		else if (pp[i] == 0 && start != -1)
		{
			end = i - 1; // Save end coordinate of text segment

			// Init bounding box
			bound = r;
			if (axis == Ox)
			{
				// Change x-coordinates
				bound.X = start;
				bound.Width = end - start;
			}
			else
			{
				// Change y-coordinates
				bound.Y = start;
				bound.Height = end - start;
			}

			// Push to collection of all bounds
			bounds.push_back(bound);

			// Reset start of text segment
			start = -1;
		}
	}

	// Image ends with black pixels
	if (start != -1)
	{
		end = n - 1; // Set end coordinate of segment

		// Init bound box
		bound = r;
		if (axis == Ox)
		{
			// Change x-coordinates
			bound.X = start;
			bound.Width = end - start;
		}
		else
		{
			// Change y-coordinates
			bound.Y = start;
			bound.Height = end - start;
		}

		// Push to collection of all bounds
		bounds.push_back(bound);
	}
	return bounds;
}

/*Calculates mean size of bounds along given axis
* param images: 1D vector of bounds
* param axis: Given axis
* returns: Mean height for Oy and width for Ox
*/
int getMeanSize(const vector<Rect>& bounds, Axis axis)
{
	int sum = 0, size;
	for (auto& bound : bounds)
	{
		size = axis == Ox ? bound.Width : bound.Height; // Get size along axis
		sum += size; // Add size to sum
	}

	// Max with 1 is used for zero division
	return sum / max(1, bounds.size());
}

/*Group bounding boxes
* param I: Matrix of image
* param letters: 1D vecotr of bounds
* returns: 2D vector of bounds
*/
vector<vector<Rect>> groupBounds(const Mat& I,
	const vector<Rect>& bounds)
{
	// Init groups
	vector<vector<Rect>> groups;
	if (bounds.size() == 0)
		return groups;
	groups.push_back(vector<Rect>());

	// Get threshold of distance (min space size)
	int thresh = (int)(0.7 * getMeanSize(bounds, Ox));

	// Iterate over bounds until pre-last image
	for (auto it = bounds.begin(); (it + 1) != bounds.end(); it++)
	{
		// Get distance between bounds
		Rect bounds = *it, nextBounds = *(it + 1);
		int distance = nextBounds.X - bounds.X - bounds.Width;

		// Add another bound to the group
		groups[groups.size() - 1].push_back(*it);
		if (distance >= thresh)
		{
			groups.push_back(vector<Rect>()); // Create a new group
		}
	}

	// Add last bound to the group
	groups[groups.size() - 1].push_back(bounds[bounds.size() - 1]);

	return groups;
}

/*Filters blank rows from bounding boxes
* param I: Matrix of image
* param rect: Bounding boxes
*/
void cropRows(const Mat& I, vector<Rect>& rects)
{
	for (auto& r : rects)
	{

		// Get projection profile
		auto hpp = getProfile(I, r, Ox);

		// Skip top white rows
		int i = 0;
		while (i < hpp.size() && hpp[i] == 0)
		{
			r.Y++;
			r.Height--;
			i++;
		}

		// Skip bottom white rows
		i = hpp.size() - 1;
		while (i >= 0 && hpp[i] == 0)
		{
			r.Height--;
			i--;
		}
	}
}

/* Erases small bounds from vector
* param bounds: Bounds to filter
* param axis: Axis to get size
* para, threshold: Threshold of size filtering
*/
void filterBounds(vector<Rect>& bounds, 
	Axis axis, 
	int threshold)
{
	for (auto it = bounds.begin(); it != bounds.end();)
	{
		int size = axis == Ox ?	it->Width : it->Height; // Get size 
		if (size < threshold)
			it = bounds.erase(it); // Filter
		if (it != bounds.end()) // Avoid incrementing vector's end
			it++;
	}
}

Bounds getBounds(const Mat& I)
{
	Bounds bounds;
	if (I.size() == 0 || I[0].size() == 0)
		return bounds;

	// Extract lines, words and letters
	auto lines = splitBound(I, Rect(0, 0, I[0].size(), I.size()), Oy);
	filterBounds(lines, Oy, 0.3 * getMeanSize(lines, Oy));
	for (auto& line : lines)
	{
		auto letters = splitBound(I, line, Ox);
		cropRows(I, letters);
    int meanWidth = getMeanSize(letters, Ox);
		auto words = groupBounds(I, letters);
		for (auto& subLetters : words)
		{
			filterBounds(subLetters, Ox, 0.6 * meanWidth);
			filterBounds(subLetters, Oy, 0.6 * meanWidth);
		}
		if (words.size() > 0)
			bounds.push_back(words);
	}
	return bounds;
}
