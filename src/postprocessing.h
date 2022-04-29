#pragma once
#include "framework.h"

/* Correct given text by dictionary
* param text: Given text with misspellings
* param dict: Given dictionary with correct words
* returns: Corrected text
*/
wstring correctText(wstring text, const vector<wstring>& dict);
