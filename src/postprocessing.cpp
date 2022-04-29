#include "postprocessing.h"
#define minimum(a, b, c) ( min( (a), min( (b), (c) ) ) )
#include <cwctype>

/* Calculate Levenstein distance of 2 strings
* param s1: First string
* param s2: Second string
* returns: Edit distance
*/
int lev(wstring s1, wstring s2)
{
	int m = s1.size(), n = s2.size(); // Sizes of matrix
	vector<int> v0(n + 1), v1(n + 1); // Previous and current rows

	// Init 1st row of matrix
	for (size_t i = 0; i < n + 1; i++)
		v0[i] = i;

	for (size_t i = 0; i < m; i++)
	{
		v1[0] = i + 1; // Init 1st column of matrix
		for (size_t j = 0; j < n; j++)
		{
			// Calculate costs of transformations
			int del = v0[j + 1] + 1;
			int ins = v1[j] + 1;
			int sub = v0[j] + (s1[i] != s2[j]);

			// Calculate next element
			v1[j + 1] = minimum(del, ins, sub);
		}
		swap(v0, v1); // Set previous row to current
	}
	return v0[n];
}

/* Split string into tokens by delimiter
* param text: Given string
* param delim: Given delimiter
* returns: 1D vector of tokens
*/
vector<wstring> split(wstring text, wchar_t delim)
{
	wstring token;
	vector<wstring> tokens;
	wistringstream ss(text);
	while (getline(ss, token, delim))
		tokens.push_back(token);
	return tokens;
}

/* Change each letter's case in lower case
* param word: Given string to convert
* returns: Same string in lowercase
*/
wstring towlower(wstring s)
{
	for (auto& letter : s)
		letter = towlower(letter);
	return s;
}

wstring correctText(wstring text, const vector<wstring>& dict)
{
	setlocale(LC_ALL, "ru-RU"); // Set locale for changing letter case
	wstring correctedText; // Output
	vector<wstring> words, lines; // Substrings of text
	lines = split(text, L'\n'); // Split text into lines
	for (const auto& line : lines)
	{
		words = split(line, L' '); // Split line into words
		for (auto& word : words)
		{
			word = towlower(word); // Convert to lower case
			
			// Find min Levenshtein distance
			int minDist = MAXINT, minIdx = -1;
			for (int k = 0; k < dict.size(); k++)
			{
				int dist = lev(word, towlower(dict[k]));
				if (dist < minDist)
				{
					minDist = dist;
					minIdx = k;
				}
			}

			// Correct word if distance is sufficient
			if (minIdx >= 0 && minDist <= word.size() / 2)
				word = dict[minIdx];
		
			correctedText += word + L" ";
		}
		correctedText += L"\n";
	}
	return correctedText;
}
