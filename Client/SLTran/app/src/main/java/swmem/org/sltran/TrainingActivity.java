package swmem.org.sltran;

import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.graphics.drawable.shapes.PathShape;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.support.v7.app.AppCompatActivity;
import android.text.InputType;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import org.w3c.dom.Text;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.HashMap;

/**
 * Created by Gabriel on 2016. 4. 3..
 */
public class TrainingActivity  extends AppCompatActivity {
    private ListView m_ListView;
    private ArrayAdapter<String> m_Adapter;
    private ArrayList<HashMap> dataList;
    private SocketListener sl = null;

    private int sendingMode = 0;
    private int loadedLabelCount = 0;

    private int selectedIndex = -1;
    private String m_Text = "";
    private ProgressDialog pDial;
    private AlertDialog alertd;
    private EditText input;

    private AlertDialog.Builder builder;
    private AlertDialog.Builder okBuilder;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.traning);
        pDial = new ProgressDialog(this);
        pDial.setCancelable(false);
        pDial.setCanceledOnTouchOutside(false);

        dataList = new ArrayList<HashMap>();

        // Android에서 제공하는 string 문자열 하나를 출력 가능한 layout으로 어댑터 생성
        m_Adapter = new ArrayAdapter<String>(getApplicationContext(), R.layout.training_list_item);

        // Xml에서 추가한 ListView 연결
        m_ListView = (ListView) findViewById(R.id.modelList);

        // ListView에 어댑터 연결
        m_ListView.setAdapter(m_Adapter);

        // ListView 아이템 터치 시 이벤트 추가
        m_ListView.setOnItemClickListener(onClickListItem);

        readModule();
        readUncomitted();

        addListenerOnButton();
        DataManager.getInstace().setModeWithActivity(this);

    }

    @Override
    protected void onDestroy(){
        super.onDestroy();
        DataManager.getInstace().setNormalMode();
    }

    private void makeOkBuilder(){
        pDial.hide();
        // Set the default text to a link of the Queen
        okBuilder = new AlertDialog.Builder(this);
        okBuilder.setTitle("단어 추가");
        okBuilder.setMessage("서버로 전송하시겠습니까?");
        okBuilder.setCancelable(false);
        alertd = okBuilder.create();

        // Set up the buttons
        okBuilder.setPositiveButton("전송", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                new Thread() {
                    public void run() {

                        //send id to server
                        Log.v("TRAINING", "ADD_ACTION_BUTTON");
                        String strMsg = DataManager.getInstace().idTxt;
                        String len = String.format("%03d", strMsg.length());

                        String strMsg2 = DataManager.getInstace().pwTxt;
                        String len2 = String.format("%03d", strMsg.length());

                        sl = new SocketListener(getApplicationContext(), sHandler, len + strMsg + len2 + strMsg2, null);
                        sl.start();
                        state = 0;

                    }
                }.start();
                pDial.show();
                okBuilder.setView(null);
                alertd.setView(null);

                dialog.dismiss();


            }
        });
        okBuilder.setNegativeButton("취소", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                sl.sendString("EndConnection");
                sl.endConnection();

                sendingMode = 0;
                okBuilder.setView(null);
                alertd.setView(null);

                dialog.dismiss();
            }
        });


    }

    private void makeAlertBuilder(){

        // Set the default text to a link of the Queen
        builder = new AlertDialog.Builder(this);
        builder.setTitle("단어 추가");
        builder.setMessage("추가할 단어를 입력해주세요.");
        builder.setCancelable(false);
        alertd = builder.create();

        // Set up the input
        input = new EditText(this);
        // Specify the type of input expected; this, for example, sets the input as a password, and will mask the text
        input.setInputType(InputType.TYPE_CLASS_TEXT);
        builder.setView(input);

        // Set up the buttons
        builder.setPositiveButton("확인", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                m_Text = input.getText().toString();

                HashMap hash = new HashMap();
                hash.put("classMean", m_Text);
                hash.put("classLabel", dataList.size() + 1);
                addRowUncomitted(hash);

                refreshListview();
                DataManager.getInstace().specificClassNumber = dataList.size();
                DataManager.getInstace().specificMeaning = m_Text;

                pDial.setMessage("장갑으로 동작을 입력해주세요");
                pDial.show();

                builder.setView(null);
                alertd.setView(null);

                dialog.dismiss();


            }
        });
        builder.setNegativeButton("취소", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {

                builder.setView(null);
                alertd.setView(null);

                dialog.dismiss();
            }
        });
    }

    private int state = 0;
    private final Handler sHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {

            switch (msg.what){
                case 0:
                    if(msg.obj.toString().equals("OK")){
                        if(state == 0) {
                            if(sendingMode == 1){ //동작추가
                                sl.sendString("FileSend");
                                pDial.setMessage("각도 파일 전송합니다...");
                            }else if(sendingMode == 4){
                                sl.sendString("CreateModel");
                            }else if(sendingMode == 3){
                                sl.sendString("FileSend");
                                pDial.setMessage("각도 파일 전송합니다...");
                            }
                            state++;
                        }else if(state == 1){
                            if(sendingMode == 1){
                                sl.sendGRTFile("/sdcard/SLTrans/angle.grt");
                                pDial.setMessage("손가락 파일 전송합니다...");
                            }else if (sendingMode == 4) {
                                deleteUncomitted();
                                sendingMode = 0;
                                sl.sendString("EndConnection");
                                sl.endConnection();
                                DataManager.getInstace().setNormalMode();
                                Toast.makeText(getApplicationContext(), "모델링 시작합니다", Toast.LENGTH_SHORT).show();
                                finish();
                            }else if (sendingMode == 3){
                                sl.sendGRTFile("/sdcard/SLTrans/angle.grt");
                                pDial.setMessage("손가락 파일 전송합니다...");
                            }
                            state++;
                        }else if(state == 2){
                            if(sendingMode == 1){
                                sl.sendGRTFile("/sdcard/SLTrans/hand.grt");
                                pDial.setMessage("전송완료 종료합니다...");
                            }else if (sendingMode == 3){
                                sl.sendGRTFile("/sdcard/SLTrans/hand.grt");
                                pDial.setMessage("전송완료 종료합니다...");
                            }
                            state++;
                        }else if(state == 3){
                            if(sendingMode == 1){
                                sl.sendString("EndConnection");
                                sl.endConnection();
                                pDial.hide();
                                Toast.makeText(getApplicationContext(), "동작 전송 완료", Toast.LENGTH_SHORT).show();

                                DataManager.getInstace().specificMeaning = "";
                                DataManager.getInstace().specificClassNumber = -1;
                                sendingMode = 0;

                            }else if (sendingMode == 3){
                                sl.sendString("EndConnection");
                                sl.endConnection();
                                pDial.hide();
                                Toast.makeText(getApplicationContext(), "동작 전송 완료", Toast.LENGTH_SHORT).show();

                                DataManager.getInstace().specificMeaning = "";
                                DataManager.getInstace().specificClassNumber = -1;
                                sendingMode = 0;
                                selectedIndex = -1;
                            }
                        }else if(state == 4){

                        }
                    }else{
                    }
                    break;
                default:
                    break;
            }
        }
    };

    // 아이템 터치 이벤트
    private AdapterView.OnItemClickListener onClickListItem = new AdapterView.OnItemClickListener() {

        @Override
        public void onItemClick(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
            // 이벤트 발생 시 해당 아이템 위치의 텍스트를 출력
            HashMap dataHash = dataList.get(arg2);
            selectedIndex = arg2;

            TextView tView = (TextView) findViewById(R.id.textAction);
            tView.setText("동작번호: " + dataHash.get("classLabel"));
        }
    };


    public void addListenerOnButton() {

        Button btnAddAction = (Button) findViewById(R.id.btnAddAction);
        btnAddAction.setOnClickListener(new View.OnClickListener() {

            @Override
            public void onClick(View arg0) {
                sendingMode = 1;

                if (selectedIndex == -1) {
                    sendingMode = 0;
                    Toast.makeText(getApplicationContext(), "먼저 동작을 선택해주세요", Toast.LENGTH_SHORT).show();
                } else {
                    HashMap hash = dataList.get(selectedIndex);
                    DataManager.getInstace().specificClassNumber = selectedIndex + 1;
                    DataManager.getInstace().specificMeaning = (String) hash.get("classMean");
                    pDial.setMessage("장갑으로 동작을 입력해주세요");
                    pDial.show();
                    state=0;
                }
            }

        });

        Button btnDelete = (Button) findViewById(R.id.btnDelete);
        btnDelete.setOnClickListener(new View.OnClickListener() {

            @Override
            public void onClick(View arg0) {
                sendingMode = 2;

            }

        });

        Button btnAddWord = (Button) findViewById(R.id.btnAddWord);
        btnAddWord.setOnClickListener(new View.OnClickListener() {

            @Override
            public void onClick(View arg0) {
                sendingMode = 3;
                selectedIndex = -1;
                makeAlertBuilder();
                builder.show();


                /*
                m_Adapter.add("" + m_Adapter.getCount());
                new Thread() {
                    public void run() {

                        //send id to server
                        Log.v("TRAINING","ADD_ACTION_BUTTON");
                        String strMsg = DataManager.getInstace().idTxt;
                        String len = String.format("%03d",strMsg.length());

                        String strMsg2 = DataManager.getInstace().pwTxt;
                        String len2 = String.format("%03d",strMsg.length());

                        sl = new SocketListener(getApplicationContext(), sHandler,len + strMsg + len2 + strMsg2,null);
                        sl.start();

                    }
                }.start();
                */
            }

        });

        Button btnStart = (Button) findViewById(R.id.btnStart);
        btnStart.setOnClickListener(new View.OnClickListener() {

            @Override
            public void onClick(View arg0) {
                sendingMode = 4;

                //send id to server
                new Thread() {
                    public void run() {

                        //send id to server
                        Log.v("TRAINING","ADD_ACTION_BUTTON");
                        String strMsg = DataManager.getInstace().idTxt;
                        String len = String.format("%03d",strMsg.length());

                        String strMsg2 = DataManager.getInstace().pwTxt;
                        String len2 = String.format("%03d",strMsg.length());

                        sl = new SocketListener(getApplicationContext(), sHandler,len + strMsg + len2 + strMsg2,null);
                        sl.start();
                        state=0;
                    }
                }.start();

            }

        });

    }

    public void recorded(){
        if(sendingMode == 1) {
            //pDial.setMessage("동작 녹화 완료, 송신중...");
            this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    makeOkBuilder();
                    okBuilder.show();
                }//public void run() {
            });

        }else if(sendingMode == 3){
            this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    makeOkBuilder();
                    okBuilder.show();
                }//public void run() {
            });
        }
    }

    @Override
    public void onStart() {
        super.onStart();

    }

    private void deleteUncomitted(){
        File dir = new File ("/sdcard/SLTrans/");
        File file = new File(dir, "uncommited.sltran");

        file.delete();
    }

    private void addRowUncomitted(HashMap hash){
        File dir = new File ("/sdcard/SLTrans/");
        dir.mkdirs();
        File file = new File(dir, "uncommited.sltran");

        try {
            FileOutputStream f = new FileOutputStream(file,true);
            PrintWriter pw = new PrintWriter(f);
            String mean = (String)hash.get("classMean");
            int label = (int)hash.get("classLabel");

            pw.println("ClassLabel: " + label);
            pw.println("ClassMean: " + mean);
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

    private void refreshListview(){
        m_Adapter.clear();
        dataList.clear();
        loadedLabelCount = 0;
        readModule();
        readUncomitted();
    }

    private void readUncomitted(){
        File userModel = new File ("/sdcard/SLTrans/uncommited.sltran");

        try {
            BufferedReader br = new BufferedReader(new FileReader(userModel));
            String line;

            while ((line = br.readLine()) != null) {
                if(line.contains("ClassLabel: ")){
                    String classLabel = line.replace("ClassLabel: ", "");
                    HashMap hash = new HashMap();
                    hash.put("classLabel", classLabel);

                    if(Integer.parseInt(classLabel) <= loadedLabelCount){
                        deleteUncomitted();
                        return;
                    }
                    dataList.add(hash);
                }else if(line.contains("ClassMean: ")) {
                    String classMean = line.replace("ClassMean: ", "");

                    HashMap hash = dataList.get(dataList.size() - 1);
                    hash.put("classMean",classMean);
                    m_Adapter.add(" " + hash.get("classLabel") + ". " + classMean);
                    loadedLabelCount++;
                }
            }
            br.close();
        }
        catch (IOException e) {
            e.printStackTrace();
            //You'll need to add proper error handling here
        }
    }

    private void readModule(){
        //m_Adapter.add("temp");
        File userModel = new File ("/sdcard/SLTrans/ModelAngleData.grt");

        try {
            BufferedReader br = new BufferedReader(new FileReader(userModel));
            String line;

            while ((line = br.readLine()) != null) {
                if(line.contains("ClassLabel: ")){
                    String classLabel = line.replace("ClassLabel: ", "");
                    HashMap hash = new HashMap();
                    hash.put("classLabel", classLabel);
                    dataList.add(hash);
                }else if(line.contains("ClassMean: ")) {
                    String classMean = line.replace("ClassMean: ", "");

                    HashMap hash = dataList.get(dataList.size() - 1);
                    hash.put("classMean",classMean);
                    m_Adapter.add(" " + hash.get("classLabel") + ". " + classMean);
                    loadedLabelCount++;
                }
            }
            br.close();
        }
        catch (IOException e) {
            e.printStackTrace();
            //You'll need to add proper error handling here
        }
    }
}
