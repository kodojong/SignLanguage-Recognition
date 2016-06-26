/**
@file
@author
@version 0.0.1
*/
#include "Dyslexic.h"

//using namespace std;

namespace GRT {
	using std::cerr;
	Dyslexic::Dyslexic(const string index) :_user_index(index), _is_loaded_traing_data(false), _is_loaded_model_data(false) {

	}
	Dyslexic::~Dyslexic() {

	}

	bool Dyslexic::loadTrainingData() {
		//already load
		/*if (_is_loaded_traing_data) {
			return false;
		}*/

		//To load new traingData, delete old traingData
		//for this, "TimeSeriesClassificationData::loadDatasetFromFile()" is adjusted
		//Line:454, 500(classTracker, data clear())
		string fileName = _user_index + "TrainingAngleData.grt";
		if (!_training_angle_data.load(fileName)) {
			cerr << "Failed to load training data!\n";
			return false;
		}
		fileName = _user_index + "TrainingHandData.grt";
		if (!_training_hand_data.load(fileName)) {
			cerr << "Failed to load training data!\n";
			return false;
		}
		_is_loaded_traing_data = true;
		return true;
	}

	bool Dyslexic::makeModelData() {

		string fileName = _user_index + "ModelHandData.grt";
		if (!_dtw_hand.save(fileName)) {
			cerr << "Failed to save the classifier model!\n";
			return false;
		}
		fileName = _user_index + "ModelAngleData.grt";
		if (!_dtw_angle.save(fileName)) {
			cerr << "Failed to save the classifier model!\n";
			return false;
		}
		return true;
	}
	bool Dyslexic::loadModelData() {

		string fileName = _user_index + "ModelHandData.grt";
		if (!_dtw_hand.load(fileName)) {
			cerr << "Failed to load the classifier model!\n";
			return false;
		}
		_dtw_hand.trained = true;

		fileName = _user_index + "ModelAngleData.grt";
		if (!_dtw_angle.load(fileName)) {
			cerr << "Failed to load the classifier model!\n";
			return false;
		}
		_dtw_angle.trained = true;

		_is_loaded_model_data = true;
		return true;
	}
	bool Dyslexic::loadTestData() {

		string fileName = _user_index + "TestAngleData.grt";
		if (!_test_angle_data.load(fileName)) {
			cerr << "Failed to load the TestAngleData!\n";
			return false;
		}
		_test_angle_data = _test_angle_data.partition(0);

		fileName = _user_index + "TestHandData.grt";
		if (!_test_hand_data.load(fileName)) {
			cerr << "Failed to load the TestAngleData!\n";
			return false;
		}
		_test_hand_data = _test_hand_data.partition(0);

		return true;
	}
	bool Dyslexic::train() {

		if (!_dtw_angle.train(_training_angle_data)) {
			cerr << "Failed to train classifier!\n";
			return false;
		}

		if (!_dtw_hand.train(_training_hand_data)) {
			cerr << "Failed to train classifier!\n";
			return false;
		}

		if (!makeModelData()) {

			return false;
		}
		return true;
	}

	bool Dyslexic::predict() {
		//필요없는 변수 삭제
	//	UINT classLabel;
	//	UINT predictedClassLabel;
		MatrixDouble timeseries;

		//first predict about angle
		for (UINT i = 0; i < _test_angle_data.getNumSamples(); i++) {
			//Get the i'th test sample - this is a timeseries
			timeseries = _test_angle_data[i].getData();

			//Perform a prediction using the classifier
			if (!_dtw_angle.predict(timeseries)) {
				//cout << "Failed to perform prediction for test sample: " << i << "\n";
				return false;
			}
		}
		if (!_dtw_angle.getPredictedClassLabels(_predicted_classLabels, _best_select_number)) {
			//cout << "Failed to get PredictedClassLabels from angle! " << endl;
			return false;
		}
		if (!_dtw_hand.setPredictedClassLabels(_predicted_classLabels, _best_select_number)) {
			//cout << "Failed to set PredictedClassLabels to hand! " << endl; 
			return false;
		}
		//second predict about hand
		_dtw_hand.setSecond(true);
		for (UINT i = 0; i < _test_hand_data.getNumSamples(); i++) {
			//Get the i'th test sample - this is a timeseries
			timeseries = _test_hand_data[i].getData();

			//Perform a prediction using the classifier
			if (!_dtw_hand.predict(timeseries)) {
				//cout << "Failed to perform prediction for test sample: " << i << "\n";
				return false;
			}

		}
		_predicted_classLabel = _dtw_hand.getPredictedClassLabel();
		_predicted_classMean = _dtw_hand.getPredictedClassMean();
		return true;
	}

	const string Dyslexic::getPredictMean() {
		return _predicted_classMean;
	}

	const int Dyslexic::getPredictedLabel() {
		return _predicted_classLabel;
	}
	/*
	string Dyslexic::getPredictMean() {
	string result;
	result =
	}
	*/
}