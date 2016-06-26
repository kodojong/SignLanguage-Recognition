package swmem.org.sltran;

import android.content.Context;
import android.os.Handler;
import android.os.Message;
import android.util.Log;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.Socket;


/**
 * Created by Gabriel on 2016. 3. 23..
 */
public class SocketListener extends Thread{

    private InputStream im;
    private BufferedReader br;
    private String sendMsg;

    Handler mHandler ;

    Context context;

    public SocketListener(Context context, Handler handler,String msg,String path) {
        this.mHandler = handler;
        this.context = context;

        try {
            im = SocketManager.getSocket().getInputStream();
            br = new BufferedReader(new InputStreamReader(im,"UTF-8"));
                    //new BufferedReader(new InputStreamReader(im));
            //new BufferedReader(new InputStreamReader(client.getInputStream(),"utf-8"));
            if(msg != null){
                SocketManager.sendMsg(msg);
            }else if(path != null){
                SocketManager.sendTxt(path);
            }

        } catch (IOException e) {
            Log.e("SocketListener/lis", e.getMessage());
        }
    }

    public void sendString(String msg){
        try {
            SocketManager.sendMsg(msg);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public void sendGRTFile(String path){
        try {
            SocketManager.sendTxt(path);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public void endConnection(){
        try {
            SocketManager.closeSocket();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    @Override
    public void run() {
        while(!SocketManager.isClosed())
        {
            try {
                String receivedmsg;
                while((receivedmsg = br.readLine())!= null)
                {
                    Log.e("SocketListener", receivedmsg);
                    Message msg = Message.obtain(mHandler, 0,0,0,receivedmsg);
                    mHandler.sendMessage(msg);
                }
            } catch (IOException e) {
                Log.e("SocketListener/run",e.getMessage());
            }
        }
        SocketManager.clearSocket();
    }

}
