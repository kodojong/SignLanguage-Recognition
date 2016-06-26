package swmem.org.sltran;

import android.Manifest;
import android.app.Activity;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.support.v4.app.ActivityCompat;
import android.util.Log;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ProgressBar;
import android.widget.Toast;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.PrintWriter;


/**
 * Created by Gabriel on 2016. 3. 15..
 */

public class LoginActivity extends Activity{
    private EditText idTxt;
    private EditText pwTxt;
    private Button loginBtn;
    private SocketListener sl = null;
    private String receiveString = "";

    private ProgressDialog pDial;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.login);
        pDial = new ProgressDialog(this);
        pDial.setCancelable(false);
        pDial.setCanceledOnTouchOutside(false);

        idTxt = (EditText) findViewById(R.id.idTxt);
        pwTxt = (EditText) findViewById(R.id.pwTxt);

        idTxt.requestFocus();

        Handler mHandler = new Handler();

        mHandler.postDelayed(new Runnable() {
            public void run() {
                InputMethodManager mgr = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
                mgr.showSoftInput(idTxt, InputMethodManager.SHOW_FORCED);
            }
        }, 500);
        

        addListenerOnButton();

        //Call GRT
        DataManager.getInstace();
        DataManager.getInstace().setTTS(this);
        verifyStoragePermissions(this);
    }

    @Override
    protected void onPause(){
        super.onPause();
        InputMethodManager imm = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
        imm.hideSoftInputFromWindow(idTxt.getWindowToken(), 0);
        imm.hideSoftInputFromWindow(pwTxt.getWindowToken(), 0);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        InputMethodManager imm = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
        imm.hideSoftInputFromWindow(idTxt.getWindowToken(), 0);
        imm.hideSoftInputFromWindow(pwTxt.getWindowToken(), 0);
    }

    private int state = 0;

    private final Handler sHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {

            switch (msg.what){
                case 0:
                    if(msg.obj.toString().equals("OK")){
                        if(state == 0) {
                            Log.v("LOGIN","ID_PW_OK");
                            sl.sendString("FileReceive");
                            receiveString = "";
                            state++;
                            pDial.setMessage("각도 모델 수신중...");
                        }else if(state == 1){
                            saveToFile("ModelAngleData.grt");
                            Log.v("LOGIN", "ANGLE_OK");
                            receiveString = "";
                            state++;
                            pDial.setMessage("손가락 모델 수신중...");
                        }else if(state == 2){
                            saveToFile("ModelHandData.grt");
                            Log.v("LOGIN", "HAND_OK");
                            sl.sendString("EndConnection");
                            sl.endConnection();

                            pDial.hide();
                            InputMethodManager imm = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
                            imm.hideSoftInputFromWindow(idTxt.getWindowToken(), 0);
                            imm.hideSoftInputFromWindow(pwTxt.getWindowToken(), 0);

                            Intent intentSubActivity = new Intent(LoginActivity.this, MainActivity.class);
                            startActivity(intentSubActivity);

                            // We need an Editor object to make preference changes.
                            // All objects are from android.context.Context
                            DataManager.getInstace().idTxt = idTxt.getText().toString();
                            DataManager.getInstace().pwTxt = pwTxt.getText().toString();

                            state++;
                        }else if(state == 3){

                        }else if(state == 4){

                        }
                    }else if(msg.obj.toString().equals("NO")){
                        if(state == 1){
                            state++;

                        }else if(state == 2){
                            sl.sendString("EndConnection");
                            sl.endConnection();

                            pDial.hide();
                            InputMethodManager imm = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
                            imm.hideSoftInputFromWindow(idTxt.getWindowToken(), 0);
                            imm.hideSoftInputFromWindow(pwTxt.getWindowToken(), 0);

                            Intent intentSubActivity = new Intent(LoginActivity.this, MainActivity.class);
                            startActivity(intentSubActivity);

                            // We need an Editor object to make preference changes.
                            // All objects are from android.context.Context
                            DataManager.getInstace().idTxt = idTxt.getText().toString();
                            DataManager.getInstace().pwTxt = pwTxt.getText().toString();
                        }
                    }else{
                        if(state == 1 || state == 2){
                            receiveString += msg.obj.toString() + '\n';

                        }
                    }
                    break;
                default:
                    break;
            }
        }
    };


    public void addListenerOnButton() {

        loginBtn = (Button) findViewById(R.id.loginBtn);
        loginBtn.setOnClickListener(new View.OnClickListener() {

            @Override
            public void onClick(View arg0) {
                pDial.show();
                pDial.setMessage("로그인 시도중...");

                new Thread() {
                    public void run() {
                        //send id to server
                        Log.v("LOGIN","BUTTON_PRESSED");
                        String strMsg = idTxt.getText().toString();
                        String len = String.format("%03d",strMsg.length());

                        String strMsg2 = pwTxt.getText().toString();
                        String len2 = String.format("%03d",strMsg.length());

                        sl = new SocketListener(getApplicationContext(), sHandler,len + strMsg + len2 + strMsg2,null);
                        sl.start();

                    }
                }.start();
/*

                DataManager.getInstace().idTxt = idTxt.getText().toString();
                DataManager.getInstace().pwTxt = pwTxt.getText().toString();

                Intent intentSubActivity = new Intent(LoginActivity.this, MainActivity.class);
                startActivity(intentSubActivity);

                InputMethodManager imm = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
                imm.hideSoftInputFromWindow(idTxt.getWindowToken(), 0);
                imm.hideSoftInputFromWindow(pwTxt.getWindowToken(), 0);

                pDial.hide();

*/

            }

        });

    }

    public void saveToFile(String filenamne){
        File dir = new File ("/sdcard/SLTrans/");
        dir.mkdirs();
        File file = new File(dir, filenamne);

        try {
            FileOutputStream f = new FileOutputStream(file);
            PrintWriter pw = new PrintWriter(f);
            pw.println(receiveString);
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

    public void receiveAgain(){

    }

    // Storage Permissions
    private static final int REQUEST_EXTERNAL_STORAGE = 1;
    private static String[] PERMISSIONS_STORAGE = {
            Manifest.permission.READ_EXTERNAL_STORAGE,
            Manifest.permission.WRITE_EXTERNAL_STORAGE
    };

    /**
     * Checks if the app has permission to write to device storage
     *
     * If the app does not has permission then the user will be prompted to grant permissions
     *
     * @param activity
     */
    public static void verifyStoragePermissions(Activity activity) {
        // Check if we have write permission
        int permission = ActivityCompat.checkSelfPermission(activity, Manifest.permission.WRITE_EXTERNAL_STORAGE);

        if (permission != PackageManager.PERMISSION_GRANTED) {
            // We don't have permission so prompt the user
            ActivityCompat.requestPermissions(
                    activity,
                    PERMISSIONS_STORAGE,
                    REQUEST_EXTERNAL_STORAGE
            );
        }
    }
}
