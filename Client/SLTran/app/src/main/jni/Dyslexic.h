/**
@file
@author
@version 0.0.1
*/
/**

1.�Լ��� const �����Ұ�

*/
#ifndef GRT_Dyslexic_HEADER
#define GRT_Dyslexic_HEADER

#include "ClassificationModules/DTW/DTW.h"

namespace GRT {
	class Dyslexic {
	public:

		/**
		userIndex setting
		*/
		explicit Dyslexic(const string index);

		/**
		Default Destructor
		*/
		~Dyslexic();

		/**
		file open&close ��� �����
		*/
		bool loadTrainingData( );

		/**

		*/
		bool makeModelData();

		/**

		*/
		bool loadModelData();


		/**

		*/
		bool loadTestData();

		/**

		*/
		bool train();

		/**

		*/
		bool predict();


		/**

		*/
		const int getPredictedLabel();
		const string getPredictMean();

		/**

		*/
		//string getPredictMean();
		bool _is_loaded_model_data;
	private:
		const string _user_index;
		bool _is_loaded_traing_data;

		DTW _dtw_angle;
		DTW _dtw_hand;
		TimeSeriesClassificationData _training_hand_data;
		TimeSeriesClassificationData _training_angle_data;
		TimeSeriesClassificationData _test_hand_data;
		TimeSeriesClassificationData _test_angle_data;
		UINT _predicted_classLabels[3];
		UINT _predicted_classLabel;
		const UINT _best_select_number = 3;
		string _predicted_classMean;

		Dyslexic& operator=(const Dyslexic&);

	};

}


#endif
