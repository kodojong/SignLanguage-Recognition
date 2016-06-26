#include "swmem_org_sltran_GRTClass.h"
#include "GRT.h"
#include <sstream>
#include "Dyslexic.h"
#include <android/log.h>
#define printf(...) __android_log_print(ANDROID_LOG_DEBUG, "TAG", __VA_ARGS__);

using namespace std;
using namespace GRT;

namespace GRT {

    Dyslexic::Dyslexic(const string index) :_user_index(index), _is_loaded_traing_data(false), _is_loaded_model_data(false) {
        printf("created Dyslexic");
    }
    Dyslexic::~Dyslexic() {

    }

    bool Dyslexic::loadModelData() {

        string fileName = "/sdcard/SLTrans/ModelHandData.grt";
        if (!_dtw_hand.load(fileName)) {
            return false;
        }
        _dtw_hand.trained = true;

        fileName = "/sdcard/SLTrans/ModelAngleData.grt";
        if (!_dtw_angle.load(fileName)) {
            return false;
        }
        _dtw_angle.trained = true;

        _is_loaded_model_data = true;
        printf("loadModelComplete 123123123");
        return true;
    }

    bool Dyslexic::loadTestData() {

        string fileName = "/sdcard/SLTrans/angle.grt";

        printf("test f1");
        if (!_test_angle_data.load(fileName)) {
            printf("test f2");
            return false;
        }
        _test_angle_data = _test_angle_data.partition(0);
        printf("test f3");
        fileName = "/sdcard/SLTrans/hand.grt";
        if (!_test_hand_data.load(fileName)) {
            printf("test f4");
            return false;
        }
        _test_hand_data = _test_hand_data.partition(0);
        printf("test f5");
        return true;
    }

    bool Dyslexic::predict() {

        _dtw_hand.trained = true;
        _dtw_angle.trained = true;

        UINT classLabel;
        UINT predictedClassLabel;
        MatrixDouble timeseries;

        //first predict about angle
        for (UINT i = 0; i < _test_angle_data.getNumSamples(); i++) {
            //Get the i'th test sample - this is a timeseries
            timeseries = _test_angle_data[i].getData();
            printf("prrrr %d",i);
            //Perform a prediction using the classifier
            if (!_dtw_angle.predict(timeseries)) {
                printf("pr1");
                //cout << "Failed to perform prediction for test sample: " << i << "\n";
                return false;
            }
        }

        if (!_dtw_angle.getPredictedClassLabels(_predicted_classLabels, _best_select_number)) {
            //cout << "Failed to get PredictedClassLabels from angle! " << endl;
            printf("pr2");
            return false;
        }

        if(!_dtw_hand.setPredictedClassLabels(_predicted_classLabels, _best_select_number)) {
            //cout << "Failed to set PredictedClassLabels to hand! " << endl;
            printf("pr3");
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
                printf("pr4");
                return false;
            }

        }

        _predicted_classLabel = _dtw_hand.getPredictedClassLabel();
        printf("classLabel : %d",_predicted_classLabel);
        _predicted_classMean = _dtw_hand.getPredictedClassMean();

        return true;
    }

    const int Dyslexic::getPredictedLabel() {
        return _predicted_classLabel;
    }

    const string Dyslexic::getPredictMean() {
        return _predicted_classMean;
    }
}



Dyslexic dys("1");

JNIEXPORT jstring JNICALL Java_swmem_org_sltran_GRTClass_getHelloNDKString
        (JNIEnv *env, jobject obj) {

    if(dys._is_loaded_model_data){
        printf("load ok");
    }
    dys.loadTestData();
    dys.predict();

    int label = dys.getPredictedLabel();
    string mean = dys.getPredictMean();
    printf("predicted label %d",label);
    stringstream ss;
    ss << mean;

    jstring str =  (*env).NewStringUTF(ss.str().c_str());
    return str;

}


JNIEXPORT void JNICALL Java_swmem_org_sltran_GRTClass_loadModule
        (JNIEnv *env, jobject obj) {
    printf("loadModelDataStart");
    dys.loadModelData();
    printf("loadModelDataComplete");

}
