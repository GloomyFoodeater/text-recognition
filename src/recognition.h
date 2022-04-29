#pragma once
#include "framework.h"

/* Recognises letters from image regions and
* combines them into 1 string
* param I: Matrix of image
* param bounds: Bounds of letters
* param labels: Labels of each template
* param templates: Templates to compare image with
* returns: Extracted text
*/
wstring recognise(const Mat& I, 
	const Bounds& bounds,
	wstring labels, 
	const vector<Mat>& templates);

