package swmem.org.sltran;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Build;
import android.os.Handler;
import android.os.Message;
import android.speech.tts.TextToSpeech;
import android.speech.tts.TextToSpeech.OnInitListener;

import android.util.Log;
import android.view.inputmethod.InputMethodManager;

import net.daum.mf.speech.api.TextToSpeechClient;
import net.daum.mf.speech.api.TextToSpeechListener;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.PrintWriter;
import java.lang.reflect.Array;
import java.util.ArrayList;
import java.util.Timer;
import java.util.TimerTask;


/**
 * Created by Gabriel on 2016. 3. 27..
 */
public class DataManager implements TextToSpeechListener, OnInitListener {
    private static DataManager uniqueInstance;
    public static final String PREFS_NAME = "MyPrefsFile";
    private String gloveRawData = "";
    private ArrayList<String> dataBuffer = new ArrayList<String>();
    private String[] leftArray = null;
    private String[] rightArray = null;
    private int mode = 0;

    public String idTxt;
    public String pwTxt;

    public MainActivity mainAct;

    public int specificClassNumber = -1;
    public String specificMeaning = "";

    public static synchronized  DataManager getInstace() {
        if(uniqueInstance == null){
            uniqueInstance = new DataManager();
        }
        return uniqueInstance;
    }

    private Activity tAct;
    public void setModeWithActivity(Activity acti){
        tAct = acti;
        mode = 1;
    }

    public void setNormalMode(){
        tAct = null;
        mode = 0;
    }

    private GRTClass grtClass;
    private TextToSpeechClient ttsClient;
    private TextToSpeech myTTS;
    private DataManager(){
        grtClass = new GRTClass();
        grtClass.loadModule();

        ttsClient = new TextToSpeechClient.Builder()
                .setApiKey("523f6c5510a994b60ac55edf80e68000")              // 발급받은 api key
                .setSpeechMode(TextToSpeechClient.NEWTONE_TALK_1)            // 음성합성방식
                .setSpeechSpeed(0.4)            // 발음 속도(0.5~4.0)
                .setSpeechVoice(TextToSpeechClient.VOICE_WOMAN_DIALOG_BRIGHT)  //TTS 음색 모드 설정(여성 차분한 낭독체)
                .setListener(this)
                .build();
    }

    public void setTTS(Context context){
            myTTS = new TextToSpeech(context, this);

    }

    public void initializeGlove(){
        gloveRawData = "";
        leftArray = null;
        rightArray = null;
        dataBuffer = new ArrayList<String>();

        if(timer!=null){
            timer.cancel();
        }

        if(mainAct!=null){
            mainAct.goDisconnected();
        }
    }

    private void parseGloveData(){
        timer.cancel();
        receivingFlag = false;

        for(String str : dataBuffer){
            if(str.contains("L")){
                leftArray = str.split(",");
            }else if(str.contains("R")){
                rightArray = str.split(",");
            }
        }

        divideData();
    }

    private boolean receivingFlag = false;
    private Timer timer = null;
    private TimerTask second = null;
    private int sharpCnt = 0;

    public void addReceivingData(String inputData){
        if(inputData.length() > 0) {
            if(timer != null) {
                timer.cancel();
            }

            second = new TimerTask() {
                @Override
                public void run() {
                    if (sharpCnt == 0) {
                        Log.v("GLOVE", "TOTAL BT ERROR");
                        gloveRawData = "";
                        leftArray = null;
                        rightArray = null;
                        dataBuffer = new ArrayList<String>();

                        if (mainAct != null) {
                            mainAct.sendMessage("~");
                        }
                    } else if (sharpCnt == 1) {
                        Log.v("GLOVE", "RIGHT BT ERROR");
                        gloveRawData = "";
                        rightArray = null;
                        if (mainAct != null) {
                            mainAct.sendMessage("@");
                        }
                    }
                }
            };

            timer = new Timer();
            timer.schedule(second,5000);

        }

        gloveRawData += inputData;

        if(inputData.contains("#")){
            sharpCnt++;
            String[] array = gloveRawData.split("#");
            dataBuffer.add(array[0]);

            if(array.length >= 2){
                gloveRawData = array[1];
            }else{
                gloveRawData = "";
            }

            parseGloveData();
        }

    }

    private void divideData(){
        if((leftArray != null) && (rightArray != null)){
            ArrayList<String> angleStrArr = new ArrayList<String>();
            ArrayList<String> fingerStrArr = new ArrayList<String>();

            if(leftArray.length == rightArray.length){
                for(int i = 1 ; i < leftArray.length ; i += 8){
                    for(int j = 0; j < 8; j++){
                        String leftVal = leftArray[i+j];

                        if(j == 0){
                            String tempStr = new String(leftVal);
                            angleStrArr.add(tempStr);
                        }else if(j < 3){
                            int lastIdx = angleStrArr.size() - 1;
                            String tempStr = angleStrArr.get(lastIdx);
                            tempStr += "   " + leftVal;
                            angleStrArr.set(lastIdx,tempStr);
                        }else if(j == 3){
                            String tempStr = new String(leftVal);
                            fingerStrArr.add(tempStr + "0");
                        }else if(j < 8){
                            int lastIdx = fingerStrArr.size() - 1;
                            String tempStr = fingerStrArr.get(lastIdx);
                            tempStr += "   " + leftVal;
                            fingerStrArr.set(lastIdx,tempStr + "0");
                        }
                    }

                    for(int j = 0; j < 8; j++){
                        String rightVal = rightArray[i+j];

                        if(j < 3){
                            String tempStr = angleStrArr.get(i/8);
                            tempStr += "   " + rightVal;
                            angleStrArr.set(i/8, tempStr);
                        }else if(j < 8){
                            String tempStr = fingerStrArr.get(i/8);
                            tempStr += "   " + rightVal;
                            fingerStrArr.set(i/8, tempStr + "0");
                        }
                    }
                }

            }else{
                Log.v("ERROR","Length Error " + leftArray.length + " " + rightArray.length);
                if(mainAct != null){
                    mainAct.sendMessage("~");
                    timer.cancel();
                    receivingFlag = false;
                }

                gloveRawData = "";
                leftArray = null;
                rightArray = null;
                dataBuffer = new ArrayList<String>();

                return;
            }


            saveToGRTFile(angleStrArr,"angle.grt",6);
            saveToGRTFile(fingerStrArr, "hand.grt", 10);

            if(mainAct != null){
                mainAct.sendMessage("!");
                timer.cancel();
                receivingFlag = false;
            }

            sharpCnt = 0;
            if(mode == 0) {
                Log.v("GLOVE", "make file complete");

                String strText = grtClass.getHelloNDKString();


               // ttsClient.play(strText);

                if (Build.VERSION.RELEASE.startsWith("5")) {
                    myTTS.speak(strText + " " + strText + " " + strText, TextToSpeech.QUEUE_FLUSH, null, null);
                }
                else {
                    myTTS.speak(strText, TextToSpeech.QUEUE_FLUSH, null);
                }

            }else{
                if(tAct != null){
                    ((TrainingActivity)tAct).recorded();
                }
            }
            gloveRawData = "";
            leftArray = null;
            rightArray = null;
            dataBuffer = new ArrayList<String>();
        }else{
            Log.e("GRTSaving", "there is no enough data");
        }
    }

    private void saveToGRTFile(ArrayList<String> array, String fileName, int numberOfDimension){
        File dir = new File ("/sdcard/SLTrans/");
        dir.mkdirs();
        File file = new File(dir, fileName);

        try {
            FileOutputStream f = new FileOutputStream(file);
            PrintWriter pw = new PrintWriter(f);
            pw.println("GRT_LABELLED_TIME_SERIES_CLASSIFICATION_DATA_FILE_V1.0");
            pw.println("DatasetName: HandGestureTimeSeriesData");
            pw.println("InfoText: This dataset contains 11 Hand timeseries gestures.");
            pw.println("NumDimensions: " + numberOfDimension);
            pw.println("TotalNumTrainingExamples: 1");
            pw.println("NumberOfClasses: 1");
            pw.println("ClassIDsAndCounters:");
            if(specificClassNumber > 0){
                pw.println("" + specificClassNumber + " 1 " + specificMeaning);
            }else {
                pw.println("1 1 mean");
            }
            pw.println("UseExternalRanges: 0");
            pw.println("LabelledTimeSeriesTrainingData:");
            pw.println("************TIME_SERIES************");
            if(specificClassNumber > 0){
                pw.println("ClassID: " + specificClassNumber);
            }else{
                pw.println("ClassID: 1");
            }
            pw.println("TimeSeriesLength: " + array.size());
            pw.println("TimeSeriesData:");
            for(String tStr : array){
                pw.println(tStr);
            }
            pw.flush();
            pw.close();
            f.close();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
            Log.i("FILELOG", "******* File not found. Did you" +
                    " add a WRITE_EXTERNAL_STORAGE permission to the   manifest?");
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private void uploadToServer(){

    }

    @Override
    public void onFinished() {

    }

    @Override
    public void onError(int i, String s) {
        Log.v("LLLL",s);
    }

    @Override
    public void onInit(int status) {

    }
}