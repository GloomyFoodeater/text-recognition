#pragma once
#include "framework.h"

/* Extracts grayscale matrix from bitmap
* 
* param bitmap: Source bitmap
* returns: 2D matrix of grayscale intensities
*/
Mat getGrayscale(Bitmap& image);

/* Binarize image matrix 
* set 1 for all that lesser than threshold and
* 0 for all that are higher or equal
*
* param m: Grayscale matrix of image
* param thresh: Threshold for binarization
*/
void threshold(Mat& I, byte thresh);

