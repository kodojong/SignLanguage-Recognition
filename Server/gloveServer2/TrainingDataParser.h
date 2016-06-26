#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
using std::ifstream;

class TrainingDataParser {
	char DatasetName[1024];				// Dataset Name
	char InfoText[1024];				// Info Text
	int NumDimensions;					// Dimension #
	int TotalNumTrainingExamples;		// Total Training Examples
	int NumberOfClasses;				// Classes #
	int *ClassIDsAndCounters;			// pair of classid - counter
	int UseExternalRanges;
public:
	TrainingDataParser();
	~TrainingDataParser();

	void ReadHeader(ifstream&);			// Read Header
	void ReadWriteTimeSeries(ifstream&, int);

	void Start(std::string, int);		// start Parsing
};