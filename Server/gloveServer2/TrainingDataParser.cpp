#include "TrainingDataParser.h"
#include <string>
using std::string;
using std::cout;
using std::endl;
using std::stringstream;

TrainingDataParser::TrainingDataParser() {
	memset(DatasetName, 0, sizeof(DatasetName));
	memset(InfoText, 0, sizeof(InfoText));
	NumDimensions = 0;
	TotalNumTrainingExamples = 0;
	NumberOfClasses = 0;
	ClassIDsAndCounters = nullptr;
	UseExternalRanges = 0;
}

TrainingDataParser::~TrainingDataParser() {
	if (ClassIDsAndCounters != nullptr)
		delete ClassIDsAndCounters;
}

void TrainingDataParser::ReadHeader(std::ifstream& stream) {
	char buffer[1024];
	while(true){
		memset(buffer, 0, 1024);
		stream.getline(buffer, 1024);

		stringstream ss(buffer);
		string token;

		getline(ss, token, ':');
		if (strcmp(token.c_str(), "") == 0) break;

		if (strcmp(token.c_str(), "DatasetName") == 0) {
			getline(ss, token, ':');
			memcpy(DatasetName, token.c_str(), strlen(token.c_str()));
			//cout << "DatasetName :" << DatasetName << endl;
		}
		else if (strcmp(token.c_str(), "InfoText") == 0) {
			getline(ss, token, ':');
			memcpy(InfoText, token.c_str(), strlen(token.c_str()));
			//cout << "InfoText :" << InfoText << endl;
		}
		else if (strcmp(token.c_str(), "NumDimensions") == 0) {
			getline(ss, token, ':');
			token.erase(token.find_last_not_of(" ") + 1);
			NumDimensions = atoi(token.c_str());
			//cout << "NumDimensions : " << NumDimensions << endl;
		}
		else if (strcmp(token.c_str(), "TotalNumTrainingExamples") == 0) {
			getline(ss, token, ':');
			token.erase(token.find_last_not_of(" ") + 1);
			TotalNumTrainingExamples = atoi(token.c_str());
			//cout << "TotalNumTrainingExamples : " << TotalNumTrainingExamples << endl;
		}
		else if (strcmp(token.c_str(), "NumberOfClasses") == 0) {
			getline(ss, token, ':');
			token.erase(token.find_last_not_of(" ") + 1);
			NumberOfClasses = atoi(token.c_str());
			//cout << "NumberOfClasses : " << NumberOfClasses << endl;
		}
		else if (strcmp(token.c_str(), "ClassIDsAndCounters") == 0) {
			int tmp = 0;
			ClassIDsAndCounters = new int[NumberOfClasses];
			//cout << "ClassIDsAndCounters [";
			for (int i = 0;i < NumberOfClasses;i++) {
			//	if (i != 0) cout << ", ";
				memset(buffer, 0, 1024);
				stream >> buffer;	//	remove class id
				stream >> buffer;
				ClassIDsAndCounters[i] = atoi(buffer);
				char buffer2[1024];
				memset(buffer2, 0, 1024);
				stream >> buffer2;
			//	cout << ClassIDsAndCounters[i];
			}
			//cout << "]" << endl;
		}
		else if (strcmp(token.c_str(), "UseExternalRanges") == 0) {
			getline(ss, token, ':');
			token.erase(token.find_last_not_of(" ") + 1);
			UseExternalRanges = atoi(token.c_str());
			//cout << "UseExternalRanges : " << UseExternalRanges << endl;
		}
		else if (strcmp(token.c_str(), "LabelledTimeSeriesTrainingData") == 0) {
			return;
		}
	}
}

void TrainingDataParser::ReadWriteTimeSeries(std::ifstream& in, int userID) {
	char buffer[1024];
	memset(buffer, 0, 1024);
	int TimeSeriesLength = 0;
	int ClassID = 0;
	string tableName = "training";
	tableName += std::to_string(userID);
	tableName += ".txt";
	std::ofstream out(tableName);
	while (in.getline(buffer, 1024)) {
		if (in.eof()) break;

		stringstream ss(buffer);
		string token;

		getline(ss, token, ':');
		if (strcmp(token.c_str(), "ClassID") == 0) {
			getline(ss, token, ':');
			token.erase(token.find_last_not_of(" ") + 1);
			ClassID = atoi(token.c_str());
		}
		else if (strcmp(token.c_str(), "TimeSeriesLength") == 0) {
			getline(ss, token, ':');
			token.erase(token.find_last_not_of(" ") + 1);
			TimeSeriesLength = atoi(token.c_str());
		}
		else if (strcmp(token.c_str(), "TimeSeriesData") == 0) {
			for (int i = 0;i < TimeSeriesLength;i++) {
				memset(buffer, 0, 1024);
				in.getline(buffer, 1024);
				out << ClassID << '\t' << 1 + i << '\t' << buffer << endl;
			}
		}
		memset(buffer, 0, 1024);
	}
	out.close();
}

void TrainingDataParser::Start(std::string fileName, int id) {
	std::ifstream in;
	in.open(fileName);

	ReadHeader(in);
	ReadWriteTimeSeries(in, id);
	in.close();
	return;
}