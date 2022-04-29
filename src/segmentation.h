#pragma once
#include "framework.h"

/* Extract letter bounding boxes from image
* param I: Matrix of image
* returns: 3D vector of bounding boxes of letters
*/
Bounds getBounds(const Mat& I);
